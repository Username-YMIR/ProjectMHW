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

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/Input/MHInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHPlayerCharacter);

AMHPlayerCharacter::AMHPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 회전 사용 비활성화
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    // 스프링암
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 260.0f;
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
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Interact))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Interact, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_Interact);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackPrimary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackPrimary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackPrimary);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackSecondary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSecondary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackSecondary);
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
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return;
    }

    UAnimMontage* RollMontage = nullptr;

    // 납도는 공통 구르기
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

    // 구르기 중 이동 입력 차단
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    LocomotionState = EMHPlayerLocomotionState::Roll;

    const float PlayedLen = AnimInstance->Montage_Play(RollMontage);
    if (PlayedLen <= 0.0f)
    {
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
        UpdateLocomotionState();
        return;
    }

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleRollMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
}

void AMHPlayerCharacter::Input_Interact(const FInputActionValue& InputActionValue)
{
    Interact();
}

void AMHPlayerCharacter::Input_AttackPrimary(const FInputActionValue& InputActionValue)
{
    // 납도 상태 + 정지 상태면 발도 공격 우선
    if (CanStartDrawAttack())
    {
        StartDrawAttack();
        return;
    }

    UsePrimaryAction();
}

void AMHPlayerCharacter::Input_AttackSecondary(const FInputActionValue& InputActionValue)
{
    if (CanStartSheathe())
    {
        StartSheathe();
        return;
    }

    if (!EquippedWeapon || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return;
    }

    AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return;
    }

    ComboComp->BufferInput(EMHComboInputType::Secondary);

    if (!AbilitySystemComponent)
    {
        return;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = EquippedWeapon->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        return;
    }

    if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
    {
        if (Spec->IsActive())
        {
            return;
        }
    }

    AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
}

void AMHPlayerCharacter::Interact()
{
}

void AMHPlayerCharacter::UsePrimaryAction()
{
    if (!EquippedWeapon)
    {
        return;
    }

    // 발도 상태에서만 콤보 시작/연계
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return;
    }

    AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon);
    if (!LongSword)
    {
        return;
    }

    UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent();
    if (!ComboComp)
    {
        return;
    }

    // 입력 버퍼 저장
    ComboComp->BufferInput(EMHComboInputType::Primary);

    // 어빌리티가 없으면 시작 불가
    if (!AbilitySystemComponent)
    {
        return;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = EquippedWeapon->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        return;
    }

    // 이미 실행 중이면 버퍼만 저장
    if (FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
    {
        if (Spec->IsActive())
        {
            return;
        }
    }

    AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
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

bool AMHPlayerCharacter::CanStartDrawAttack() const
{
    // 납도 상태에서만 발도 가능
    if (WeaponSheathState != EMHWeaponSheathState::Sheathed)
    {
        return false;
    }

    // 정지 상태에서만 발도 공격(요구사항)
    const float Speed2D = GetVelocity().Size2D();
    if (Speed2D > 3.0f)
    {
        return false;
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    return AnimConfig && !AnimConfig->DrawMontage.IsNull();
}

void AMHPlayerCharacter::StartDrawAttack()
{
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return;
    }

    const FMHWeaponAnimConfig* AnimConfig = GetEquippedWeaponAnimConfig();
    UAnimMontage* Montage = AnimConfig ? AnimConfig->DrawMontage.LoadSynchronous() : nullptr;
    if (!Montage)
    {
        return;
    }

    // 발도 진행 상태
    WeaponSheathState = EMHWeaponSheathState::Unsheathing;

    const float PlayedLen = AnimInstance->Montage_Play(Montage);
    if (PlayedLen <= 0.0f)
    {
        WeaponSheathState = EMHWeaponSheathState::Sheathed;
        return;
    }

    // 몽타주 종료 시 상태 확정
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleDrawMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
}

void AMHPlayerCharacter::HandleDrawMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathing)
    {
        return;
    }

    // 발도 중단 시 납도 상태로 복귀
    if (bInterrupted)
    {
        WeaponSheathState = EMHWeaponSheathState::Sheathed;
        AttachWeaponToBack();
        UpdateAnimClassByWeaponState();
        return;
    }

    WeaponSheathState = EMHWeaponSheathState::Unsheathed;

    UpdateAnimClassByWeaponState();
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
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
    }

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