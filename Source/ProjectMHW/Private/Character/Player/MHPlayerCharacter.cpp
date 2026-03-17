#include "Character/Player/MHPlayerCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"
#include "GameplayTags/MHCombatStateGameplayTags.h"
#include "GameplayTags/MHInputPatternGameplayTags.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/Input/MHInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHGameplayTags.h"
#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "Combat/Attributes/MHHealthAttributeSet.h"
#include "Combat/Attributes/MHPlayerAttributeSet.h"
#include "Combat/Attributes/MHResistanceAttributeSet.h"

DEFINE_LOG_CATEGORY(LogMHPlayerCharacter);

namespace
{
    /** 입력 2D 벡터를 카메라 yaw 기준 월드 방향으로 변환한다. */
    static FVector ResolveWorldMoveDirection(const AController* InController, const FVector2D& InMoveInput)
    {
        if (InMoveInput.IsNearlyZero())
        {
            return FVector::ZeroVector;
        }

        const float ControlYaw = InController ? InController->GetControlRotation().Yaw : 0.0f;
        const FRotator ControlRotation(0.0f, ControlYaw, 0.0f);
        const FVector ForwardDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);
        return (ForwardDirection * InMoveInput.Y + RightDirection * InMoveInput.X).GetSafeNormal();
    }
}

AMHPlayerCharacter::AMHPlayerCharacter()
{
    InitializeCapsuleSettings();
    InitializeMeshCollisionSettings();
    
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 회전 사용 비활성화
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    // 스프링암
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 280.0f;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 65.f);
    CameraBoom->bUsePawnControlRotation = true;

    // 카메라
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // 이동 세팅
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
        MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
    }


    CurrentStamina = StaminaConfig.MaxStamina;
    
    // 데칼 영향 제거
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->bReceivesDecals = false;
    }
    
    // GAS - AttributeSet 
    HealthAttributeSet = CreateDefaultSubobject<UMHHealthAttributeSet>(TEXT("HealthAttributeSet"));
    CombatAttributeSet = CreateDefaultSubobject<UMHCombatAttributeSet>(TEXT("CombatAttributeSet"));
    ResistanceAttributeSet = CreateDefaultSubobject<UMHResistanceAttributeSet>(TEXT("ResistanceAttributeSet"));
    PlayerAttributeSet = CreateDefaultSubobject<UMHPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void AMHPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : BeginPlay"), *GetName());

    ApplyPlayerVisuals();

    SpawnAndEquipDefaultWeapon();

    // 무브먼트 업데이트 델리게이트 바인딩
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        OnCharacterMovementUpdated.AddDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }
}

void AMHPlayerCharacter::InitializeCapsuleSettings()
{
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
}

void AMHPlayerCharacter::InitializeMeshCollisionSettings()
{
}

void AMHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    checkf(InputConfigDataAsset, TEXT("InputConfigDataAsset is null"));

    APlayerController* PC = GetController<APlayerController>();
    check(PC);

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    check(LocalPlayer);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    check(Subsystem);

    if (InputConfigDataAsset->DefaultMappingContext)
    {
        Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
    }

    UMHInputComponent* MHInputComponent = CastChecked<UMHInputComponent>(PlayerInputComponent);
    MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Move, ETriggerEvent::Triggered, this, &AMHPlayerCharacter::Input_Move);
    MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Move, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_Move);
    MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Look, ETriggerEvent::Triggered, this, &AMHPlayerCharacter::Input_Look);

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Sprint))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Sprint, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_SprintStarted);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Sprint, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_SprintCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Dodge))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Dodge, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_Dodge);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Dodge, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_DodgeCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Interact))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Interact, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_Interact);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackPrimary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackPrimary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackPrimary);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackPrimary, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_AttackPrimaryCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackSecondary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSecondary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackSecondary);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSecondary, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_AttackSecondaryCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_WeaponSpecial))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_WeaponSpecial, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_WeaponSpecial); //손승우 추가
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_WeaponSpecial, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_WeaponSpecialCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackSimultaneous))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSimultaneous, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackSimultaneous); //손승우 추가
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AimHold))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AimHold, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AimHoldStarted); //손승우 추가
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AimHold, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_AimHoldCompleted); //손승우 추가
    }
}

void AMHPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 무브먼트 업데이트 델리게이트 해제
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        OnCharacterMovementUpdated.RemoveDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }

    // 무기 어빌리티 해제
    if (EquippedWeapon && AbilitySystemComponent)
    {
        EquippedWeapon->ClearWeaponAbilities(AbilitySystemComponent);
    }

    UnlinkCurrentWeaponAnimLayer();

    Super::EndPlay(EndPlayReason);
}

void AMHPlayerCharacter::HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
    UpdateStamina(DeltaSeconds);
    EvaluateSprintState();
    UpdateLocomotionState();
}

void AMHPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
    const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
    CachedMoveInput2D = MovementVector;

    // 방향 입력이 들어온 프레임만 마지막 유효 입력으로 갱신한다.
    if (!MovementVector.IsNearlyZero())
    {
        LastNonZeroMoveInput2D = MovementVector;
    }

    if (!Controller)
    {
        return;
    }

    const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

    if (MovementVector.Y != 0.f)
    {
        const FVector ForwardDir = MovementRotation.RotateVector(FVector::ForwardVector);
        AddMovementInput(ForwardDir, MovementVector.Y);
    }

    if (MovementVector.X != 0.f)
    {
        const FVector RightDir = MovementRotation.RotateVector(FVector::RightVector);
        AddMovementInput(RightDir, MovementVector.X);
    }
}

void AMHPlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
    const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

    if (LookAxisVector.X != 0.f)
    {
        AddControllerYawInput(LookAxisVector.X);
    }

    if (LookAxisVector.Y != 0.f)
    {
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AMHPlayerCharacter::Input_SprintStarted(const FInputActionValue& InputActionValue)
{
    // 가만히 선 발도 상태에서는 Shift 입력을 납도로 사용한다. //손승우 수정
    if (CanStartSheathe())
    {
        StartSheathe();
        return;
    }

    bSprintHeld = true;
    EvaluateSprintState();
}

void AMHPlayerCharacter::Input_SprintCompleted(const FInputActionValue& InputActionValue)
{
    bSprintHeld = false;
    EvaluateSprintState();
}

void AMHPlayerCharacter::Input_Dodge(const FInputActionValue& InputActionValue)
{
    bDodgeHeld = true;

    // 태도 발도 상태에서 특정 동시 입력이 들어오면 회피보다 특수 액션 패턴을 우선 처리한다.
    if (TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForDodgeInput()))
    {
        return;
    }

    UAnimMontage* RollMontage = nullptr;

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        LastResolvedDodgeContext = EMHDodgeContext::Sheathed;
        LastResolvedDodgeVariant = EMHDirectionalVariant::Forward;
        TryRotateActorTowardsMoveInput();
        RollMontage = ResolveSheathedRollMontage();
    }
    else if (WeaponSheathState == EMHWeaponSheathState::Unsheathed)
    {
        if (IsLongSwordAttackChainDodgeContext())
        {
            LastResolvedDodgeContext = EMHDodgeContext::AttackChain;
            LastResolvedDodgeVariant = ResolveDirectionalVariantFromInput(true);
        }
        else
        {
            LastResolvedDodgeContext = EMHDodgeContext::UnsheathedNeutral;
            LastResolvedDodgeVariant = EMHDirectionalVariant::Forward;
            TryRotateActorTowardsMoveInput();
        }

        RollMontage = ResolveUnsheathedRollMontage();
    }

    if (!TryPlayRollMontage(RollMontage))
    {
        UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Roll montage play failed. Context=%d Variant=%d"), *GetName(), static_cast<int32>(LastResolvedDodgeContext), static_cast<int32>(LastResolvedDodgeVariant));
    }
}

void AMHPlayerCharacter::Input_DodgeCompleted(const FInputActionValue& InputActionValue)
{
    bDodgeHeld = false;
}

void AMHPlayerCharacter::Input_Interact(const FInputActionValue& InputActionValue)
{
    Interact();
}

void AMHPlayerCharacter::Input_AttackPrimary(const FInputActionValue& InputActionValue)
{
    bAttackPrimaryHeld = true;
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForPrimaryInput());
}

void AMHPlayerCharacter::Input_AttackPrimaryCompleted(const FInputActionValue& InputActionValue)
{
    bAttackPrimaryHeld = false;
}

void AMHPlayerCharacter::Input_AttackSecondary(const FInputActionValue& InputActionValue)
{
    bAttackSecondaryHeld = true;
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForSecondaryInput());
}

void AMHPlayerCharacter::Input_AttackSecondaryCompleted(const FInputActionValue& InputActionValue)
{
    bAttackSecondaryHeld = false;
}

void AMHPlayerCharacter::Input_WeaponSpecial(const FInputActionValue& InputActionValue)
{
    bWeaponSpecialHeld = true;
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForWeaponSpecialInput());
}

void AMHPlayerCharacter::Input_WeaponSpecialCompleted(const FInputActionValue& InputActionValue)
{
    bWeaponSpecialHeld = false;
}

void AMHPlayerCharacter::Input_AttackSimultaneous(const FInputActionValue& InputActionValue)
{
    // Mouse5 단일 입력은 베어내리기 계열 전용 진입으로 사용한다.
    // 좌클릭 + 우클릭 동시 입력과 동일한 기술군을 가리키지만,
    // Mouse5 입력은 별도 액션으로 들어오므로 전용 해석 함수를 거친다.
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForAttackSimultaneousInput());
}

void AMHPlayerCharacter::Input_AimHoldStarted(const FInputActionValue& InputActionValue)
{
    bAimHeld = true; //손승우 추가
}

void AMHPlayerCharacter::Input_AimHoldCompleted(const FInputActionValue& InputActionValue)
{
    bAimHeld = false; //손승우 추가
}

void AMHPlayerCharacter::Interact()
{
}

void AMHPlayerCharacter::UsePrimaryAction()
{
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForPrimaryInput());
}

void AMHPlayerCharacter::HandleComboMontageStateTransition(bool bInterrupted)
{
    if (!bPendingUnsheatheFromComboEntry)
    {
        return;
    }

    bPendingUnsheatheFromComboEntry = false;

    if (bInterrupted)
    {
        WeaponSheathState = EMHWeaponSheathState::Sheathed;
        AttachWeaponToBack();
        RefreshWeaponAnimationLayerState();
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Unsheathed;
    AttachWeaponToHand();
    RefreshWeaponAnimationLayerState();
}

void AMHPlayerCharacter::Notify_AttachWeaponToHand()
{
    AttachWeaponToHand();
}

void AMHPlayerCharacter::Notify_AttachWeaponToBack()
{
    AttachWeaponToBack();
}

void AMHPlayerCharacter::Notify_BeginComboChainWindow()
{
    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            ComboComp->SetChainWindowOpen(true);
        }
    }
}

void AMHPlayerCharacter::Notify_EndComboChainWindow()
{
    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            ComboComp->SetChainWindowOpen(false);
        }
    }
}

void AMHPlayerCharacter::Notify_BeginEarlyTransitionWindow()
{
    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            ComboComp->BeginEarlyTransitionWindow();
            TryRequestLongSwordEarlyTransition();
        }
    }
}

void AMHPlayerCharacter::Notify_EndEarlyTransitionWindow()
{
    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            ComboComp->EndEarlyTransitionWindow();
        }
    }
}

void AMHPlayerCharacter::Notify_LongSwordForesightCounterSuccess()
{
    bLongSwordForesightCounterSuccess = true;
}

void AMHPlayerCharacter::ClearLongSwordForesightCounterSuccess()
{
    bLongSwordForesightCounterSuccess = false;
}

bool AMHPlayerCharacter::TryStartAutoSheatheAfterLongSwordMove(const FGameplayTag& CompletedMoveTag)
{
    if (CompletedMoveTag != MHLongSwordGameplayTags::Move_LS_SpiritRoundslash)
    {
        return false;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    StartSheathe();
    return true;
}

void AMHPlayerCharacter::SpawnAndEquipDefaultWeapon()
{
    if (!DefaultWeaponClass)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    EquippedWeapon = World->SpawnActor<AMHWeaponInstance>(DefaultWeaponClass, SpawnParams);
    if (!EquippedWeapon)
    {
        return;
    }

    // 무기 어빌리티 부여
    if (AbilitySystemComponent)
    {
        EquippedWeapon->GrantWeaponAbilities(AbilitySystemComponent);
    }

    CurrentWeaponType = EquippedWeapon->GetWeaponType();
    CurrentWeaponTag = GetCurrentWeaponTypeGameplayTag();

    AttachWeaponActorToBack();
    AttachWeaponToBack();
    RefreshWeaponAnimationLayerState();
}

void AMHPlayerCharacter::AttachWeaponActorToBack()
{
    if (!EquippedWeapon || !GetMesh())
    {
        return;
    }

    EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketConfig.BackSocketName);
}

USkeletalMeshComponent* AMHPlayerCharacter::GetWeaponBladeMesh() const
{
    return EquippedWeapon ? EquippedWeapon->GetWeaponMeshComponent() : nullptr;
}

const FMHWeaponAnimConfig* AMHPlayerCharacter::GetEquippedWeaponAnimConfig() const
{
    return EquippedWeapon ? &EquippedWeapon->GetWeaponAnimConfig() : nullptr;
}

void AMHPlayerCharacter::AttachWeaponToBack()
{
    if (!EquippedWeapon)
    {
        return;
    }

    USkeletalMeshComponent* BladeMesh = GetWeaponBladeMesh();
    if (!BladeMesh)
    {
        return;
    }

    BladeMesh->AttachToComponent(EquippedWeapon->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AMHPlayerCharacter::AttachWeaponToHand()
{
    if (!EquippedWeapon || !GetMesh())
    {
        return;
    }

    USkeletalMeshComponent* BladeMesh = GetWeaponBladeMesh();
    if (!BladeMesh)
    {
        return;
    }

    BladeMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketConfig.HandSocketName);
}

bool AMHPlayerCharacter::IsLongSwordEquipped() const
{
    return CurrentWeaponType == EMHWeaponType::LongSword && Cast<AMHLongSwordInstance>(EquippedWeapon) != nullptr;
}

bool AMHPlayerCharacter::HasMovementInputForCombat() const
{
    return !GetPreferredMoveInput2D().IsNearlyZero() || GetLastMovementInputVector().Size2D() > KINDA_SMALL_NUMBER;
}

bool AMHPlayerCharacter::IsStandingStillForCombat() const
{
    return !HasMovementInputForCombat() && GetVelocity().Size2D() <= 3.0f;
}

bool AMHPlayerCharacter::IsInLongSwordSpecialSheatheState() const
{
    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            return ComboComp->GetCurrentMoveTag() == MHLongSwordGameplayTags::Move_LS_SpecialSheathe;
        }
    }

    return false;
}

bool AMHPlayerCharacter::CanResolveLongSwordFollowupDuringUnsheathing() const
{
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathing)
    {
        return false;
    }

    const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return false;
    }

    const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    return ComboComp && ComboComp->IsComboActive();
}

FVector2D AMHPlayerCharacter::GetPreferredMoveInput2D() const
{
    // 같은 프레임의 입력이 없더라도, 직전에 누르던 방향으로 회피/Variant를 해석할 수 있게 fallback을 둔다.
    if (!CachedMoveInput2D.IsNearlyZero())
    {
        return CachedMoveInput2D;
    }

    return LastNonZeroMoveInput2D;
}

EMHDirectionalVariant AMHPlayerCharacter::ResolveDirectionalVariantFromInput(const bool bPreserveActorFacing) const
{
    const FVector2D MoveInput = GetPreferredMoveInput2D();
    if (MoveInput.IsNearlyZero())
    {
        return EMHDirectionalVariant::Forward;
    }

    const FVector WorldMoveDirection = ResolveWorldMoveDirection(Controller, MoveInput);
    if (WorldMoveDirection.IsNearlyZero())
    {
        return EMHDirectionalVariant::Forward;
    }

    const FVector LocalDirection = bPreserveActorFacing
        ? GetActorTransform().InverseTransformVectorNoScale(WorldMoveDirection)
        : FVector::ForwardVector;

    if (!bPreserveActorFacing)
    {
        return EMHDirectionalVariant::Forward;
    }

    if (FMath::Abs(LocalDirection.X) >= FMath::Abs(LocalDirection.Y))
    {
        return LocalDirection.X >= 0.0f ? EMHDirectionalVariant::Forward : EMHDirectionalVariant::Backward;
    }

    return LocalDirection.Y >= 0.0f ? EMHDirectionalVariant::Right : EMHDirectionalVariant::Left;
}

bool AMHPlayerCharacter::TryRotateActorTowardsMoveInput()
{
    const FVector WorldMoveDirection = ResolveWorldMoveDirection(Controller, GetPreferredMoveInput2D());
    if (WorldMoveDirection.IsNearlyZero())
    {
        return false;
    }

    const FRotator NewActorRotation = WorldMoveDirection.Rotation();
    SetActorRotation(FRotator(0.0f, NewActorRotation.Yaw, 0.0f));
    return true;
}

bool AMHPlayerCharacter::IsLongSwordAttackChainDodgeContext() const
{
    if (!IsLongSwordEquipped() || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            return ComboComp->IsComboActive();
        }
    }

    return false;
}

UAnimMontage* AMHPlayerCharacter::ResolveSheathedRollMontage() const
{
    return SheathedRollMontage.IsNull() ? nullptr : SheathedRollMontage.LoadSynchronous();
}

UAnimMontage* AMHPlayerCharacter::ResolveUnsheathedRollMontage() const
{
    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    if (!AnimConfig)
    {
        return nullptr;
    }

    if (IsLongSwordAttackChainDodgeContext())
    {
        const EMHDirectionalVariant DirectionalVariant = ResolveDirectionalVariantFromInput(true);
        const TSoftObjectPtr<UAnimMontage> VariantMontage = AnimConfig->ChainRollMontages.GetMontageByVariant(DirectionalVariant);
        if (!VariantMontage.IsNull())
        {
            return VariantMontage.LoadSynchronous();
        }
    }

    if (!AnimConfig->NeutralUnsheathedForwardRollMontage.IsNull())
    {
        return AnimConfig->NeutralUnsheathedForwardRollMontage.LoadSynchronous();
    }

    return AnimConfig->UnsheathedRollMontage.IsNull() ? nullptr : AnimConfig->UnsheathedRollMontage.LoadSynchronous();
}

bool AMHPlayerCharacter::TryPlayRollMontage(UAnimMontage* InMontage)
{
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance || !InMontage)
    {
        return false;
    }

    if (bRollMontagePlaying)
    {
        return false;
    }

    LocomotionState = EMHPlayerLocomotionState::Roll;

    const float PlayedLength = AnimInstance->Montage_Play(InMontage);
    if (PlayedLength <= 0.0f)
    {
        UpdateLocomotionState();
        return false;
    }

    bRollMontagePlaying = true;

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleRollMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, InMontage);
    return true;
}

UAnimMontage* AMHPlayerCharacter::ResolveLongSwordMoveMontageOverride(const FGameplayTag& InMoveTag, UAnimMontage* InDefaultMontage) const
{
    if (!IsLongSwordEquipped())
    {
        return InDefaultMontage;
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    if (!AnimConfig)
    {
        return InDefaultMontage;
    }

    // Fade 계열은 입력 방향에 따라 전용 Variant 몽타주를 우선 선택한다.
    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_FadeSlash)
    {
        if (!AnimConfig->FadeSlashBackwardMontage.IsNull())
        {
            return AnimConfig->FadeSlashBackwardMontage.LoadSynchronous();
        }
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_LateralFadeSlash)
    {
        switch (ResolveDirectionalVariantFromInput(true))
        {
        case EMHDirectionalVariant::Left:
            if (!AnimConfig->LateralFadeSlashLeftMontage.IsNull())
            {
                return AnimConfig->LateralFadeSlashLeftMontage.LoadSynchronous();
            }
            break;
        case EMHDirectionalVariant::Right:
            if (!AnimConfig->LateralFadeSlashRightMontage.IsNull())
            {
                return AnimConfig->LateralFadeSlashRightMontage.LoadSynchronous();
            }
            break;
        default:
            break;
        }
    }

    return InDefaultMontage;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForPrimaryInput() const
{
    using namespace MHInputPatternGameplayTags;
    using namespace MHLongSwordGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    // 특수납도 상태 좌클릭은 앉아발도베기로 고정한다.
    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSlash;
    }

    // 납도 상태 좌클릭은 정지/이동 여부에 따라 두 가지 드로우만 허용한다.
    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_DrawAdvancingSlash : InputPattern_LS_DrawOnly;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    // 기인찌르기 이후 좌클릭은 기인투구깨기로 이어진다.
    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            if (ComboComp->IsComboActive() && ComboComp->GetCurrentMoveTag() == Move_LS_SpiritThrust)
            {
                return InputPattern_LS_Helmbreaker;
            }
        }
    }

    // Mouse4 + 좌클릭 = 기인찌르기
    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    // 좌클릭 + 우클릭 = 베어내리기 계열
    if (bAttackSecondaryHeld)
    {
        return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    // 발도 시작/파생의 좌클릭 일반 공격은 현재 노드 기준 기본 파생으로 해석한다.
    // 발도 상태 좌클릭은 현재 콤보 문맥에 따라 실제 일반 공격 입력 패턴으로 해석한다.
    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            const FGameplayTag CurrentMoveTag = ComboComp->GetCurrentMoveTag();

            if (!ComboComp->IsComboActive() || !CurrentMoveTag.IsValid())
            {
                return InputPattern_LS_AdvancingSlash;
            }

            if (CurrentMoveTag == Move_LS_DrawOnly || CurrentMoveTag == Move_LS_ForesightSlash)
            {
                return InputPattern_LS_AdvancingSlash;
            }

            if (CurrentMoveTag == Move_LS_DrawAdvancingSlash || CurrentMoveTag == Move_LS_AdvancingSlash
                || CurrentMoveTag == Move_LS_RisingSlash || CurrentMoveTag == Move_LS_IaiSlash)
            {
                return InputPattern_LS_VerticalSlash;
            }

            if (CurrentMoveTag == Move_LS_VerticalSlash || CurrentMoveTag == Move_LS_FadeSlash
                || CurrentMoveTag == Move_LS_LateralFadeSlash || CurrentMoveTag == Move_LS_SpiritSlash1)
            {
                return InputPattern_LS_Thrust;
            }

            if (CurrentMoveTag == Move_LS_Thrust || CurrentMoveTag == Move_LS_SpiritSlash2
                || CurrentMoveTag == Move_LS_SpiritAdvancingSlash)
            {
                return InputPattern_LS_RisingSlash;
            }
        }
    }

    return FGameplayTag::EmptyTag;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForSecondaryInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    // 특수납도 상태 우클릭은 현재 설계상 별도 파생이 없다.
    if (IsInLongSwordSpecialSheatheState())
    {
        return FGameplayTag::EmptyTag;
    }

    // 납도 상태 우클릭 단독 입력은 유효한 드로우 공격이 없다.
    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    // Mouse4 + 우클릭 = 간파베기
    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    // 좌클릭 + 우클릭 = 베어내리기 계열
    if (bAttackPrimaryHeld)
    {
        return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    // 우클릭 단독은 찌르기 시작/파생 축이다.
    // 기인베기2 / 기인내딛어베기에서는 우클릭도 베어올리기로 해석한다.
    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            const FGameplayTag CurrentMoveTag = ComboComp->GetCurrentMoveTag();

            if (CurrentMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritSlash2
                || CurrentMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritAdvancingSlash)
            {
                return InputPattern_LS_RisingSlash;
            }
        }
    }

    return InputPattern_LS_Thrust;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForWeaponSpecialInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    // 실제 입력 키 txt 기준으로 5번/4번을 서로 치환해서 읽으므로,
    // Input_WeaponSpecial 액션은 Mouse4(기인 축)로 해석한다.

    // 특수납도 상태 Mouse4는 앉아발도기인베기다.
    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSpiritSlash;
    }

    // 납도 상태 Mouse4 단독은 기인베기1 드로우로 연결한다.
    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return InputPattern_LS_DrawSpiritSlash1;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    // Mouse4 + 우클릭 = 간파베기
    if (bAttackSecondaryHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    // Mouse4 + 스페이스 = 특수납도
    if (bDodgeHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    // Mouse4 + 좌클릭 = 기인찌르기
    if (bAttackPrimaryHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    // Mouse4 단독은 기인베기 축이다.
    return InputPattern_LS_Spirit;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForDodgeInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    // 발도 상태에서는 Mouse4 + Space 조합만 특수납도로 사용한다.
    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    return FGameplayTag::EmptyTag;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForCompositeInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    if (IsInLongSwordSpecialSheatheState())
    {
        if (bAttackPrimaryHeld)
        {
            return InputPattern_LS_IaiSlash;
        }

        if (bWeaponSpecialHeld)
        {
            return InputPattern_LS_IaiSpiritSlash;
        }
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    // Mouse4 + Space = 특수납도
    if (bWeaponSpecialHeld && bDodgeHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    // Mouse4 + 우클릭 = 간파베기
    if (bWeaponSpecialHeld && bAttackSecondaryHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    // Mouse4 + 좌클릭 = 기인찌르기
    if (bWeaponSpecialHeld && bAttackPrimaryHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    // 좌클릭 + 우클릭 = 베어내리기 계열
    if (bAttackPrimaryHeld && bAttackSecondaryHeld)
    {
        return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    return FGameplayTag::EmptyTag;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForAttackSimultaneousInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped() || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    // 실제 입력 키 txt 기준으로 4번/5번을 서로 치환해서 읽으므로,
    // Input_AttackSimultaneous 액션은 Mouse5(베어내리기 계열)로 해석한다.
    // 시작 공격에서는 좌우이동베기를 금지하고, 후속 파생 상태에서만 좌우이동베기를 연다.
    return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
}

bool AMHPlayerCharacter::IsLongSwordStartAttackContext() const
{
    if (!IsLongSwordEquipped())
    {
        return false;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return false;
    }

    const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return false;
    }

    return !ComboComp->IsComboActive() || !ComboComp->GetCurrentMoveTag().IsValid();
}

bool AMHPlayerCharacter::IsLongSwordFollowupContext() const
{
    if (!IsLongSwordEquipped())
    {
        return false;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return false;
    }

    const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return false;
    }

    return ComboComp->IsComboActive() && ComboComp->GetCurrentMoveTag().IsValid();
}

bool AMHPlayerCharacter::ShouldUseDirectionalLateralFadeSlash() const
{
    // 좌/우 입력이 실제로 들어왔을 때만 좌우이동베기 Variant를 허용한다.
    const EMHDirectionalVariant DirectionalVariant = ResolveDirectionalVariantFromInput(false);

    return DirectionalVariant == EMHDirectionalVariant::Left
        || DirectionalVariant == EMHDirectionalVariant::Right;
}

bool AMHPlayerCharacter::ShouldUseLateralFadeSlashPattern() const
{
    // 시작 공격에서는 무조건 베어내리기(FadeSlash)만 허용하고,
    // 후속 파생 문맥에서만 좌/우 입력 기반 LateralFadeSlash를 허용한다.
    return IsLongSwordFollowupContext() && ShouldUseDirectionalLateralFadeSlash();
}

FGameplayTag AMHPlayerCharacter::GetCurrentWeaponTypeGameplayTag() const
{
    return CurrentWeaponType == EMHWeaponType::LongSword
        ? MHCombatStateGameplayTags::WeaponType_LongSword
        : FGameplayTag::EmptyTag;
}

FGameplayTag AMHPlayerCharacter::GetCurrentWeaponSheathGameplayTag() const
{
    switch (WeaponSheathState)
    {
    case EMHWeaponSheathState::Sheathed:
        return MHCombatStateGameplayTags::WeaponSheath_Sheathed;
    case EMHWeaponSheathState::Unsheathing:
        return MHCombatStateGameplayTags::WeaponSheath_Unsheathing;
    case EMHWeaponSheathState::Unsheathed:
        return MHCombatStateGameplayTags::WeaponSheath_Unsheathed;
    case EMHWeaponSheathState::Sheathing:
        return MHCombatStateGameplayTags::WeaponSheath_Sheathing;
    default:
        return FGameplayTag::EmptyTag;
    }
}

FGameplayTag AMHPlayerCharacter::GetCurrentCombatStateGameplayTag() const
{
    if (WeaponSheathState == EMHWeaponSheathState::Unsheathing)
    {
        return MHCombatStateGameplayTags::CombatState_Draw;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathing)
    {
        return MHCombatStateGameplayTags::CombatState_Sheathe;
    }

    if (IsInLongSwordSpecialSheatheState())
    {
        return MHCombatStateGameplayTags::CombatState_SpecialSheathe;
    }

    return MHCombatStateGameplayTags::CombatState_None;
}

bool AMHPlayerCharacter::IsLongSwordDrawEntryPattern(const FGameplayTag& InPatternTag) const
{
    return InPatternTag == MHInputPatternGameplayTags::InputPattern_LS_DrawOnly
        || InPatternTag == MHInputPatternGameplayTags::InputPattern_LS_DrawAdvancingSlash
        || InPatternTag == MHInputPatternGameplayTags::InputPattern_LS_DrawSpiritSlash1;
}

bool AMHPlayerCharacter::TryResolveAndHandleLongSwordPattern(const FGameplayTag& PreferredPatternTag)
{
    if (!IsLongSwordEquipped())
    {
        return false;
    }

    const FGameplayTag PatternTag = PreferredPatternTag.IsValid() ? PreferredPatternTag : ResolveLongSwordPatternForCompositeInput();
    if (!PatternTag.IsValid())
    {
        return false;
    }

    return TryHandleWeaponComboInput(PatternTag);
}

bool AMHPlayerCharacter::TryHandleWeaponComboInput(const FGameplayTag& InPatternTag)
{
    if (!EquippedWeapon || !InPatternTag.IsValid())
    {
        return false;
    }

    AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return false;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp || !ComboComp->GetComboGraph())
    {
        return false;
    }

    const bool bComboActive = ComboComp->IsComboActive();
    const bool bDrawEntryFromSheathed =
        !bComboActive
        && WeaponSheathState == EMHWeaponSheathState::Sheathed
        && IsLongSwordDrawEntryPattern(InPatternTag);

    // 발도 시작 공격으로 이미 콤보가 활성화된 상태라면,
    // Unsheathing 중에도 후속 콤보 입력은 허용해야 함.
    const bool bAllowFollowupDuringUnsheathing =
        bComboActive && WeaponSheathState == EMHWeaponSheathState::Unsheathing;

    // 납도 중에는 여전히 입력 차단
    if (WeaponSheathState == EMHWeaponSheathState::Sheathing)
    {
        return false;
    }

    // 발도 중(Unsheathing)에는 "이미 진행 중인 콤보의 후속 입력"만 허용
    if (WeaponSheathState == EMHWeaponSheathState::Unsheathing && !bAllowFollowupDuringUnsheathing)
    {
        return false;
    }

    // 콤보 비활성 + 납도 상태면 발도 시작 공격만 허용
    if (!bComboActive && WeaponSheathState == EMHWeaponSheathState::Sheathed && !bDrawEntryFromSheathed)
    {
        return false;
    }

    // 콤보 비활성 상태에서는 발도 완료(Unsheathed) 상태여야 시작 공격 가능
    // 단, 이미 발도 시작 공격으로 들어와 Unsheathing 중 후속 입력을 넣는 경우는 허용
    if (!bComboActive
        && !bDrawEntryFromSheathed
        && WeaponSheathState != EMHWeaponSheathState::Unsheathed
        && !bAllowFollowupDuringUnsheathing)
    {
        return false;
    }

    if (!ComboComp->BufferInputPattern(InPatternTag))
    {
        return false;
    }

    if (!AbilitySystemComponent)
    {
        if (bDrawEntryFromSheathed)
        {
            HandleComboMontageStateTransition(true);
        }

        ComboComp->ResetCombo();
        return false;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = EquippedWeapon->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        if (bDrawEntryFromSheathed)
        {
            HandleComboMontageStateTransition(true);
        }

        ComboComp->ResetCombo();
        return false;
    }

    if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
    {
        if (Spec->IsActive())
        {
            if (ComboComp->IsEarlyTransitionWindowOpen())
            {
                TryRequestLongSwordEarlyTransition();
            }

            return true;
        }
    }

    if (bDrawEntryFromSheathed)
    {
        WeaponSheathState = EMHWeaponSheathState::Unsheathing;
        bPendingUnsheatheFromComboEntry = true;
    }

    const bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
    if (!bActivated)
    {
        HandleComboMontageStateTransition(true);
        ComboComp->ResetCombo();
    }

    return bActivated;
}

bool AMHPlayerCharacter::TryRequestLongSwordEarlyTransition()
{
    if (!AbilitySystemComponent || !EquippedWeapon)
    {
        return false;
    }

    AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return false;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp || !ComboComp->IsEarlyTransitionWindowOpen())
    {
        return false;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = EquippedWeapon->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        return false;
    }

    FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
    if (!Spec || !Spec->IsActive())
    {
        return false;
    }

    UGameplayAbility* ActiveAbility = Spec->GetPrimaryInstance();
    UMHGA_LongSwordCombo* ComboAbility = Cast<UMHGA_LongSwordCombo>(ActiveAbility);
    if (!ComboAbility)
    {
        return false;
    }

    return ComboAbility->TryEvaluateEarlyTransitionNow();
}

bool AMHPlayerCharacter::CanStartSheathe() const
{
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    const float Speed2D = GetVelocity().Size2D();
    if (Speed2D > 3.0f)
    {
        return false;
    }

    if (const UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        if (AnimInstance->IsAnyMontagePlaying())
        {
            return false;
        }
    }

    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            if (ComboComp->IsComboActive())
            {
                return false;
            }
        }
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    return AnimConfig && !AnimConfig->SheatheMontage.IsNull();
}

void AMHPlayerCharacter::StartSheathe()
{
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return;
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    UAnimMontage* Montage = AnimConfig ? AnimConfig->SheatheMontage.LoadSynchronous() : nullptr;
    if (!Montage)
    {
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Sheathing;

    const float PlayedLen = AnimInstance->Montage_Play(Montage);
    if (PlayedLen <= 0.0f)
    {
        WeaponSheathState = EMHWeaponSheathState::Unsheathed;
        return;
    }

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleSheatheMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
}

void AMHPlayerCharacter::HandleSheatheMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (WeaponSheathState != EMHWeaponSheathState::Sheathing)
    {
        return;
    }

    if (bInterrupted)
    {
        WeaponSheathState = EMHWeaponSheathState::Unsheathed;
        AttachWeaponToHand();
        RefreshWeaponAnimationLayerState();
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Sheathed;
    AttachWeaponToBack();
    RefreshWeaponAnimationLayerState();
}

void AMHPlayerCharacter::HandleRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bRollMontagePlaying = false;
    UpdateLocomotionState();
}

void AMHPlayerCharacter::RefreshWeaponAnimationLayerState()
{
    if (!EquippedWeapon)
    {
        UnlinkCurrentWeaponAnimLayer();
        return;
    }

    LinkCurrentWeaponAnimLayer();
}

void AMHPlayerCharacter::LinkCurrentWeaponAnimLayer()
{
    if (bWeaponAnimLayerLinked)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    const TSoftClassPtr<UAnimInstance> LayerClassPtr = GetCurrentWeaponLinkedAnimLayerClass();
    if (LayerClassPtr.IsNull())
    {
        return;
    }

    TSubclassOf<UAnimInstance> LoadedLayerClass = LayerClassPtr.LoadSynchronous();
    if (!LoadedLayerClass)
    {
        return;
    }

    MeshComp->LinkAnimClassLayers(LoadedLayerClass);
    bWeaponAnimLayerLinked = true;
}

void AMHPlayerCharacter::UnlinkCurrentWeaponAnimLayer()
{
    if (!bWeaponAnimLayerLinked)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        bWeaponAnimLayerLinked = false;
        return;
    }

    const TSoftClassPtr<UAnimInstance> LayerClassPtr = GetCurrentWeaponLinkedAnimLayerClass();
    if (!LayerClassPtr.IsNull())
    {
        TSubclassOf<UAnimInstance> LoadedLayerClass = LayerClassPtr.LoadSynchronous();
        if (LoadedLayerClass)
        {
            MeshComp->UnlinkAnimClassLayers(LoadedLayerClass);
        }
    }

    bWeaponAnimLayerLinked = false;
}

TSoftClassPtr<UAnimInstance> AMHPlayerCharacter::GetCurrentWeaponLinkedAnimLayerClass() const
{
    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    return AnimConfig ? AnimConfig->LinkedWeaponAnimLayerClass : TSoftClassPtr<UAnimInstance>();
}

void AMHPlayerCharacter::ApplyPlayerVisuals()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    MeshComp->SetRelativeLocation(DefaultMeshRelativeLocation);
    MeshComp->SetRelativeRotation(DefaultMeshRelativeRotation);

    if (USkeletalMesh* LoadedMesh = DefaultSkeletalMesh.LoadSynchronous())
    {
        MeshComp->SetSkeletalMesh(LoadedMesh);
    }

    if (UClass* LoadedAnimClass = DefaultAnimClass.LoadSynchronous())
    {
        MeshComp->SetAnimInstanceClass(LoadedAnimClass);
    }
}

void AMHPlayerCharacter::ApplyMovementProfile(EMHPlayerMoveProfile InProfile)
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
    MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;

    switch (InProfile)
    {
    case EMHPlayerMoveProfile::Run:
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
        break;
    case EMHPlayerMoveProfile::Sprint:
        MoveComp->MaxWalkSpeed = MovementConfig.SprintSpeed;
        break;
    default:
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
        break;
    }
}

void AMHPlayerCharacter::UpdateLocomotionState()
{
    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const float Speed2D = GetVelocity().Size2D();

    if (MoveComp && MoveComp->IsFalling())
    {
        // 공중 상태는 추후 확장
        return;
    }

    if (bIsSprinting)
    {
        LocomotionState = EMHPlayerLocomotionState::Sprint;
        return;
    }

    LocomotionState = (Speed2D < 3.0f) ? EMHPlayerLocomotionState::Idle : EMHPlayerLocomotionState::Move;
}

void AMHPlayerCharacter::UpdateStamina(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    if (bIsSprinting)
    {
        CurrentStamina -= StaminaConfig.SprintCostPerSecond * DeltaSeconds;
    }
    else
    {
        CurrentStamina += StaminaConfig.RecoveryPerSecond * DeltaSeconds;
    }

    CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, StaminaConfig.MaxStamina);

    if (bIsSprinting && CurrentStamina <= 0.0f)
    {
        bIsSprinting = false;
        ApplyMovementProfile(EMHPlayerMoveProfile::Run);
    }
}

void AMHPlayerCharacter::EvaluateSprintState()
{
    const bool bHasMoveInput = GetLastMovementInputVector().Size2D() > KINDA_SMALL_NUMBER;
    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const bool bInAir = MoveComp ? MoveComp->IsFalling() : false;

    const bool bWantsSprint = bSprintHeld && bHasMoveInput && !bInAir;

    if (bWantsSprint && CanStartSprint())
    {
        if (!bIsSprinting)
        {
            bIsSprinting = true;
            ApplyMovementProfile(EMHPlayerMoveProfile::Sprint);
        }
        return;
    }

    if (bIsSprinting)
    {
        bIsSprinting = false;
        ApplyMovementProfile(EMHPlayerMoveProfile::Run);
    }
}

bool AMHPlayerCharacter::CanStartSprint() const
{
    return CurrentStamina > StaminaConfig.LowStaminaThreshold;
}

bool AMHPlayerCharacter::TryEnterSlide()
{
    return false;
}

bool AMHPlayerCharacter::TryStartClimb()
{
    return false;
}

bool AMHPlayerCharacter::TryJumpOffLedge()
{
    return false;
}

void AMHPlayerCharacter::HandleLandingState()
{
}
