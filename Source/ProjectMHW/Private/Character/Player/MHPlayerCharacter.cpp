#include "Character/Player/MHPlayerCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/mh_hit_enemy_camera_shake.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
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
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "Combat/Effects/MHGameplayEffect_PlayerDamage.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Combat/mh_attack_definition_library.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Items/Effects/MHGameplayEffect_WeaponStat.h"

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

    static FMHHitAcknowledge BuildLongSwordCounterAcknowledge()
    {
        FMHHitAcknowledge Result;
        Result.bAcceptedHit = true;
        Result.bConsumeHitOnce = true;
        Result.bShouldStopAttackWindow = false;
        Result.ResultType = EMHHitResultType::Invulnerable;
        return Result;
    }

    static const TCHAR* ResolveLongSwordResourceCommitTypeText(const EMHLongSwordResourceCommitType InCommitType)
    {
        switch (InCommitType)
        {
        case EMHLongSwordResourceCommitType::FirstValidHit:
            return TEXT("FirstValidHit");
        case EMHLongSwordResourceCommitType::FinisherHit:
            return TEXT("FinisherHit");
        case EMHLongSwordResourceCommitType::CounterSuccess:
            return TEXT("CounterSuccess");
        default:
            return TEXT("None");
        }
    }
}

AMHPlayerCharacter::AMHPlayerCharacter()
{
    InitializeCapsuleSettings();
    
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

    DebugDamageEffectClass = UMHGameplayEffect_Damage::StaticClass();
    PlayerIncomingDamageEffectClass = UMHGameplayEffect_PlayerDamage::StaticClass();
    // DebugIncomingAttackTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Attack.Debug.Counterable")), false);
    WeaponStatEffectClass = UMHGameplayEffect_WeaponStat::StaticClass();

    // 플레이어 기본 스태미나는 현재 프로젝트 기준으로 100으로 고정한다.
    StaminaConfig.MaxStamina = 100.0f;
}

void AMHPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : BeginPlay"), *GetName());

    ApplyPlayerVisuals();

    SpawnAndEquipDefaultWeapon();

    SyncStaminaAttributesFromConfig();
    ApplyDefaultPlayerAttributes();

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

    // 입력 액션 자산과 별개로 디버그 피격 키를 직접 바인딩해서 간파/거합 검증에 사용한다.
    PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AMHPlayerCharacter::Input_DebugIncomingDamageKeyPressed);
}

void AMHPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 무브먼트 업데이트 델리게이트 해제
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        OnCharacterMovementUpdated.RemoveDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }

    // 무기 해제 함수화 _ 이건주
    UnequipCurrentWeapon(false);
    
    // // 무기 어빌리티 해제
    // if (EquippedWeapon && AbilitySystemComponent)
    // {
    //     EquippedWeapon->ClearWeaponAbilities(AbilitySystemComponent);
    // }

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

    if (UWorld* World = GetWorld())
    {
        UpdateDirectionalTurnWindow(World->GetDeltaSeconds());
    }

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

void AMHPlayerCharacter::Input_DebugIncomingDamageKeyPressed()
{
    const FGameplayTag AttackTagToUse = DebugIncomingAttackTag.IsValid()
        ? DebugIncomingAttackTag
        : FGameplayTag::RequestGameplayTag(FName(TEXT("Attack.Debug.Counterable")), false);

    if (!AttackTagToUse.IsValid())
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : 디버그 피격 키 입력을 처리할 수 없습니다. Attack.Debug.Counterable 태그를 찾지 못했습니다."), *GetName());
        return;
    }

    ApplyDebugDamageFromSource(this, DebugIncomingPhysicalDamage, AttackTagToUse);

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("%s : 디버그 피격 키 입력 처리. Physical=%.2f AttackTag=%s"),
        *GetName(),
        DebugIncomingPhysicalDamage,
        *AttackTagToUse.ToString()
    );
}

void AMHPlayerCharacter::ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage)
{
    ApplyDebugDamageFromSource(InSourceActor, InPhysicalDamage, DebugIncomingAttackTag);
}

void AMHPlayerCharacter::ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage, const FGameplayTag& InAttackTag)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : Debug damage skipped. AbilitySystemComponent is null."), *GetName());
        return;
    }

    UAbilitySystemComponent* SourceASC = nullptr;
    if (IsValid(InSourceActor))
    {
        SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InSourceActor);
    }

    if (!SourceASC)
    {
        SourceASC = AbilitySystemComponent;
    }

    if (!DebugDamageEffectClass)
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : Debug damage skipped. DebugDamageEffectClass is null."), *GetName());
        return;
    }

    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(IsValid(InSourceActor) ? InSourceActor : this);

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DebugDamageEffectClass, 1.0f, EffectContext);
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : Debug damage spec creation failed."), *GetName());
        return;
    }

    const float PhysicalDamage = FMath::Max(0.0f, InPhysicalDamage);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Physical, PhysicalDamage);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Fire, 0.0f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Water, 0.0f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Thunder, 0.0f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Ice, 0.0f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Dragon, 0.0f);

    const FMHHitAcknowledge HitAcknowledge = ReceiveDamageSpec_Implementation(
        IsValid(InSourceActor) ? InSourceActor : this,
        nullptr,
        InAttackTag,
        SpecHandle,
        FHitResult()
    );

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("%s : Debug self damage requested. Source=%s Physical=%.2f AttackTag=%s Accepted=%s ResultType=%d"),
        *GetName(),
        *GetNameSafe(InSourceActor),
        PhysicalDamage,
        InAttackTag.IsValid() ? *InAttackTag.ToString() : TEXT("None"),
        HitAcknowledge.bAcceptedHit ? TEXT("true") : TEXT("false"),
        static_cast<int32>(HitAcknowledge.ResultType)
    );
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


void AMHPlayerCharacter::UsePrimaryAction()
{
    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForPrimaryInput());
}

#pragma region WeaponAndLongSwordRuntime
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

void AMHPlayerCharacter::Notify_BeginDirectionalTurnWindow(float InMaxYawDeltaDegrees, float InRotationInterpSpeed)
{
    bDirectionalTurnWindowActive = true;
    DirectionalTurnWindowBaseYaw = GetActorRotation().Yaw;
    DirectionalTurnWindowMaxYawDeltaDegrees = FMath::Max(0.0f, InMaxYawDeltaDegrees);
    DirectionalTurnWindowRotationInterpSpeed = FMath::Max(0.0f, InRotationInterpSpeed);

    if (UWorld* World = GetWorld())
    {
        UpdateDirectionalTurnWindow(World->GetDeltaSeconds());
    }
}

void AMHPlayerCharacter::Notify_EndDirectionalTurnWindow()
{
    bDirectionalTurnWindowActive = false;
    DirectionalTurnWindowMaxYawDeltaDegrees = 0.0f;
    DirectionalTurnWindowRotationInterpSpeed = 0.0f;
}

void AMHPlayerCharacter::Notify_LongSwordForesightCounterSuccess()
{
    bLongSwordForesightCounterSuccess = true;
}

void AMHPlayerCharacter::ClearLongSwordForesightCounterSuccess()
{
    bLongSwordForesightCounterSuccess = false;
}

void AMHPlayerCharacter::Notify_LongSwordSpecialSheatheSlashCounterSuccess()
{
    bLongSwordSpecialSheatheSlashCounterSuccess = true;
}

void AMHPlayerCharacter::ClearLongSwordSpecialSheatheSlashCounterSuccess()
{
    bLongSwordSpecialSheatheSlashCounterSuccess = false;
}

void AMHPlayerCharacter::Notify_LongSwordSpecialSheatheSpiritCounterSuccess()
{
    bLongSwordSpecialSheatheSpiritCounterSuccess = true;
}

void AMHPlayerCharacter::ClearLongSwordSpecialSheatheSpiritCounterSuccess()
{
    bLongSwordSpecialSheatheSpiritCounterSuccess = false;
}

void AMHPlayerCharacter::ClearLongSwordCounterSuccessFlagsForMoveExit(const FGameplayTag InMoveTag)
{
    if (!InMoveTag.IsValid())
    {
        return;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash)
    {
        ClearLongSwordForesightCounterSuccess();
        return;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSlash)
    {
        ClearLongSwordSpecialSheatheSlashCounterSuccess();
        return;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash)
    {
        ClearLongSwordSpecialSheatheSpiritCounterSuccess();
    }
}

void AMHPlayerCharacter::ClearAllLongSwordCounterSuccessFlags()
{
    ClearLongSwordForesightCounterSuccess();
    ClearLongSwordSpecialSheatheSlashCounterSuccess();
    ClearLongSwordSpecialSheatheSpiritCounterSuccess();
}

void AMHPlayerCharacter::Notify_LongSwordAttackHitConfirmed(const FGameplayTag& InMoveTag)
{
    if (!IsLongSwordEquipped() || !InMoveTag.IsValid())
    {
        return;
    }

    const EMHLongSwordResourceCommitType CommitType = ResolveLongSwordResourceCommitType(InMoveTag);
    if (CommitType == EMHLongSwordResourceCommitType::None
        || CommitType == EMHLongSwordResourceCommitType::CounterSuccess)
    {
        return;
    }

    CommitLongSwordResourceDelta(InMoveTag, CommitType);

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSlash)
    {
        ClearLongSwordSpecialSheatheSlashCounterSuccess();
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash)
    {
        ClearLongSwordSpecialSheatheSpiritCounterSuccess();
    }
}

void AMHPlayerCharacter::Notify_LongSwordCounterCommitSuccess(const EMHLongSwordCounterWindowType InCounterWindowType)
{
    // 카운터 성공 시점에 확정해야 하는 기술만 여기서 자원을 처리한다.
    if (!IsLongSwordEquipped())
    {
        return;
    }

    const FGameplayTag CurrentMoveTag = GetCurrentLongSwordMoveTag();
    if (!CurrentMoveTag.IsValid())
    {
        return;
    }

    const EMHLongSwordResourceCommitType CommitType = ResolveLongSwordResourceCommitType(CurrentMoveTag);
    if (CommitType != EMHLongSwordResourceCommitType::CounterSuccess)
    {
        return;
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword counter commit resolved. Window=%d Move=%s"),
        *GetName(),
        static_cast<int32>(InCounterWindowType),
        *CurrentMoveTag.ToString()
    );

    CommitLongSwordResourceDelta(CurrentMoveTag, CommitType);
}

void AMHPlayerCharacter::Notify_BeginLongSwordCounterWindow(const EMHLongSwordCounterWindowType InCounterWindowType)
{
    ActiveLongSwordCounterWindowType = InCounterWindowType;

    switch (InCounterWindowType)
    {
    case EMHLongSwordCounterWindowType::Foresight:
        ClearLongSwordForesightCounterSuccess();
        break;
    case EMHLongSwordCounterWindowType::SpecialSheatheSlash:
        ClearLongSwordSpecialSheatheSlashCounterSuccess();
        break;
    case EMHLongSwordCounterWindowType::SpecialSheatheSpirit:
        ClearLongSwordSpecialSheatheSpiritCounterSuccess();
        break;
    default:
        break;
    }
}

void AMHPlayerCharacter::Notify_EndLongSwordCounterWindow(const EMHLongSwordCounterWindowType InCounterWindowType)
{
    if (ActiveLongSwordCounterWindowType == InCounterWindowType)
    {
        ActiveLongSwordCounterWindowType = EMHLongSwordCounterWindowType::None;
    }
}

FMHHitAcknowledge AMHPlayerCharacter::ReceiveDamageSpec_Implementation(
    AActor* SourceActor,
    AActor* SourceWeapon,
    FGameplayTag AttackTag,
    const FGameplayEffectSpecHandle& DamageSpecHandle,
    const FHitResult& HitResult)
{
    
    if (!ValidateDamageSpec(DamageSpecHandle))
    {
        HandleDamageRejected(SourceActor, SourceWeapon, AttackTag, HitResult);
        return BuildRejectedHitAcknowledge();
    }

    ClearExpiredLongSwordDamageIgnoreState();

    if (bIgnoreDamageUntilCurrentActionEnd)
    {
        UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Ignore incoming damage until current action ends. MoveTag=%s"),
            *GetName(),
            *DamageIgnoreUntilCurrentMoveTag.ToString());
        return BuildLongSwordInvulnerableHitAcknowledge();
    }

    if (CanTriggerLongSwordForesightCounter() && IsAttackAllowedForForesightCounter(AttackTag))
    {
        Notify_LongSwordForesightCounterSuccess();
        Notify_LongSwordCounterCommitSuccess(EMHLongSwordCounterWindowType::Foresight);
        UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : Foresight counter success. AttackTag=%s"), *GetName(), *AttackTag.ToString());
        return BuildLongSwordInvulnerableHitAcknowledge();
    }

    if (CanTriggerLongSwordSpecialSheatheSlashCounter() && IsAttackAllowedForSpecialSheatheCounter(AttackTag))
    {
        Notify_LongSwordSpecialSheatheSlashCounterSuccess();
        Notify_LongSwordCounterCommitSuccess(EMHLongSwordCounterWindowType::SpecialSheatheSlash);
        bIgnoreDamageUntilCurrentActionEnd = true;
        DamageIgnoreUntilCurrentMoveTag = GetCurrentLongSwordMoveTag();
        UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : Special sheathe slash counter success. AttackTag=%s MoveTag=%s"),
            *GetName(),
            *AttackTag.ToString(),
            *DamageIgnoreUntilCurrentMoveTag.ToString());
        return BuildLongSwordInvulnerableHitAcknowledge();
    }

    if (CanTriggerLongSwordSpecialSheatheSpiritCounter() && IsAttackAllowedForSpecialSheatheCounter(AttackTag))
    {
        Notify_LongSwordSpecialSheatheSpiritCounterSuccess();
        Notify_LongSwordCounterCommitSuccess(EMHLongSwordCounterWindowType::SpecialSheatheSpirit);
        bIgnoreDamageUntilCurrentActionEnd = true;
        DamageIgnoreUntilCurrentMoveTag = GetCurrentLongSwordMoveTag();
        UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : Special sheathe spirit counter success. AttackTag=%s MoveTag=%s"),
            *GetName(),
            *AttackTag.ToString(),
            *DamageIgnoreUntilCurrentMoveTag.ToString());
        return BuildLongSwordInvulnerableHitAcknowledge();
    }

    return Super::ReceiveDamageSpec_Implementation(SourceActor, SourceWeapon, AttackTag, DamageSpecHandle, HitResult);
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

#pragma region WeaponRuntimeFunctions
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
    
    // ==========================
    // 무기 장착 로직 함수화 (무기 교체 기능 지원)_이건주
    AMHWeaponInstance* SpawnedWeapon = World->SpawnActor<AMHWeaponInstance>(DefaultWeaponClass, SpawnParams);
    if (!SpawnedWeapon)
    {
        return;
    }

    EquipWeaponInstance(SpawnedWeapon, true);

    // 장착GE_장착 직후 _이건주
    UE_LOG(LogTemp, Warning, TEXT("[Equip] Weapon=%s AP=%.2f CR=%.2f"),
    *GetNameSafe(EquippedWeapon),
    CombatAttributeSet ? CombatAttributeSet->GetAttackPower() : -1.f,
    CombatAttributeSet ? CombatAttributeSet->GetCriticalRate() : -1.f);

    
    //===========================
    
    // EquippedWeapon = World->SpawnActor<AMHWeaponInstance>(DefaultWeaponClass, SpawnParams);
    // if (!EquippedWeapon)
    // {
    //     return;
    // }
    //
    // // 무기 어빌리티 부여
    // if (AbilitySystemComponent)
    // {
    //     EquippedWeapon->GrantWeaponAbilities(AbilitySystemComponent);
    // }
    //
    // CurrentWeaponType = EquippedWeapon->GetWeaponType();
    // CurrentWeaponTag = GetCurrentWeaponTypeGameplayTag();
    //
    // AttachWeaponActorToBack();
    // AttachWeaponToBack();
    // RefreshWeaponAnimationLayerState();
    //
    // //무기 스탯 GE 적용 _ 이건주
    // RefreshEquippedWeaponStatEffect();
}

bool AMHPlayerCharacter::EquipWeaponInstance(AMHWeaponInstance* InWeapon, bool bDestroyPreviousWeapon)
{
    if (!IsValid(InWeapon))
    {
        return false;
    }

    // 현제 무기 해제
    UnequipCurrentWeapon(bDestroyPreviousWeapon);

    // 새 무기 캐싱
    EquippedWeapon = InWeapon;
    EquippedWeapon->SetOwner(this);

    if (AbilitySystemComponent)
    {
        // 새 무기의 어빌리티 적용
        EquippedWeapon->GrantWeaponAbilities(AbilitySystemComponent);
    }

    WeaponSheathState = EMHWeaponSheathState::Sheathed;
    CurrentWeaponType = EquippedWeapon->GetWeaponType();
    CurrentWeaponTag = GetCurrentWeaponTypeGameplayTag();

    AttachWeaponActorToBack();
    AttachWeaponToBack();
    RefreshWeaponAnimationLayerState();
    
    // 무기 스탯 GE 적용
    RefreshEquippedWeaponStatEffect();

    return true;
}

void AMHPlayerCharacter::UnequipCurrentWeapon(bool bDestroyWeapon)
{
    RemoveEquippedWeaponStatEffect();

    if (EquippedWeapon && AbilitySystemComponent)
    {
        EquippedWeapon->ClearWeaponAbilities(AbilitySystemComponent);
    }

    if (bDestroyWeapon && EquippedWeapon)
    {
        EquippedWeapon->Destroy();
    }

    EquippedWeapon = nullptr;
    CurrentWeaponType = EMHWeaponType::None;
    CurrentWeaponTag = FGameplayTag();
    CurrentWeaponElementTag = FGameplayTag();
    CurrentSharpnessColor = EMHSharpnessColor::Red;
    CurrentSharpnessValue = 0.0f;

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

#pragma endregion

#pragma region LongSwordRuntimeFunctions
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

FGameplayTag AMHPlayerCharacter::GetCurrentLongSwordMoveTag() const
{
    if (const AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (const UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            return ComboComp->GetCurrentMoveTag();
        }
    }

    return FGameplayTag();
}

void AMHPlayerCharacter::ClearExpiredLongSwordDamageIgnoreState()
{
    if (!bIgnoreDamageUntilCurrentActionEnd)
    {
        return;
    }

    const FGameplayTag CurrentMoveTag = GetCurrentLongSwordMoveTag();
    if (CurrentMoveTag.IsValid() && CurrentMoveTag == DamageIgnoreUntilCurrentMoveTag)
    {
        return;
    }

    bIgnoreDamageUntilCurrentActionEnd = false;
    DamageIgnoreUntilCurrentMoveTag = FGameplayTag();
}

bool AMHPlayerCharacter::CanTriggerLongSwordForesightCounter() const
{
    return ActiveLongSwordCounterWindowType == EMHLongSwordCounterWindowType::Foresight
        && GetCurrentLongSwordMoveTag() == MHLongSwordGameplayTags::Move_LS_ForesightSlash;
}

bool AMHPlayerCharacter::CanTriggerLongSwordSpecialSheatheSlashCounter() const
{
    return ActiveLongSwordCounterWindowType == EMHLongSwordCounterWindowType::SpecialSheatheSlash
        && GetCurrentLongSwordMoveTag() == MHLongSwordGameplayTags::Move_LS_IaiSlash;
}

bool AMHPlayerCharacter::CanTriggerLongSwordSpecialSheatheSpiritCounter() const
{
    return ActiveLongSwordCounterWindowType == EMHLongSwordCounterWindowType::SpecialSheatheSpirit
        && GetCurrentLongSwordMoveTag() == MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash;
}

const FMHAttackDefinitionRow* AMHPlayerCharacter::FindAttackDefinitionRow(const FGameplayTag& InAttackTag) const
{
    return UMHAttackDefinitionLibrary::FindAttackDefinitionRowPtr(AttackDefinitionTable, InAttackTag);
}

bool AMHPlayerCharacter::FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const
{
    if (!IsValid(LongSwordAttackMetaTable))
    {
        return false;
    }

    return UMHCombatDataLibrary::FindAttackMetaRowByTag(LongSwordAttackMetaTable, InMoveTag, OutAttackMetaRow);
}

bool AMHPlayerCharacter::CanStartLongSwordMove(const FGameplayTag& InMoveTag) const
{
    if (!InMoveTag.IsValid())
    {
        return false;
    }

    FMHAttackMetaRow AttackMetaRow;
    if (!FindAttackMetaRow(InMoveTag, AttackMetaRow))
    {
        return true;
    }

    const float RequiredSpiritGauge = FMath::Max(0.0f, AttackMetaRow.SpiritGaugeConsume);
    if (RequiredSpiritGauge <= 0.0f)
    {
        return true;
    }

    return CurrentSpiritGauge + KINDA_SMALL_NUMBER >= RequiredSpiritGauge;
}

void AMHPlayerCharacter::PlayLongSwordHitCameraShake(const FGameplayTag& InMoveTag) const
{
    if (!Controller)
    {
        return;
    }

    const APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC || !PC->IsLocalController() || !PC->PlayerCameraManager)
    {
        return;
    }

    FMHAttackMetaRow AttackMetaRow;
    TSubclassOf<UCameraShakeBase> ShakeClass = UMHHitEnemyCameraShake::StaticClass();
    float ShakeScale = 1.0f;

    if (FindAttackMetaRow(InMoveTag, AttackMetaRow))
    {
        if (!AttackMetaRow.CameraShakeClass.IsNull())
        {
            if (UClass* LoadedShakeClass = AttackMetaRow.CameraShakeClass.LoadSynchronous())
            {
                ShakeClass = LoadedShakeClass;
            }
        }

        ShakeScale = FMath::Max(0.0f, AttackMetaRow.CameraShakeScale);
    }

    if (!ShakeClass || ShakeScale <= 0.0f)
    {
        return;
    }

    PC->PlayerCameraManager->StartCameraShake(ShakeClass, ShakeScale);
}

EMHLongSwordResourceCommitType AMHPlayerCharacter::ResolveLongSwordResourceCommitType(const FGameplayTag& InMoveTag) const
{
    // 일반 기술은 첫 유효 타격, 일부 피니시 기술은 마지막 확정 타격, 간파는 카운터 성공 시점으로 분리한다.
    if (!InMoveTag.IsValid())
    {
        return EMHLongSwordResourceCommitType::None;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash)
    {
        return EMHLongSwordResourceCommitType::CounterSuccess;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritRoundslash
        || InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker)
    {
        return EMHLongSwordResourceCommitType::FinisherHit;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_DrawOnly
        || InMoveTag == MHLongSwordGameplayTags::Move_LS_SpecialSheathe)
    {
        return EMHLongSwordResourceCommitType::None;
    }

    return EMHLongSwordResourceCommitType::FirstValidHit;
}

void AMHPlayerCharacter::CommitLongSwordResourceDelta(const FGameplayTag& InMoveTag, const EMHLongSwordResourceCommitType InCommitType)
{
    // 실제 수치 변화는 이 함수 한곳에서만 처리해서 기술별 예외를 모은다.
    if (!InMoveTag.IsValid())
    {
        return;
    }

    FMHAttackMetaRow AttackMetaRow;
    if (!FindAttackMetaRow(InMoveTag, AttackMetaRow))
    {
        return;
    }

    const float PreviousSpiritGauge = CurrentSpiritGauge;
    const int32 PreviousSpiritLevel = CurrentSpiritLevel;

    const bool bShouldIgnoreSpiritConsume =
        bLongSwordForesightCounterSuccess
        || bLongSwordSpecialSheatheSlashCounterSuccess
        || bLongSwordSpecialSheatheSpiritCounterSuccess;

    if (InCommitType != EMHLongSwordResourceCommitType::CounterSuccess)
    {
        AddSpiritGauge(FMath::Max(0.0f, AttackMetaRow.SpiritGaugeGain));
    }

    if (InCommitType == EMHLongSwordResourceCommitType::CounterSuccess)
    {
        ConsumeSpiritGauge(FMath::Max(0.0f, AttackMetaRow.SpiritGaugeConsume));
    }
    else if (!bShouldIgnoreSpiritConsume)
    {
        ConsumeSpiritGauge(FMath::Max(0.0f, AttackMetaRow.SpiritGaugeConsume));
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritRoundslash)
    {
        IncreaseSpiritLevel();
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker)
    {
        DecreaseSpiritLevel();
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash && !bLongSwordSpecialSheatheSpiritCounterSuccess)
    {
        DecreaseSpiritLevel();
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword resource committed. Move=%s CommitType=%s Gauge=%.2f->%.2f Level=%d->%d"),
        *GetName(),
        *InMoveTag.ToString(),
        ResolveLongSwordResourceCommitTypeText(InCommitType),
        PreviousSpiritGauge,
        CurrentSpiritGauge,
        PreviousSpiritLevel,
        CurrentSpiritLevel
    );
}

float AMHPlayerCharacter::GetCurrentSpiritDamageMultiplier() const
{
    switch (FMath::Clamp(CurrentSpiritLevel, 0, 3))
    {
    case 1:
        return SpiritLevelMultiplierLv1;
    case 2:
        return SpiritLevelMultiplierLv2;
    case 3:
        return SpiritLevelMultiplierLv3;
    default:
        return SpiritLevelMultiplierLv0;
    }
}

float AMHPlayerCharacter::ResolveLongSwordDamageMultiplier(const FGameplayTag& InMoveTag) const
{
    FMHAttackMetaRow AttackMetaRow;
    const float MetaDamageMultiplier =
        FindAttackMetaRow(InMoveTag, AttackMetaRow)
        ? FMath::Max(0.0f, AttackMetaRow.DamageMultiplier)
        : 1.0f;

    return MetaDamageMultiplier * GetCurrentSpiritDamageMultiplier();
}

#pragma endregion

float AMHPlayerCharacter::GetCurrentHealthValue() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetHealth() : 0.0f;
}

float AMHPlayerCharacter::GetMaxHealthValue() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetMaxHealth() : 0.0f;
}

float AMHPlayerCharacter::GetHealthRatio() const
{
    const float MaxHealthValue = GetMaxHealthValue();
    if (MaxHealthValue <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(GetCurrentHealthValue() / MaxHealthValue, 0.0f, 1.0f);
}

float AMHPlayerCharacter::GetCurrentStaminaValue() const
{
    return PlayerAttributeSet ? FMath::Max(0.0f, PlayerAttributeSet->GetStamina()) : 0.0f;
}

float AMHPlayerCharacter::GetMaxStaminaValue() const
{
    return PlayerAttributeSet ? FMath::Max(0.0f, PlayerAttributeSet->GetMaxStamina()) : 0.0f;
}

float AMHPlayerCharacter::GetStaminaRatio() const
{
    const float MaxStaminaValue = GetMaxStaminaValue();
    if (MaxStaminaValue <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(GetCurrentStaminaValue() / MaxStaminaValue, 0.0f, 1.0f);
}

float AMHPlayerCharacter::GetCurrentSpiritGaugeValue() const
{
    return CurrentSpiritGauge;
}

float AMHPlayerCharacter::GetMaxSpiritGaugeValue() const
{
    return FMath::Max(0.0f, MaxSpiritGauge);
}

float AMHPlayerCharacter::GetSpiritGaugeRatio() const
{
    const float MaxSpiritGaugeValue = GetMaxSpiritGaugeValue();
    if (MaxSpiritGaugeValue <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(GetCurrentSpiritGaugeValue() / MaxSpiritGaugeValue, 0.0f, 1.0f);
}

int32 AMHPlayerCharacter::GetCurrentSpiritLevelValue() const
{
    return CurrentSpiritLevel;
}

int32 AMHPlayerCharacter::GetMaxSpiritLevelValue() const
{
    return 3;
}

void AMHPlayerCharacter::GetPlayerVitalStatus(float& OutCurrentHealth, float& OutMaxHealth, float& OutCurrentStamina, float& OutMaxStamina) const
{
    OutCurrentHealth = GetCurrentHealthValue();
    OutMaxHealth = GetMaxHealthValue();
    OutCurrentStamina = GetCurrentStaminaValue();
    OutMaxStamina = GetMaxStaminaValue();
}

void AMHPlayerCharacter::GetLongSwordSpiritStatus(float& OutCurrentSpiritGauge, float& OutMaxSpiritGauge, int32& OutCurrentSpiritLevel, int32& OutMaxSpiritLevel) const
{
    OutCurrentSpiritGauge = GetCurrentSpiritGaugeValue();
    OutMaxSpiritGauge = GetMaxSpiritGaugeValue();
    OutCurrentSpiritLevel = GetCurrentSpiritLevelValue();
    OutMaxSpiritLevel = GetMaxSpiritLevelValue();
}

void AMHPlayerCharacter::AddSpiritGauge(const float InAmount)
{
    if (InAmount <= 0.0f)
    {
        return;
    }

    CurrentSpiritGauge = FMath::Clamp(CurrentSpiritGauge + InAmount, 0.0f, FMath::Max(0.0f, MaxSpiritGauge));
}

void AMHPlayerCharacter::ConsumeSpiritGauge(const float InAmount)
{
    if (InAmount <= 0.0f)
    {
        return;
    }

    CurrentSpiritGauge = FMath::Clamp(CurrentSpiritGauge - InAmount, 0.0f, FMath::Max(0.0f, MaxSpiritGauge));
}

void AMHPlayerCharacter::IncreaseSpiritLevel(const int32 InAmount)
{
    if (InAmount <= 0)
    {
        return;
    }

    CurrentSpiritLevel = FMath::Clamp(CurrentSpiritLevel + InAmount, 0, 3);
}

void AMHPlayerCharacter::DecreaseSpiritLevel(const int32 InAmount)
{
    if (InAmount <= 0)
    {
        return;
    }

    CurrentSpiritLevel = FMath::Clamp(CurrentSpiritLevel - InAmount, 0, 3);
}

bool AMHPlayerCharacter::IsAttackAllowedForForesightCounter(const FGameplayTag& InAttackTag) const
{
    const FMHAttackDefinitionRow* AttackDefinitionRow = FindAttackDefinitionRow(InAttackTag);
    if (!AttackDefinitionRow)
    {
        return true;
    }

    return AttackDefinitionRow->bCanBeForesightCountered;
}

bool AMHPlayerCharacter::IsAttackAllowedForSpecialSheatheCounter(const FGameplayTag& InAttackTag) const
{
    const FMHAttackDefinitionRow* AttackDefinitionRow = FindAttackDefinitionRow(InAttackTag);
    if (!AttackDefinitionRow)
    {
        return true;
    }

    return AttackDefinitionRow->bCanBeSpecialSheatheCountered;
}

FMHHitAcknowledge AMHPlayerCharacter::BuildLongSwordInvulnerableHitAcknowledge() const
{
    return BuildLongSwordCounterAcknowledge();
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

void AMHPlayerCharacter::UpdateDirectionalTurnWindow(const float DeltaSeconds)
{
    if (!bDirectionalTurnWindowActive)
    {
        return;
    }

    TryApplyDirectionalTurnWindowRotation(DeltaSeconds);
}

bool AMHPlayerCharacter::TryApplyDirectionalTurnWindowRotation(const float DeltaSeconds)
{
    const FVector WorldMoveDirection = ResolveWorldMoveDirection(Controller, GetPreferredMoveInput2D());
    if (WorldMoveDirection.IsNearlyZero())
    {
        return false;
    }

    const float DesiredYaw = WorldMoveDirection.Rotation().Yaw;
    const float ClampedYawDelta = FMath::Clamp(
        FMath::FindDeltaAngleDegrees(DirectionalTurnWindowBaseYaw, DesiredYaw),
        -DirectionalTurnWindowMaxYawDeltaDegrees,
        DirectionalTurnWindowMaxYawDeltaDegrees);

    const float TargetYaw = DirectionalTurnWindowBaseYaw + ClampedYawDelta;
    const float CurrentYaw = GetActorRotation().Yaw;

    float NewYaw = TargetYaw;
    if (DirectionalTurnWindowRotationInterpSpeed > 0.0f && DeltaSeconds > 0.0f)
    {
        NewYaw = FMath::FixedTurn(CurrentYaw, TargetYaw, DirectionalTurnWindowRotationInterpSpeed * DeltaSeconds);
    }

    SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));
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



void AMHPlayerCharacter::ApplyEquippedWeaponStatEffect()
{
    // 스탯 적용 실패 원인 분석_이건주
    UE_LOG(LogTemp, Warning, TEXT("[WeaponGE Apply:State] HasAuthority=%d bGASInitialized=%d ASC=%d ActorInfoValid=%d WeaponClass=%s Weapon=%s"),
    HasAuthority() ? 1 : 0,
    bGASInitialized ? 1 : 0,
    AbilitySystemComponent ? 1 : 0,
    (AbilitySystemComponent && AbilitySystemComponent->AbilityActorInfo.IsValid()) ? 1 : 0,
    *GetNameSafe(WeaponStatEffectClass),
    *GetNameSafe(EquippedWeapon));

    
    if (!ensure(AbilitySystemComponent))
    {
        return;
    }
    
    if (!ensure(WeaponStatEffectClass))
    {
        return;
    }
    
    if (!ensure(EquippedWeapon))
    {
        return;
    }

    const FMHAttackStats& Stat = EquippedWeapon->GetAttackStats();

    // -----------------------
    // 1. GE 적용 (순수 수치)
    // -----------------------

    FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
    ContextHandle.AddInstigator(this, this); // 추가
    ContextHandle.AddSourceObject(EquippedWeapon);

    FGameplayEffectSpecHandle SpecHandle =
        AbilitySystemComponent->MakeOutgoingSpec(WeaponStatEffectClass, 1.0f, ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[WeaponGE Apply:Spec] SpecValid=%d DataValid=%d ItemAP=%.2f ItemAffinity=%.2f"),
    SpecHandle.IsValid() ? 1 : 0,
    (SpecHandle.IsValid() && SpecHandle.Data.IsValid()) ? 1 : 0,
    Stat.AttackPower,
    Stat.Affinity);


    // 어트리뷰트 셋에 무기 스탯 적용
    SpecHandle.Data->SetSetByCallerMagnitude(
        MHGameplayTags::Data_Weapon_AttackPower,
        Stat.AttackPower);

    SpecHandle.Data->SetSetByCallerMagnitude(
        MHGameplayTags::Data_Weapon_Affinity,
        Stat.Affinity);

    EquippedWeaponStatEffectHandle =
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

    // -----------------------
    // 2. Sharpness 초기화
    // -----------------------

    CurrentSharpnessColor = Stat.MaxSharpnessColor;

    CurrentSharpnessValue = GetMaxSharpnessValueFromColor(
        Stat.SharpnessLength,
        Stat.MaxSharpnessColor
    );

    // -----------------------
    // 3. Element 저장
    // -----------------------

    CurrentWeaponElementTag = Stat.AttackElementTag;
    
    // 장착GE_GE 적용 _이건주
    UE_LOG(LogTemp, Warning, TEXT("[WeaponGE Apply] HandleValid=%d Weapon=%s ItemAP=%.2f ItemAffinity=%.2f ASC_AP=%.2f ASC_CR=%.2f"),
    EquippedWeaponStatEffectHandle.IsValid() ? 1 : 0,
    *GetNameSafe(EquippedWeapon),
    Stat.AttackPower,
    Stat.Affinity,
    CombatAttributeSet ? CombatAttributeSet->GetAttackPower() : -1.f,
    CombatAttributeSet ? CombatAttributeSet->GetCriticalRate() : -1.f);
}

void AMHPlayerCharacter::RemoveEquippedWeaponStatEffect()
{
    if (AbilitySystemComponent && EquippedWeaponStatEffectHandle.IsValid())
    {
        AbilitySystemComponent->RemoveActiveGameplayEffect(EquippedWeaponStatEffectHandle);
        EquippedWeaponStatEffectHandle.Invalidate();
    }
    
    // 장착GE_GE 제거 _이건주
    UE_LOG(LogTemp, Warning, TEXT("[WeaponGE Remove] HandleValid=%d ASC_AP=%.2f ASC_CR=%.2f"),
    EquippedWeaponStatEffectHandle.IsValid() ? 1 : 0,
    CombatAttributeSet ? CombatAttributeSet->GetAttackPower() : -1.f,
    CombatAttributeSet ? CombatAttributeSet->GetCriticalRate() : -1.f);
}

void AMHPlayerCharacter::RefreshEquippedWeaponStatEffect()
{
    RemoveEquippedWeaponStatEffect();
    ApplyEquippedWeaponStatEffect();
}

float AMHPlayerCharacter::GetMaxSharpnessValueFromColor(
    const FMHSharpnessData& Data,
    EMHSharpnessColor Color) const
{
    switch (Color)
    {
    case EMHSharpnessColor::Red:    return Data.Red;
    case EMHSharpnessColor::Orange: return Data.Orange;
    case EMHSharpnessColor::Yellow: return Data.Yellow;
    case EMHSharpnessColor::Green:  return Data.Green;
    case EMHSharpnessColor::Blue:   return Data.Blue;
    case EMHSharpnessColor::White:  return Data.White;
    default: return 0.f;
    }
}

void AMHPlayerCharacter::ConsumeSharpness(float Amount)
{
    if (!EquippedWeapon)
        return;

    const FMHAttackStats& Stat = EquippedWeapon->GetAttackStats();

    CurrentSharpnessValue -= Amount;

    while (CurrentSharpnessValue <= 0.f)
    {
        if (!DowngradeSharpnessColor())
        {
            CurrentSharpnessValue = 0.f;
            break;
        }

        CurrentSharpnessValue = GetMaxSharpnessValueFromColor(
            Stat.SharpnessLength,
            CurrentSharpnessColor
        );
    }
}

bool AMHPlayerCharacter::DowngradeSharpnessColor()
{
    switch (CurrentSharpnessColor)
    {
    case EMHSharpnessColor::White:  CurrentSharpnessColor = EMHSharpnessColor::Blue; return true;
    case EMHSharpnessColor::Blue:   CurrentSharpnessColor = EMHSharpnessColor::Green; return true;
    case EMHSharpnessColor::Green:  CurrentSharpnessColor = EMHSharpnessColor::Yellow; return true;
    case EMHSharpnessColor::Yellow: CurrentSharpnessColor = EMHSharpnessColor::Orange; return true;
    case EMHSharpnessColor::Orange: CurrentSharpnessColor = EMHSharpnessColor::Red; return true;
    case EMHSharpnessColor::Red:    return false;
    }

    return false;
}

void AMHPlayerCharacter::HandleWeaponAttackHit(
    AActor* Target,
    AMHWeaponInstance* Weapon)
{
    // 1. Sharpness
    ConsumeSharpness(5.f);
    
    // 2. SpiritGuage
    AddSpiritGauge(5.f);

    // // 3. 무기별 처리
    // if (Weapon)
    // {
    //     Weapon->OnAttackHit(Target);
    // }
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

#pragma endregion

#pragma region WeaponAnimationLayerFunctions
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
#pragma endregion

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

void AMHPlayerCharacter::ApplyDefaultPlayerAttributes()
{
    // 현재 프로젝트 기준 플레이어 기본 체력, 방어력, 스태미나는 하드코딩 값으로 고정한다.
    constexpr float DefaultMaxHealth = 1000.0f;
    constexpr float DefaultCurrentHealth = 1000.0f;
    constexpr float DefaultDefense = 10.0f;
    constexpr float DefaultMaxStamina = 100.0f;
    constexpr float DefaultCurrentStamina = 100.0f;

    SetMaxHealthAttributeValue(DefaultMaxHealth);
    SetCurrentHealthAttributeValue(DefaultCurrentHealth);
    SetDefenseAttributeValue(DefaultDefense);
    SetMaxStaminaAttributeValue(DefaultMaxStamina);
    SetCurrentStaminaAttributeValue(DefaultCurrentStamina);
}

void AMHPlayerCharacter::SetCurrentHealthAttributeValue(float InNewValue)
{
    const float MaxHealthValue = GetMaxHealthValue();
    const float ClampedHealthValue = MaxHealthValue > 0.0f
        ? FMath::Clamp(InNewValue, 0.0f, MaxHealthValue)
        : FMath::Max(0.0f, InNewValue);

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UMHHealthAttributeSet::GetHealthAttribute(), ClampedHealthValue);
    }
    else if (HealthAttributeSet)
    {
        HealthAttributeSet->SetHealth(ClampedHealthValue);
    }
}

void AMHPlayerCharacter::SetMaxHealthAttributeValue(float InNewValue)
{
    const float ClampedMaxHealthValue = FMath::Max(0.0f, InNewValue);

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UMHHealthAttributeSet::GetMaxHealthAttribute(), ClampedMaxHealthValue);
    }
    else if (HealthAttributeSet)
    {
        HealthAttributeSet->SetMaxHealth(ClampedMaxHealthValue);
    }
}

void AMHPlayerCharacter::SetDefenseAttributeValue(float InNewValue)
{
    const float ClampedDefenseValue = FMath::Max(0.0f, InNewValue);

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UMHCombatAttributeSet::GetDefenseAttribute(), ClampedDefenseValue);
    }
    else if (CombatAttributeSet)
    {
        CombatAttributeSet->SetDefense(ClampedDefenseValue);
    }
}

void AMHPlayerCharacter::SyncStaminaAttributesFromConfig()
{
    const float ConfigMaxStamina = FMath::Max(0.0f, StaminaConfig.MaxStamina);
    SetMaxStaminaAttributeValue(ConfigMaxStamina);
    SetCurrentStaminaAttributeValue(ConfigMaxStamina);
}

void AMHPlayerCharacter::SetCurrentStaminaAttributeValue(float InNewValue)
{
    const float MaxStaminaValue = GetMaxStaminaValue();
    const float ClampedStaminaValue = FMath::Clamp(InNewValue, 0.0f, MaxStaminaValue);

    if (PlayerAttributeSet)
    {
        if (AbilitySystemComponent)
        {
            AbilitySystemComponent->SetNumericAttributeBase(UMHPlayerAttributeSet::GetStaminaAttribute(), ClampedStaminaValue);
        }
        else
        {
            PlayerAttributeSet->SetStamina(ClampedStaminaValue);
        }
    }
}

void AMHPlayerCharacter::SetMaxStaminaAttributeValue(float InNewValue)
{
    const float ClampedMaxStaminaValue = FMath::Max(0.0f, InNewValue);

    if (PlayerAttributeSet)
    {
        if (AbilitySystemComponent)
        {
            AbilitySystemComponent->SetNumericAttributeBase(UMHPlayerAttributeSet::GetMaxStaminaAttribute(), ClampedMaxStaminaValue);
        }
        else
        {
            PlayerAttributeSet->SetMaxStamina(ClampedMaxStaminaValue);
        }
    }
}

void AMHPlayerCharacter::UpdateStamina(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    const float CurrentStaminaValue = GetCurrentStaminaValue();
    const float MaxStaminaValue = GetMaxStaminaValue();
    float NewStaminaValue = CurrentStaminaValue;

    if (bIsSprinting)
    {
        NewStaminaValue -= StaminaConfig.SprintCostPerSecond * DeltaSeconds;
    }
    else
    {
        NewStaminaValue += StaminaConfig.RecoveryPerSecond * DeltaSeconds;
    }

    NewStaminaValue = FMath::Clamp(NewStaminaValue, 0.0f, MaxStaminaValue);

    if (!FMath::IsNearlyEqual(CurrentStaminaValue, NewStaminaValue))
    {
        SetCurrentStaminaAttributeValue(NewStaminaValue);
    }

    if (bIsSprinting && GetCurrentStaminaValue() <= 0.0f)
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
    return GetCurrentStaminaValue() > StaminaConfig.LowStaminaThreshold;
}



bool AMHPlayerCharacter::ApplyIncomingDamageSpec(
    const FGameplayEffectSpecHandle& DamageSpecHandle
)
{
    if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
    {
        return false;
    }

    return ApplyIncomingPlayerDamageSpec(*DamageSpecHandle.Data.Get());
}

bool AMHPlayerCharacter::ApplyIncomingPlayerDamageSpec(
    const FGameplayEffectSpec& IncomingSpec
)
{
    UAbilitySystemComponent* TargetASC = GetCharacterASC();
    if (!IsValid(TargetASC))
    {
        return false;
    }

    TSubclassOf<UGameplayEffect> DamageGEClass = PlayerIncomingDamageEffectClass;
    if (!DamageGEClass)
    {
        DamageGEClass = UMHGameplayEffect_PlayerDamage::StaticClass();
    }

    if (!DamageGEClass)
    {
        return false;
    }

    FGameplayEffectContextHandle EffectContext = IncomingSpec.GetContext();
    if (!EffectContext.IsValid())
    {
        EffectContext = TargetASC->MakeEffectContext();
        EffectContext.AddInstigator(this, this);
    }

    const FGameplayEffectSpecHandle PlayerDamageSpecHandle =
        TargetASC->MakeOutgoingSpec(DamageGEClass, IncomingSpec.GetLevel(), EffectContext);

    if (!PlayerDamageSpecHandle.IsValid() || !PlayerDamageSpecHandle.Data.IsValid())
    {
        return false;
    }

    static const FGameplayTag DamageTags[] =
    {
        MHGameplayTags::Data_Damage_Physical,
        MHGameplayTags::Data_Damage_Fire,
        MHGameplayTags::Data_Damage_Water,
        MHGameplayTags::Data_Damage_Thunder,
        MHGameplayTags::Data_Damage_Ice,
        MHGameplayTags::Data_Damage_Dragon
    };

    for (const FGameplayTag& DamageTag : DamageTags)
    {
        if (!DamageTag.IsValid())
        {
            continue;
        }

        const float Magnitude = IncomingSpec.GetSetByCallerMagnitude(DamageTag, false, 0.0f);
        if (!FMath::IsNearlyZero(Magnitude))
        {
            PlayerDamageSpecHandle.Data->SetSetByCallerMagnitude(DamageTag, Magnitude);
        }
    }

    const FActiveGameplayEffectHandle ActiveHandle =
        TargetASC->ApplyGameplayEffectSpecToSelf(*PlayerDamageSpecHandle.Data.Get());

    return ActiveHandle.WasSuccessfullyApplied();
}
