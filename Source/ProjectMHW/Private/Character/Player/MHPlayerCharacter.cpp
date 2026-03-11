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
#include "GameplayTags/MHCombatStateGameplayTags.h"
#include "GameplayTags/MHInputPatternGameplayTags.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/Input/MHInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHPlayerCharacter);

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

    //매시 세팅
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Assets/Player/Character/Legiana_β/SkeletonMesh/Legiana_β.Legiana_β"));

    if (MeshAsset.Succeeded()) {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
    }

    //매시 초기위치 세팅 Z방향으로 -100 내리고 Yaw축으로 -90도 회전
    GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -7.0f), FRotator(0, -90.0f, 0));
    
    // 데칼 영향 제거
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->bReceivesDecals = false;
    }
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

    if (TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForDodgeInput()))
    {
        return;
    }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return;
    }

    UAnimMontage* RollMontage = nullptr;

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        RollMontage = SheathedRollMontage.LoadSynchronous();
    }
    else if (WeaponSheathState == EMHWeaponSheathState::Unsheathed)
    {
        const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
        RollMontage = AnimConfig ? AnimConfig->UnsheathedRollMontage.LoadSynchronous() : nullptr;
    }

    if (!RollMontage)
    {
        return;
    }

    LocomotionState = EMHPlayerLocomotionState::Roll;

    const float PlayedLen = AnimInstance->Montage_Play(RollMontage);
    if (PlayedLen <= 0.0f)
    {
        UpdateLocomotionState();
        return;
    }

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleRollMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
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
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForCompositeInput());
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
        UpdateAnimClassByWeaponState();
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Unsheathed;
    AttachWeaponToHand();
    UpdateAnimClassByWeaponState();
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
    UpdateAnimClassByWeaponState();
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
    return GetLastMovementInputVector().Size2D() > KINDA_SMALL_NUMBER;
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

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForPrimaryInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSlash;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_DrawAdvancingSlash : InputPattern_LS_DrawOnly;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bAttackSecondaryHeld || bAimHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    if (bWeaponSpecialHeld)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    return InputPattern_LS_Basic;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForSecondaryInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped())
    {
        return FGameplayTag::EmptyTag;
    }

    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSpiritSlash;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return InputPattern_LS_DrawSpiritSlash1;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    if (bDodgeHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    if (bAttackPrimaryHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    return InputPattern_LS_Spirit;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForWeaponSpecialInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped() || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bAttackSecondaryHeld || bAimHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    if (bAttackPrimaryHeld)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    return InputPattern_LS_Thrust;
}

FGameplayTag AMHPlayerCharacter::ResolveLongSwordPatternForDodgeInput() const
{
    using namespace MHInputPatternGameplayTags;

    if (!IsLongSwordEquipped() || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bAttackSecondaryHeld || bAimHeld)
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

        if (bAttackSecondaryHeld || bAimHeld)
        {
            return InputPattern_LS_IaiSpiritSlash;
        }
    }

    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    if ((bAttackSecondaryHeld || bAimHeld) && bDodgeHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    if ((bAttackSecondaryHeld || bAimHeld) && bWeaponSpecialHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    if ((bAttackSecondaryHeld || bAimHeld) && bAttackPrimaryHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    if (bAttackPrimaryHeld && bWeaponSpecialHeld)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

    return FGameplayTag::EmptyTag;
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

    if (WeaponSheathState == EMHWeaponSheathState::Unsheathing || WeaponSheathState == EMHWeaponSheathState::Sheathing)
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
    const bool bDrawEntryFromSheathed = !bComboActive && WeaponSheathState == EMHWeaponSheathState::Sheathed && IsLongSwordDrawEntryPattern(InPatternTag);

    if (!bComboActive && WeaponSheathState == EMHWeaponSheathState::Sheathed && !bDrawEntryFromSheathed)
    {
        return false;
    }

    if (!bDrawEntryFromSheathed && WeaponSheathState != EMHWeaponSheathState::Unsheathed)
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
        UpdateAnimClassByWeaponState();
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Sheathed;
    AttachWeaponToBack();
    UpdateAnimClassByWeaponState();
}

void AMHPlayerCharacter::HandleRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UpdateLocomotionState();
}

void AMHPlayerCharacter::UpdateAnimClassByWeaponState()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    const bool bIsSheathed = (WeaponSheathState == EMHWeaponSheathState::Sheathed) || (WeaponSheathState == EMHWeaponSheathState::Unsheathing) || (WeaponSheathState == EMHWeaponSheathState::Sheathing);

    if (bIsSheathed)
    {
        if (UClass* LoadedAnimClass = DefaultAnimClass.LoadSynchronous())
        {
            MeshComp->SetAnimInstanceClass(LoadedAnimClass);
        }
        return;
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    if (AnimConfig && !AnimConfig->UnsheathedAnimClass.IsNull())
    {
        if (UClass* LoadedAnimClass = AnimConfig->UnsheathedAnimClass.LoadSynchronous())
        {
            MeshComp->SetAnimInstanceClass(LoadedAnimClass);
        }
    }
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
