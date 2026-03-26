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
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameFramework/PlayerController.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "Items/Instance/MHGreatSwordInstance.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"
#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Status/MHGA_MHSharpen.h"
#include "AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h"
#include "AbilitySystem/Abilities/Status/MHGA_Potion.h"
#include "AbilitySystem/Abilities/Weapon/GreatSword/MHGA_GreatSwordAttack.h"
#include "GameplayTags/MHGreatSwordGameplayTags.h"
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
#include "TimerManager.h"
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
#include "Interfaces/MHDamageSpecReceiverInterface.h"

DEFINE_LOG_CATEGORY(LogMHPlayerCharacter);

namespace
{
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

    static FMHHitAcknowledge BuildPlayerInvulnerableAcknowledge()
    {
        FMHHitAcknowledge Result;
        Result.bAcceptedHit = true;
        Result.bConsumeHitOnce = true;
        Result.bShouldStopAttackWindow = false;
        Result.ResultType = EMHHitResultType::Invulnerable;
        return Result;
    }

    static const TCHAR* ResolveSharpnessColorText(const EMHSharpnessColor InColor)
    {
        switch (InColor)
        {
        case EMHSharpnessColor::Red:
            return TEXT("Red");
        case EMHSharpnessColor::Orange:
            return TEXT("Orange");
        case EMHSharpnessColor::Yellow:
            return TEXT("Yellow");
        case EMHSharpnessColor::Green:
            return TEXT("Green");
        case EMHSharpnessColor::Blue:
            return TEXT("Blue");
        case EMHSharpnessColor::White:
            return TEXT("White");
        default:
            return TEXT("Unknown");
        }
    }

    // 납도 상태와 전이 윈도우를 반영해 대검 기본 입력 패턴을 결정한다.
    static FGameplayTag ResolveGreatSwordPrimaryPatternTag(const UMHGreatSwordActionComponent* InActionComponent, const bool bInForwardInput, const bool bInSheathed)
    {
        using namespace MHInputPatternGameplayTags;

        if (bInSheathed)
        {
            return bInForwardInput ? InputPattern_GS_DrawCharge : InputPattern_GS_DrawOnly;
        }

        const bool bUseForwardPrimary = bInForwardInput
            && InActionComponent
            && InActionComponent->GetActionState() == EMHGreatSwordActionState::Acting
            && InActionComponent->IsAnyTransitionWindowOpen();

        return bUseForwardPrimary ? InputPattern_GS_ForwardPrimary : InputPattern_GS_Primary;
    }

    // 무기 특수 입력은 납도 시 발도 가드, 발도 시 일반 가드로 해석한다.
    static FGameplayTag ResolveGreatSwordWeaponSpecialPatternTag(const bool bInSheathed)
    {
        using namespace MHInputPatternGameplayTags;
        return bInSheathed ? InputPattern_GS_DrawGuard : InputPattern_GS_WeaponSpecial;
    }
}

AMHPlayerCharacter::AMHPlayerCharacter()
{
    InitializeCapsuleSettings();
    
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 280.0f;
    CameraBoom->SocketOffset = FVector(0.f, 0.f, 65.f);
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
        MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
    }


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
    RefreshSharpnessState();
    BroadcastInitialAttributeSnapshot();

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
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_WeaponSpecial, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_WeaponSpecial);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_WeaponSpecial, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_WeaponSpecialCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackSimultaneous))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSimultaneous, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackSimultaneous);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AimHold))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AimHold, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AimHoldStarted);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AimHold, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_AimHoldCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_ItemSelectSharpen))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_ItemSelectSharpen, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_ItemSelectSharpen);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_ItemSelectPotion))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_ItemSelectPotion, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_ItemSelectPotion);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_ItemUse))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_ItemUse, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_ItemUseStarted);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_ItemUse, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_ItemUseCompleted);
    }

    PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AMHPlayerCharacter::Input_DebugIncomingDamageKeyPressed);
}

void AMHPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        OnCharacterMovementUpdated.RemoveDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }

    ClearBurningState();

    UnequipCurrentWeapon(false);
    
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
    if (bActionInputLocked)
    {
        CachedMoveInput2D = FVector2D::ZeroVector;
        return;
    }

    const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
    CachedMoveInput2D = MovementVector;

    if (UWorld* World = GetWorld())
    {
        UpdateDirectionalTurnWindow(World->GetDeltaSeconds());
    }

    if (!MovementVector.IsNearlyZero())
    {
        LastNonZeroMoveInput2D = MovementVector;
        CancelSharpenAbilityIfActive();
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
    if (bActionInputLocked)
    {
        return;
    }

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
    if (bActionInputLocked)
    {
        return;
    }

    bSprintHeld = false;
    EvaluateSprintState();
}

void AMHPlayerCharacter::Input_Dodge(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bDodgeHeld = true;
    CancelSharpenAbilityIfActive();

    if (IsGreatSwordEquipped())
    {
        AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
        UMHGreatSwordActionComponent* ActionComponent = GreatSword ? GreatSword->GetActionComponent() : nullptr;
        if (!ActionComponent || WeaponSheathState == EMHWeaponSheathState::Sheathing)
        {
            return;
        }

        const bool bSheathed = WeaponSheathState == EMHWeaponSheathState::Sheathed;
        const EMHGreatSwordActionState GreatSwordActionState = ActionComponent->GetActionState();
        EMHDirectionalVariant DodgeVariant = EMHDirectionalVariant::Forward;
        bool bShouldRotateTowardsMoveInput = false;

        if (bSheathed)
        {
            LastResolvedDodgeContext = EMHDodgeContext::Sheathed;
            LastResolvedDodgeVariant = EMHDirectionalVariant::Forward;
            bShouldRotateTowardsMoveInput = true;
        }
        else if (ActionComponent->IsAttackRollWindowOpen())
        {
            LastResolvedDodgeContext = EMHDodgeContext::AttackChain;
            DodgeVariant = ResolveDirectionalVariantFromInput(true);
            LastResolvedDodgeVariant = DodgeVariant;
        }
        else if (GreatSwordActionState == EMHGreatSwordActionState::Neutral || GreatSwordActionState == EMHGreatSwordActionState::Guarding)
        {
            LastResolvedDodgeContext = EMHDodgeContext::UnsheathedNeutral;
            LastResolvedDodgeVariant = EMHDirectionalVariant::Forward;
            bShouldRotateTowardsMoveInput = true;
        }
        else
        {
            // 공격 중 롤 윈도우가 열리기 전 입력은 선회만 일으키지 않도록 막는다.
            LastResolvedDodgeContext = EMHDodgeContext::AttackChain;
            DodgeVariant = ResolveDirectionalVariantFromInput(true);
            LastResolvedDodgeVariant = DodgeVariant;
        }

        // 이번 프레임에 회피 실행이 실패하면 이 스냅샷으로 상태를 되돌린다.
        FMHGreatSwordRuntimeSnapshot ActionSnapshot;
        ActionComponent->CaptureRuntimeSnapshot(ActionSnapshot);

        const EMHWeaponSheathState PreviousSheathState = WeaponSheathState;
        const bool bPreviousPendingUnsheathe = bPendingUnsheatheFromComboEntry;
        const FRotator PreviousActorRotation = GetActorRotation();

        if (!ActionComponent->HandleDodgePressed(bSheathed, DodgeVariant))
        {
            return;
        }

        const bool bAppliedPreDodgeRotation = bShouldRotateTowardsMoveInput && TryRotateActorTowardsMoveInput();

        if (bSheathed)
        {
            WeaponSheathState = EMHWeaponSheathState::Unsheathing;
            bPendingUnsheatheFromComboEntry = true;
        }

        if (!TryExecuteGreatSwordPendingMove())
        {
            WeaponSheathState = PreviousSheathState;
            bPendingUnsheatheFromComboEntry = bPreviousPendingUnsheathe;
            ActionComponent->RestoreRuntimeSnapshot(ActionSnapshot);
            if (bAppliedPreDodgeRotation)
            {
                SetActorRotation(PreviousActorRotation);
            }

            UE_LOG(
                LogMHPlayerCharacter,
                Verbose,
                TEXT("%s : GreatSword dodge execution failed. Context=%d Variant=%d"),
                *GetName(),
                static_cast<int32>(LastResolvedDodgeContext),
                static_cast<int32>(LastResolvedDodgeVariant)
            );
        }
        else
        {
            HandleBurnRollSucceeded();
        }
        return;
    }

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
    if (bActionInputLocked)
    {
        return;
    }

    bDodgeHeld = false;
}

void AMHPlayerCharacter::Input_DebugIncomingDamageKeyPressed()
{
    const FGameplayTag AttackTagToUse = DebugIncomingAttackTag.IsValid()
        ? DebugIncomingAttackTag
        : FGameplayTag::RequestGameplayTag(FName(TEXT("Attack.Debug.Counterable")), false);

    if (!AttackTagToUse.IsValid())
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : 디버그 공격 태그를 찾지 못했습니다. Attack.Debug.Counterable 태그를 확인하세요."), *GetName());
        return;
    }

    ApplyDebugDamageFromSource(this, DebugIncomingPhysicalDamage, AttackTagToUse);

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("%s : 디버그 피격을 적용했습니다. Physical=%.2f AttackTag=%s"),
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
    if (bActionInputLocked)
    {
        return;
    }

    bAttackPrimaryHeld = true;
    CancelSharpenAbilityIfActive();

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordPrimaryInput();
        return;
    }

    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForPrimaryInput());
}

void AMHPlayerCharacter::Input_AttackPrimaryCompleted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bAttackPrimaryHeld = false;

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordPrimaryRelease();
    }
}

void AMHPlayerCharacter::Input_AttackSecondary(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bAttackSecondaryHeld = true;
    CancelSharpenAbilityIfActive();

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordSecondaryInput();
        return;
    }

    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForSecondaryInput());
}

void AMHPlayerCharacter::Input_AttackSecondaryCompleted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bAttackSecondaryHeld = false;
}

void AMHPlayerCharacter::Input_WeaponSpecial(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bWeaponSpecialHeld = true;
    CancelSharpenAbilityIfActive();

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordWeaponSpecialInput();
        return;
    }

    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForWeaponSpecialInput());
}

void AMHPlayerCharacter::Input_WeaponSpecialCompleted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bWeaponSpecialHeld = false;

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordWeaponSpecialRelease();
    }
}

void AMHPlayerCharacter::Input_AttackSimultaneous(const FInputActionValue& InputActionValue)
{

    if (bActionInputLocked)
    {
        return;
    }

    CancelSharpenAbilityIfActive();


    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordSimultaneousInput();
        return;
    }

    TryResolveAndHandleLongSwordPattern(ResolveLongSwordPatternForAttackSimultaneousInput());
}

void AMHPlayerCharacter::Input_AimHoldStarted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bAimHeld = true;
}

void AMHPlayerCharacter::Input_AimHoldCompleted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    bAimHeld = false;
}


void AMHPlayerCharacter::UsePrimaryAction()
{
    if (bActionInputLocked)
    {
        return;
    }

    if (IsGreatSwordEquipped())
    {
        TryHandleGreatSwordPrimaryInput();
        return;
    }

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

void AMHPlayerCharacter::Notify_AttachWeaponToSocket(const FName InSocketName)
{
    AttachWeaponToSocket(InSocketName);
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

void AMHPlayerCharacter::Notify_GreatSwordChargeLevelReached(const int32 InChargeLevel)
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyChargeLevelReached(InChargeLevel);
        }
    }
}

void AMHPlayerCharacter::Notify_GreatSwordChargeAutoRelease()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            if (ActionComponent->NotifyChargeAutoReleaseRequested())
            {
                TryExecuteGreatSwordPendingMove();
            }
        }
    }
}

void AMHPlayerCharacter::Notify_BeginGreatSwordAttackRollWindow()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyBeginAttackRollWindow();
        }
    }
}

void AMHPlayerCharacter::Notify_EndGreatSwordAttackRollWindow()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyEndAttackRollWindow();
        }
    }
}

void AMHPlayerCharacter::Notify_BeginGreatSwordEarlyTransitionWindow()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyBeginEarlyTransitionWindow();
            if (ActionComponent->HasPendingMove())
            {
                TryExecuteGreatSwordPendingMove();
            }
        }
    }
}

void AMHPlayerCharacter::Notify_EndGreatSwordEarlyTransitionWindow()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyEndEarlyTransitionWindow();
        }
    }
}

void AMHPlayerCharacter::Notify_BeginGreatSwordChargeFollowUpWindow(const FGameplayTag InSourceMoveTag)
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyBeginChargeFollowUpWindow(InSourceMoveTag);
            if (ActionComponent->HasPendingMove())
            {
                TryExecuteGreatSwordPendingMove();
            }
        }
    }
}

void AMHPlayerCharacter::Notify_EndGreatSwordChargeFollowUpWindow()
{
    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyEndChargeFollowUpWindow();
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
    bLongSwordSpiritThrustHelmbreakerReady = false;
    bLongSwordForesightFreeSpiritRoundslashReady = false;
}

void AMHPlayerCharacter::Notify_LongSwordMoveStarted(const FGameplayTag& InMoveTag)
{
    if (!IsLongSwordEquipped() || !InMoveTag.IsValid())
    {
        return;
    }

    ApplyLongSwordMoveStartCost(InMoveTag);
}

void AMHPlayerCharacter::Notify_LongSwordAttackHitConfirmed(const FGameplayTag& InMoveTag)
{
    if (!IsLongSwordEquipped() || !InMoveTag.IsValid())
    {
        return;
    }

    ApplyLongSwordMoveHitReward(InMoveTag);

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
    if (!IsLongSwordEquipped())
    {
        return;
    }

    const FGameplayTag CurrentMoveTag = GetCurrentLongSwordMoveTag();
    if (!CurrentMoveTag.IsValid())
    {
        return;
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword counter success resolved. Window=%d Move=%s"),
        *GetName(),
        static_cast<int32>(InCounterWindowType),
        *CurrentMoveTag.ToString()
    );

    ApplyLongSwordCounterSuccessReward(CurrentMoveTag, InCounterWindowType);
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

bool AMHPlayerCharacter::CanReceiveDamage(
    AActor* SourceActor,
    FGameplayTag AttackTag,
    const FGameplayEffectSpecHandle& DamageSpecHandle,
    const FHitResult& HitResult
) const
{
    if (IsDamageHitReactActive())
    {
        return false;
    }

    return Super::CanReceiveDamage(SourceActor, AttackTag, DamageSpecHandle, HitResult);
}

bool AMHPlayerCharacter::IsDamageHitReactActive() const
{
    return bDamageHitReactMontagePlaying;
}

void AMHPlayerCharacter::BeginPotionUse(UMHGA_Potion* InPotionAbility)
{
    if (!InPotionAbility)
    {
        return;
    }

    bPotionInUse = true;
    ActivePotionAbility = InPotionAbility;
}

void AMHPlayerCharacter::EndPotionUse(UMHGA_Potion* InPotionAbility)
{
    if (ActivePotionAbility.IsValid() && ActivePotionAbility.Get() != InPotionAbility)
    {
        return;
    }

    bPotionInUse = false;
    ActivePotionAbility = nullptr;
}

void AMHPlayerCharacter::CancelActivePotionUseOnDamageTaken()
{
    if (!bPotionInUse)
    {
        return;
    }

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[HitReact] 피격으로 포션 사용을 취소합니다."));

    if (ActivePotionAbility.IsValid())
    {
        ActivePotionAbility->EndPotionByDamageTaken();
    }

    bPotionInUse = false;
    ActivePotionAbility = nullptr;
}

void AMHPlayerCharacter::TryIgniteBurning()
{
    if (IsDead())
    {
        return;
    }

    if (!bBurningActive)
    {
        bBurningActive = true;
        BurnRollCount = 0;

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(BurnDamageTimerHandle);
            World->GetTimerManager().SetTimer(
                BurnDamageTimerHandle,
                this,
                &AMHPlayerCharacter::HandleBurnDamageTick,
                FMath::Max(0.01f, BurnTickInterval),
                true
            );
        }

        if (!BurningLoopNiagaraComponent && !BurningLoopNiagara.IsNull() && GetMesh())
        {
            UNiagaraSystem* BurningSystem = BurningLoopNiagara.LoadSynchronous();
            if (BurningSystem)
            {
                BurningLoopNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
                    BurningSystem,
                    GetMesh(),
                    NAME_None,
                    FVector::ZeroVector,
                    FRotator::ZeroRotator,
                    EAttachLocation::KeepRelativeOffset,
                    true
                );
            }
        }

        UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Burn] 화상 상태를 시작합니다."));
        return;
    }

    if (!BurningLoopNiagaraComponent && !BurningLoopNiagara.IsNull() && GetMesh())
    {
        UNiagaraSystem* BurningSystem = BurningLoopNiagara.LoadSynchronous();
        if (BurningSystem)
        {
            BurningLoopNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
                BurningSystem,
                GetMesh(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
    }
}

void AMHPlayerCharacter::HandleBurnDamageTick()
{
    if (!bBurningActive)
    {
        return;
    }

    if (IsDead() || GetCurrentHealthValue() <= 0.0f)
    {
        ClearBurningState();
        return;
    }

    UAbilitySystemComponent* TargetASC = GetCharacterASC();
    if (!IsValid(TargetASC))
    {
        ClearBurningState();
        return;
    }

    TSubclassOf<UGameplayEffect> DamageGEClass = PlayerIncomingDamageEffectClass;
    if (!DamageGEClass)
    {
        DamageGEClass = UMHGameplayEffect_PlayerDamage::StaticClass();
    }

    if (!DamageGEClass)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    EffectContext.AddInstigator(this, this);

    const FGameplayEffectSpecHandle BurnDamageSpecHandle = TargetASC->MakeOutgoingSpec(DamageGEClass, 1.0f, EffectContext);
    if (!BurnDamageSpecHandle.IsValid() || !BurnDamageSpecHandle.Data.IsValid())
    {
        return;
    }

    BurnDamageSpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Physical, FMath::Max(0.0f, BurnDamagePerTick));

    static const FGameplayTag ElementDamageTags[] =
    {
        MHGameplayTags::Data_Damage_Fire,
        MHGameplayTags::Data_Damage_Water,
        MHGameplayTags::Data_Damage_Thunder,
        MHGameplayTags::Data_Damage_Ice,
        MHGameplayTags::Data_Damage_Dragon
    };

    for (const FGameplayTag& DamageTag : ElementDamageTags)
    {
        if (DamageTag.IsValid())
        {
            BurnDamageSpecHandle.Data->SetSetByCallerMagnitude(DamageTag, 0.0f);
        }
    }

    const FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*BurnDamageSpecHandle.Data.Get());
    if (!ActiveHandle.WasSuccessfullyApplied())
    {
        return;
    }

    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("[Burn] 화상 도트 데미지를 적용했습니다. HP=%.1f/%.1f"), GetCurrentHealthValue(), GetMaxHealthValue());

    if (GetCurrentHealthValue() <= 0.0f)
    {
        HandleDeath();
    }
}

void AMHPlayerCharacter::HandleBurnRollSucceeded()
{
    if (!bBurningActive)
    {
        return;
    }

    ++BurnRollCount;

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Burn] 구르기로 화상 해제 카운트를 올렸습니다. Count=%d / %d"),
        BurnRollCount,
        BurnRequiredRollCount
    );

    if (BurnRollCount >= FMath::Max(1, BurnRequiredRollCount))
    {
        ClearBurningState();
    }
}

void AMHPlayerCharacter::ClearBurningState()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BurnDamageTimerHandle);
    }

    if (BurningLoopNiagaraComponent)
    {
        BurningLoopNiagaraComponent->Deactivate();
        BurningLoopNiagaraComponent->DestroyComponent();
        BurningLoopNiagaraComponent = nullptr;
    }

    if (bBurningActive)
    {
        UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Burn] 화상 상태를 해제했습니다."));
    }

    bBurningActive = false;
    BurnRollCount = 0;
}

void AMHPlayerCharacter::RefreshActionInputLockState()
{
    const bool bShouldLock = bActionInputLockedByDamageHitReact || bActionInputLockedBySharpnessBounce;
    if (bActionInputLocked == bShouldLock)
    {
        return;
    }

    bActionInputLocked = bShouldLock;

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        PlayerController->SetIgnoreMoveInput(bActionInputLocked);
    }

    if (bActionInputLocked)
    {
        bSprintHeld = false;
        bIsSprinting = false;
        bAimHeld = false;
        bAttackPrimaryHeld = false;
        bAttackSecondaryHeld = false;
        bWeaponSpecialHeld = false;
        bDodgeHeld = false;
        bItemUseHeld = false;
        CachedMoveInput2D = FVector2D::ZeroVector;

        ApplyMovementProfile(EMHPlayerMoveProfile::Run);

        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }
    }
}

void AMHPlayerCharacter::SetDamageHitReactInputLock(bool bEnable)
{
    bActionInputLockedByDamageHitReact = bEnable;
    RefreshActionInputLockState();
}

void AMHPlayerCharacter::SetSharpnessBounceInputLock(bool bEnable)
{
    bActionInputLockedBySharpnessBounce = bEnable;
    RefreshActionInputLockState();
}

bool AMHPlayerCharacter::ResolveDamageHitReactFacingYaw(AActor* SourceActor, const FHitResult& HitResult, FRotator& OutFacingRotation) const
{
    FVector DirectionToSource = FVector::ZeroVector;

    if (IsValid(SourceActor))
    {
        DirectionToSource = GetActorLocation() - SourceActor->GetActorLocation();
    }
    else if (!HitResult.ImpactPoint.IsNearlyZero())
    {
        DirectionToSource = GetActorLocation() - HitResult.ImpactPoint;
    }

    DirectionToSource.Z = 0.0f;
    if (DirectionToSource.IsNearlyZero())
    {
        return false;
    }

    OutFacingRotation = DirectionToSource.Rotation();
    OutFacingRotation.Pitch = 0.0f;
    OutFacingRotation.Roll = 0.0f;
    return true;
}

bool AMHPlayerCharacter::TryPlayDamageHitReactMontage(AActor* SourceActor, const FHitResult& HitResult)
{
    if (bDamageHitReactMontagePlaying || !DamageHitReactMontage)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return false;
    }

    FRotator FacingRotation = GetActorRotation();
    if (ResolveDamageHitReactFacingYaw(SourceActor, HitResult, FacingRotation))
    {
        SetActorRotation(FacingRotation);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }

    const float PlayedLength = AnimInstance->Montage_Play(DamageHitReactMontage);
    if (PlayedLength <= 0.0f)
    {
        return false;
    }

    ActiveDamageHitReactMontage = DamageHitReactMontage;
    bDamageHitReactMontagePlaying = true;
    SetDamageHitReactInputLock(true);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleDamageHitReactMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, DamageHitReactMontage);

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[HitReact] 피격 몽타주를 재생합니다."));
    return true;
}

void AMHPlayerCharacter::HandleDamageHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != ActiveDamageHitReactMontage)
    {
        return;
    }

    bDamageHitReactMontagePlaying = false;
    ActiveDamageHitReactMontage = nullptr;
    SetDamageHitReactInputLock(false);
}

void AMHPlayerCharacter::HandleSharpnessBounceMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != ActiveSharpnessBounceMontage)
    {
        return;
    }

    ActiveSharpnessBounceMontage = nullptr;
    SetSharpnessBounceInputLock(false);
}

bool AMHPlayerCharacter::TryPlayDeathMontage()
{
    if (ActiveDeathMontage || !DeathMontage)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance)
    {
        return false;
    }

    const float PlayedLength = AnimInstance->Montage_Play(DeathMontage);
    if (PlayedLength <= 0.0f)
    {
        return false;
    }

    ActiveDeathMontage = DeathMontage;

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleDeathMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Death] 사망 몽타주를 재생합니다."));
    return true;
}

void AMHPlayerCharacter::HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != ActiveDeathMontage)
    {
        return;
    }

    ActiveDeathMontage = nullptr;
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Death] Death montage finished"));
}

bool AMHPlayerCharacter::IsDead() const
{
    return bDeathStateActive || GetCurrentHealthValue() <= 0.0f;
}

void AMHPlayerCharacter::HandleDeath()
{
    if (bDeathStateActive)
    {
        return;
    }

    Super::HandleDeath();

    bDeathStateActive = true;
    CancelActivePotionUseOnDamageTaken();
    ClearBurningState();

    if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        if (ActiveDamageHitReactMontage)
        {
            UAnimMontage* PreviousHitReactMontage = ActiveDamageHitReactMontage;
            ActiveDamageHitReactMontage = nullptr;
            bDamageHitReactMontagePlaying = false;
            AnimInstance->Montage_Stop(0.05f, PreviousHitReactMontage);
        }
    }

    SetDamageHitReactInputLock(true);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }

    TryPlayDeathMontage();
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Death] 사망 처리를 시작합니다."));
}

void AMHPlayerCharacter::HandleDamageAccepted(
    AActor* SourceActor,
    AActor* SourceWeapon,
    FGameplayTag AttackTag,
    const FHitResult& HitResult
)
{
    Super::HandleDamageAccepted(SourceActor, SourceWeapon, AttackTag, HitResult);

    CancelActivePotionUseOnDamageTaken();

    if (GetCurrentHealthValue() <= 0.0f || IsDamageHitReactActive())
    {
        return;
    }

    TryPlayDamageHitReactMontage(SourceActor, HitResult);
}

FMHHitAcknowledge AMHPlayerCharacter::ReceiveDamageSpec_Implementation(
    AActor* SourceActor,
    AActor* SourceWeapon,
    FGameplayTag AttackTag,
    const FGameplayEffectSpecHandle& DamageSpecHandle,
    const FHitResult& HitResult)
{
    const float IncomingFireDamage = DamageSpecHandle.IsValid() && DamageSpecHandle.Data.IsValid()
        ? DamageSpecHandle.Data->GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Fire, false, 0.0f)
        : 0.0f;

    if (!ValidateDamageSpec(DamageSpecHandle))
    {
        HandleDamageRejected(SourceActor, SourceWeapon, AttackTag, HitResult);
        return BuildRejectedHitAcknowledge();
    }

    CancelSharpenAbilityIfActive();

    ClearExpiredLongSwordDamageIgnoreState();

    if (IsDamageHitReactActive())
    {
        return BuildPlayerInvulnerableAcknowledge();
    }

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

    const FMHHitAcknowledge HitAcknowledge = Super::ReceiveDamageSpec_Implementation(SourceActor, SourceWeapon, AttackTag, DamageSpecHandle, HitResult);

    if (HitAcknowledge.bAcceptedHit && IncomingFireDamage > 0.0f && !IsDead())
    {
        TryIgniteBurning();
    }

    return HitAcknowledge;
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
    AMHWeaponInstance* SpawnedWeapon = World->SpawnActor<AMHWeaponInstance>(DefaultWeaponClass, SpawnParams);
    if (!SpawnedWeapon)
    {
        return;
    }

    EquipWeaponInstance(SpawnedWeapon, true);

    UE_LOG(LogMHPlayerCharacter, Warning, TEXT("[Equip] Weapon=%s AP=%.2f CR=%.2f"),
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
    // RefreshEquippedWeaponStatEffect();
}

bool AMHPlayerCharacter::EquipWeaponInstance(AMHWeaponInstance* InWeapon, bool bDestroyPreviousWeapon)
{
    if (!IsValid(InWeapon))
    {
        return false;
    }

    UnequipCurrentWeapon(bDestroyPreviousWeapon);

    EquippedWeapon = InWeapon;
    EquippedWeapon->SetOwner(this);

    if (AbilitySystemComponent)
    {
        EquippedWeapon->GrantWeaponAbilities(AbilitySystemComponent);
    }

    WeaponSheathState = EMHWeaponSheathState::Sheathed;
    CurrentWeaponType = EquippedWeapon->GetWeaponType();
    CurrentWeaponTag = GetCurrentWeaponTypeGameplayTag();

    AttachWeaponActorToBack();
    AttachWeaponToBack();
    RefreshWeaponAnimationLayerState();
    
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
    SetSharpnessAttributeValues(0.0f, 0.0f);

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
    AttachWeaponToSocket(WeaponSocketConfig.HandSocketName);
}

void AMHPlayerCharacter::AttachWeaponToSocket(const FName& InSocketName)
{
    if (!EquippedWeapon || !GetMesh() || InSocketName.IsNone())
    {
        return;
    }

    USkeletalMeshComponent* BladeMesh = GetWeaponBladeMesh();
    if (!BladeMesh)
    {
        return;
    }

    BladeMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, InSocketName);
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

bool AMHPlayerCharacter::FindEquippedWeaponAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const
{
    if (IsGreatSwordEquipped())
    {
        const AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
        const UMHGreatSwordActionComponent* ActionComponent = GreatSword ? GreatSword->GetActionComponent() : nullptr;
        if (ActionComponent && ActionComponent->FindAttackMetaRow(InMoveTag, OutAttackMetaRow))
        {
            return true;
        }
    }

    return FindAttackMetaRow(InMoveTag, OutAttackMetaRow);
}

bool AMHPlayerCharacter::CanStartLongSwordMove(const FGameplayTag& InMoveTag) const
{
    if (!InMoveTag.IsValid())
    {
        return false;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker)
    {
        return bLongSwordSpiritThrustHelmbreakerReady && CurrentSpiritLevel >= 1;
    }

    FMHAttackMetaRow AttackMetaRow;
    const bool bHasAttackMeta = FindAttackMetaRow(InMoveTag, AttackMetaRow);
    if (!bHasAttackMeta && DoesLongSwordMoveRequireAttackMeta(InMoveTag))
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : LongSword attack meta missing. Move=%s"), *GetName(), *InMoveTag.ToString());
        return false;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritRoundslash
        && bLongSwordForesightFreeSpiritRoundslashReady)
    {
        return true;
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash || !bHasAttackMeta)
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

bool AMHPlayerCharacter::DoesLongSwordMoveRequireAttackMeta(const FGameplayTag& InMoveTag) const
{
    return InMoveTag.IsValid() && InMoveTag != MHLongSwordGameplayTags::Move_LS_SpecialSheathe;
}

bool AMHPlayerCharacter::DoesLongSwordMoveBuildDamageSpec(const FGameplayTag& InMoveTag) const
{
    return InMoveTag.IsValid() && InMoveTag != MHLongSwordGameplayTags::Move_LS_SpecialSheathe;
}

void AMHPlayerCharacter::PlayWeaponHitCameraShake(const FGameplayTag& InMoveTag) const
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

    if (FindEquippedWeaponAttackMetaRow(InMoveTag, AttackMetaRow))
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

void AMHPlayerCharacter::ApplyLongSwordMoveStartCost(const FGameplayTag& InMoveTag)
{
    if (!InMoveTag.IsValid())
    {
        return;
    }

    FMHAttackMetaRow AttackMetaRow;
    const bool bHasAttackMeta = FindAttackMetaRow(InMoveTag, AttackMetaRow);
    const float PreviousSpiritGauge = CurrentSpiritGauge;
    const int32 PreviousSpiritLevel = CurrentSpiritLevel;

    if (InMoveTag != MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker)
    {
        bLongSwordSpiritThrustHelmbreakerReady = false;
    }

    bool bConsumedForesightReward = false;

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash)
    {
        SetCurrentSpiritGauge(0.0f);
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritRoundslash
        && bLongSwordForesightFreeSpiritRoundslashReady)
    {
        bLongSwordForesightFreeSpiritRoundslashReady = false;
        bConsumedForesightReward = true;
    }
    else if (bHasAttackMeta)
    {
        ConsumeSpiritGauge(FMath::Max(0.0f, AttackMetaRow.SpiritGaugeConsume));
    }

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker)
    {
        bLongSwordSpiritThrustHelmbreakerReady = false;
        DecreaseSpiritLevel();
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword move started. Move=%s Gauge=%.2f->%.2f Level=%d->%d ForesightRewardConsumed=%d"),
        *GetName(),
        *InMoveTag.ToString(),
        PreviousSpiritGauge,
        CurrentSpiritGauge,
        PreviousSpiritLevel,
        CurrentSpiritLevel,
        bConsumedForesightReward ? 1 : 0
    );
}

void AMHPlayerCharacter::ApplyLongSwordMoveHitReward(const FGameplayTag& InMoveTag)
{
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

    AddSpiritGauge(FMath::Max(0.0f, AttackMetaRow.SpiritGaugeGain));

    if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritRoundslash)
    {
        IncreaseSpiritLevel();
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_SpiritThrust)
    {
        bLongSwordSpiritThrustHelmbreakerReady = true;
    }
    else if (InMoveTag == MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash && !bLongSwordSpecialSheatheSpiritCounterSuccess)
    {
        DecreaseSpiritLevel();
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword hit reward applied. Move=%s Gauge=%.2f->%.2f Level=%d->%d HelmbreakerReady=%d"),
        *GetName(),
        *InMoveTag.ToString(),
        PreviousSpiritGauge,
        CurrentSpiritGauge,
        PreviousSpiritLevel,
        CurrentSpiritLevel,
        bLongSwordSpiritThrustHelmbreakerReady ? 1 : 0
    );
}

void AMHPlayerCharacter::ApplyLongSwordCounterSuccessReward(const FGameplayTag& InMoveTag, const EMHLongSwordCounterWindowType InCounterWindowType)
{
    if (!InMoveTag.IsValid())
    {
        return;
    }

    switch (InCounterWindowType)
    {
    case EMHLongSwordCounterWindowType::Foresight:
        bLongSwordForesightFreeSpiritRoundslashReady = true;
        break;

    case EMHLongSwordCounterWindowType::SpecialSheatheSlash:
        break;

    case EMHLongSwordCounterWindowType::SpecialSheatheSpirit:
        break;

    default:
        break;
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : LongSword counter reward applied. Move=%s Window=%d ForesightRewardReady=%d"),
        *GetName(),
        *InMoveTag.ToString(),
        static_cast<int32>(InCounterWindowType),
        bLongSwordForesightFreeSpiritRoundslashReady ? 1 : 0
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

void AMHPlayerCharacter::SetSpiritGaugeValues(const float InSpiritValue, const float InMaxSpiritValue)
{
    MaxSpiritGauge = FMath::Max(0.f, InMaxSpiritValue);
    CurrentSpiritGauge = FMath::Clamp(InSpiritValue, 0.0f, MaxSpiritGauge);
    
    OnSpiritGaugeChanged.Broadcast(GetCurrentSpiritGaugeValue(), GetMaxSpiritGaugeValue());
}

void AMHPlayerCharacter::SetCurrentSpiritGauge(const float InSpiritValue)
{
    CurrentSpiritGauge = FMath::Clamp(InSpiritValue, 0.0f, MaxSpiritGauge);
    
    OnSpiritGaugeChanged.Broadcast(GetCurrentSpiritGaugeValue(), GetMaxSpiritGaugeValue());
}

void AMHPlayerCharacter::SetMaxSpiritGuage(const float InMaxSpiritValue)
{
    MaxSpiritGauge = FMath::Max(0.f, InMaxSpiritValue);
    
    OnSpiritGaugeChanged.Broadcast(GetCurrentSpiritGaugeValue(), GetMaxSpiritGaugeValue());
}

float AMHPlayerCharacter::ResolveLongSwordDamageMultiplier(const FGameplayTag& InMoveTag) const
{
    FMHAttackMetaRow AttackMetaRow;
    if (!FindAttackMetaRow(InMoveTag, AttackMetaRow))
    {
        if (DoesLongSwordMoveRequireAttackMeta(InMoveTag))
        {
            UE_LOG(LogMHPlayerCharacter, Warning, TEXT("%s : LongSword damage multiplier resolve failed. Move=%s"), *GetName(), *InMoveTag.ToString());
            return 0.0f;
        }

        return GetCurrentSpiritDamageMultiplier();
    }

    return FMath::Max(0.0f, AttackMetaRow.DamageMultiplier) * GetCurrentSpiritDamageMultiplier();
}

#pragma region GreatSwordRuntimeFunctions
bool AMHPlayerCharacter::IsGreatSwordEquipped() const
{
    return CurrentWeaponType == EMHWeaponType::GreatSword && Cast<AMHGreatSwordInstance>(EquippedWeapon) != nullptr;
}

bool AMHPlayerCharacter::TryHandleGreatSwordPatternInput(const FGameplayTag& InPatternTag, const bool bInPromoteSheathedToUnsheathing)
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    UMHGreatSwordActionComponent* ActionComponent = GreatSword ? GreatSword->GetActionComponent() : nullptr;
    if (!ActionComponent || !InPatternTag.IsValid())
    {
        return false;
    }

    // 해석된 입력을 이번 프레임에 실행하지 못하면 이 스냅샷으로 되돌린다.
    FMHGreatSwordRuntimeSnapshot ActionSnapshot;
    ActionComponent->CaptureRuntimeSnapshot(ActionSnapshot);

    const EMHWeaponSheathState PreviousSheathState = WeaponSheathState;
    const bool bPreviousPendingUnsheathe = bPendingUnsheatheFromComboEntry;
    const bool bWasSheathed = WeaponSheathState == EMHWeaponSheathState::Sheathed;

    if (!ActionComponent->HandleInputPatternWithBuffering(InPatternTag))
    {
        return false;
    }

    if (bInPromoteSheathedToUnsheathing && bWasSheathed)
    {
        WeaponSheathState = EMHWeaponSheathState::Unsheathing;
        bPendingUnsheatheFromComboEntry = true;
    }

    if (TryExecuteGreatSwordPendingMove())
    {
        return true;
    }

    WeaponSheathState = PreviousSheathState;
    bPendingUnsheatheFromComboEntry = bPreviousPendingUnsheathe;
    ActionComponent->RestoreRuntimeSnapshot(ActionSnapshot);
    return false;
}

bool AMHPlayerCharacter::TryHandleGreatSwordPrimaryInput()
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    UMHGreatSwordActionComponent* ActionComponent = GreatSword ? GreatSword->GetActionComponent() : nullptr;
    if (!ActionComponent)
    {
        return false;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathing)
    {
        return false;
    }

    const bool bForwardInput = CachedMoveInput2D.Y > 0.1f;
    const bool bSheathed = WeaponSheathState == EMHWeaponSheathState::Sheathed;
    const FGameplayTag PatternTag = ResolveGreatSwordPrimaryPatternTag(ActionComponent, bForwardInput, bSheathed);
    return TryHandleGreatSwordPatternInput(PatternTag, bSheathed);
}

bool AMHPlayerCharacter::TryHandleGreatSwordPrimaryRelease()
{
    return TryHandleGreatSwordPatternInput(MHInputPatternGameplayTags::InputPattern_GS_PrimaryRelease, false);
}

bool AMHPlayerCharacter::TryHandleGreatSwordSecondaryInput()
{
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    return TryHandleGreatSwordPatternInput(MHInputPatternGameplayTags::InputPattern_GS_Secondary, false);
}

bool AMHPlayerCharacter::TryHandleGreatSwordWeaponSpecialInput()
{
    const bool bSheathed = WeaponSheathState == EMHWeaponSheathState::Sheathed;
    const FGameplayTag PatternTag = ResolveGreatSwordWeaponSpecialPatternTag(bSheathed);
    return TryHandleGreatSwordPatternInput(PatternTag, bSheathed);
}

bool AMHPlayerCharacter::TryHandleGreatSwordWeaponSpecialRelease()
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    UMHGreatSwordActionComponent* ActionComponent = GreatSword ? GreatSword->GetActionComponent() : nullptr;
    if (!ActionComponent)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance || !ActiveGreatSwordUtilityMontage)
    {
        return false;
    }

    if (!ActionComponent->HandleResolvedInputPattern(MHInputPatternGameplayTags::InputPattern_GS_WeaponSpecialRelease))
    {
        return false;
    }

    AnimInstance->Montage_JumpToSection(TEXT("guard_end"), ActiveGreatSwordUtilityMontage);
    return true;
}

bool AMHPlayerCharacter::TryHandleGreatSwordSimultaneousInput()
{
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    return TryHandleGreatSwordPatternInput(MHInputPatternGameplayTags::InputPattern_GS_Simultaneous, false);
}

bool AMHPlayerCharacter::TryExecuteGreatSwordPendingMove()
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    if (!GreatSword)
    {
        return false;
    }

    UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent();
    if (!ActionComponent)
    {
        return false;
    }

    if (!ActionComponent->HasPendingMove())
    {
        return true;
    }

    // 유틸리티 기술은 몽타주로 처리하고, 공격 기술은 기본 어빌리티 경로로 넘긴다.
    const FGameplayTag PendingMoveTag = ActionComponent->GetPendingMoveTag();
    if (ActionComponent->IsUtilityMoveTag(PendingMoveTag))
    {
        return TryPlayGreatSwordUtilityMontage(PendingMoveTag);
    }

    if (IsEquippedWeaponPrimaryAbilityActive())
    {
        UE_LOG(
            LogMHPlayerCharacter,
            Verbose,
            TEXT("%s : 활성 중인 대검 공격 어빌리티를 재트리거합니다. PendingMove=%s"),
            *GetName(),
            *PendingMoveTag.ToString()
        );
    }

    const bool bActivated = TryActivateGreatSwordPrimaryAbility();
    if (!bActivated)
    {
        UE_LOG(
            LogMHPlayerCharacter,
            Warning,
            TEXT("%s : 대검 후속 공격 실행에 실패했습니다. PendingMove=%s AbilityActive=%d"),
            *GetName(),
            *PendingMoveTag.ToString(),
            IsEquippedWeaponPrimaryAbilityActive() ? 1 : 0
        );
    }

    return bActivated;
}

bool AMHPlayerCharacter::TryActivateGreatSwordPrimaryAbility()
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    if (!GreatSword || !AbilitySystemComponent)
    {
        return false;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = GreatSword->GetPrimaryAttackAbilityClass();
    if (AbilityClass == nullptr)
    {
        return false;
    }

    return AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
}

bool AMHPlayerCharacter::TryPlayGreatSwordUtilityMontage(const FGameplayTag& InMoveTag)
{
    AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    if (!GreatSword || !GreatSword->GetActionComponent())
    {
        return false;
    }

    UAnimMontage* Montage = GreatSword->GetActionComponent()->ResolveMontageForMove(InMoveTag);
    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!Montage || !AnimInstance)
    {
        return false;
    }

    const float PlayedLength = AnimInstance->Montage_Play(Montage);
    if (PlayedLength <= 0.0f)
    {
        return false;
    }

    ActiveGreatSwordUtilityMontage = Montage;
    ActiveGreatSwordUtilityMoveTag = InMoveTag;
    GreatSword->GetActionComponent()->ConsumePendingMoveTag();
    GreatSword->GetActionComponent()->NotifyUtilityMoveStarted(InMoveTag);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleGreatSwordUtilityMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

    if (bPendingUnsheatheFromComboEntry)
    {
        HandleComboMontageStateTransition(false);
    }

    return true;
}

void AMHPlayerCharacter::HandleGreatSwordUtilityMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != ActiveGreatSwordUtilityMontage)
    {
        return;
    }

    if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyUtilityMoveEnded(ActiveGreatSwordUtilityMoveTag, bInterrupted);
        }
    }

    ActiveGreatSwordUtilityMontage = nullptr;
    ActiveGreatSwordUtilityMoveTag = FGameplayTag();
}

bool AMHPlayerCharacter::IsGreatSwordAttackChainDodgeContext() const
{
    if (!IsGreatSwordEquipped() || WeaponSheathState != EMHWeaponSheathState::Unsheathed)
    {
        return false;
    }

    const AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon);
    return GreatSword && GreatSword->GetActionComponent() && GreatSword->GetActionComponent()->IsAttackRollWindowOpen();
}
#pragma endregion

#pragma endregion

float AMHPlayerCharacter::GetCurrentHealthValue() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetHealth() : 0.0f;
}

float AMHPlayerCharacter::GetMaxHealthValue() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetMaxHealth() : 0.0f;
}

float AMHPlayerCharacter::GetCurrentHealableHealthValue() const
{
    return HealthAttributeSet ? FMath::Max(0.0f, HealthAttributeSet->GetHealableHealth()) : 0.0f;
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

    const float NewSpiritGauge = FMath::Clamp(CurrentSpiritGauge + InAmount, 0.0f, FMath::Max(0.0f, MaxSpiritGauge));
    SetCurrentSpiritGauge(NewSpiritGauge);
}

void AMHPlayerCharacter::ConsumeSpiritGauge(const float InAmount)
{
    if (InAmount <= 0.0f)
    {
        return;
    }

    const float NewSpiritGauge = FMath::Clamp(CurrentSpiritGauge - InAmount, 0.0f, FMath::Max(0.0f, MaxSpiritGauge));
    SetCurrentSpiritGauge(NewSpiritGauge);
}

void AMHPlayerCharacter::IncreaseSpiritLevel(const int32 InAmount)
{
    if (InAmount <= 0)
    {
        return;
    }

    const int32 PreviousSpiritLevel = CurrentSpiritLevel;
    CurrentSpiritLevel = FMath::Clamp(CurrentSpiritLevel + InAmount, 0, 3);

    if (CurrentSpiritLevel != PreviousSpiritLevel)
    {
        RefreshSpiritLevelDecayState(true);
        BroadcastSpiritLevelChanged();
    }
}

void AMHPlayerCharacter::DecreaseSpiritLevel(const int32 InAmount)
{
    if (InAmount <= 0)
    {
        return;
    }

    const int32 PreviousSpiritLevel = CurrentSpiritLevel;
    CurrentSpiritLevel = FMath::Clamp(CurrentSpiritLevel - InAmount, 0, 3);

    if (CurrentSpiritLevel != PreviousSpiritLevel)
    {
        RefreshSpiritLevelDecayState(CurrentSpiritLevel > 0);
        BroadcastSpiritLevelChanged();
    }
}

float AMHPlayerCharacter::GetSpiritLevelRemainingTime() const
{
    return FMath::Max(0.0f, SpiritLevelRemainingTime);
}

float AMHPlayerCharacter::GetSpiritLevelDuration() const
{
    return FMath::Max(0.0f, SpiritLevelDuration);
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

    if (IsLongSwordAttackChainDodgeContext() || IsGreatSwordAttackChainDodgeContext())
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
    HandleBurnRollSucceeded();

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
    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Weapon GE apply state. HasAuthority=%d bGASInitialized=%d ASC=%d ActorInfoValid=%d WeaponClass=%s Weapon=%s"),
    *GetName(),
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
    // -----------------------

    FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
    ContextHandle.AddInstigator(this, this); // ?곕떽?
    ContextHandle.AddSourceObject(EquippedWeapon);

    FGameplayEffectSpecHandle SpecHandle =
        AbilitySystemComponent->MakeOutgoingSpec(WeaponStatEffectClass, 1.0f, ContextHandle);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        return;
    }

    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Weapon GE spec ready. SpecValid=%d DataValid=%d ItemAP=%.2f ItemAffinity=%.2f"),
    *GetName(),
    SpecHandle.IsValid() ? 1 : 0,
    (SpecHandle.IsValid() && SpecHandle.Data.IsValid()) ? 1 : 0,
    Stat.AttackPower,
    Stat.Affinity);


    SpecHandle.Data->SetSetByCallerMagnitude(
        MHGameplayTags::Data_Weapon_AttackPower,
        Stat.AttackPower);

    SpecHandle.Data->SetSetByCallerMagnitude(
        MHGameplayTags::Data_Weapon_Affinity,
        Stat.Affinity);

    EquippedWeaponStatEffectHandle =
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

    // -----------------------
    // -----------------------

    CurrentSharpnessColor = Stat.MaxSharpnessColor;
    CurrentSharpnessValue = GetTotalSharpnessLength(Stat.SharpnessLength);
    SetSharpnessAttributeValues(CurrentSharpnessValue, CurrentSharpnessValue);

    // -----------------------
    // -----------------------

    CurrentWeaponElementTag = Stat.AttackElementTag;
    
    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Weapon GE applied. HandleValid=%d Weapon=%s ItemAP=%.2f ItemAffinity=%.2f ASC_AP=%.2f ASC_CR=%.2f"),
    *GetName(),
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

    if (CombatAttributeSet)
    {
        CombatAttributeSet->SetSharpnessModifier(1.0f);
    }

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UMHCombatAttributeSet::GetSharpnessModifierAttribute(), 1.0f);
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Verbose,
        TEXT("%s : Weapon stat effect removed. SharpnessModifier reset to 1.0"),
        *GetName()
    );
}

void AMHPlayerCharacter::RefreshEquippedWeaponStatEffect()
{
    RemoveEquippedWeaponStatEffect();
    ApplyEquippedWeaponStatEffect();
}

void AMHPlayerCharacter::SetSharpnessAttributeValues(float InCurrentSharpness, float InMaxSharpness)
{
    const float ClampedMaxSharpness = FMath::Max(0.0f, InMaxSharpness);
    const float ClampedCurrentSharpness = FMath::Clamp(InCurrentSharpness, 0.0f, ClampedMaxSharpness);

    bSyncingSharpnessState = true;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(
            UMHPlayerAttributeSet::GetMaxSharpnessAttribute(),
            ClampedMaxSharpness);
        AbilitySystemComponent->SetNumericAttributeBase(
            UMHPlayerAttributeSet::GetSharpnessAttribute(),
            ClampedCurrentSharpness);
    }
    else if (PlayerAttributeSet)
    {
        PlayerAttributeSet->SetMaxSharpness(ClampedMaxSharpness);
        PlayerAttributeSet->SetSharpness(ClampedCurrentSharpness);
    }

    bSyncingSharpnessState = false;

    NormalizeSharpnessStateFromAttribute();
    OnSharpnessChanged.Broadcast(GetCurrentSharpnessValue(), GetMaxSharpnessValue());

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Sharpness] Set Attributes Current=%.1f Max=%.1f Color=%s Modifier=%.2f"),
        GetCurrentSharpnessValue(),
        GetMaxSharpnessValue(),
        ResolveSharpnessColorText(CurrentSharpnessColor),
        CombatAttributeSet ? CombatAttributeSet->GetSharpnessModifier() : -1.0f);
}

void AMHPlayerCharacter::UpdateSharpnessModifierFromCurrentColor()
{
    const float SharpnessModifier =
        (EquippedWeapon && GetMaxSharpnessValue() > 0.0f)
        ? GetSharpnessMultiplier(CurrentSharpnessColor)
        : 1.0f;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetNumericAttributeBase(
            UMHCombatAttributeSet::GetSharpnessModifierAttribute(),
            SharpnessModifier);
    }
    else if (CombatAttributeSet)
    {
        CombatAttributeSet->SetSharpnessModifier(SharpnessModifier);
    }
}

void AMHPlayerCharacter::ConsumeSharpness(float Amount)
{
    const float MaxSharpness = GetMaxSharpnessValue();
    if (MaxSharpness <= 0.0f)
    {
        return;
    }

    const float CurrentSharpness = GetCurrentSharpnessValue();
    const float NewSharpness = FMath::Clamp(CurrentSharpness - FMath::Max(0.0f, Amount), 0.0f, MaxSharpness);
    SetSharpnessAttributeValues(NewSharpness, MaxSharpness);
}

float AMHPlayerCharacter::GetCurrentSharpnessValue() const
{
    return PlayerAttributeSet
        ? FMath::Max(0.0f, PlayerAttributeSet->GetSharpness())
        : FMath::Max(0.0f, CurrentSharpnessValue);
}

float AMHPlayerCharacter::GetMaxSharpnessValue() const
{
    if (PlayerAttributeSet)
    {
        return FMath::Max(0.0f, PlayerAttributeSet->GetMaxSharpness());
    }

    if (!EquippedWeapon)
    {
        return 0.0f;
    }

    return GetTotalSharpnessLength(EquippedWeapon->GetAttackStats().SharpnessLength);
}

float AMHPlayerCharacter::GetCurrentSharpnessSegmentValue() const
{
    if (!EquippedWeapon)
    {
        return 0.0f;
    }

    float SegmentValue = 0.0f;
    float SegmentMax = 0.0f;
    GetSharpnessSegmentValues(
        EquippedWeapon->GetAttackStats().SharpnessLength,
        CurrentSharpnessColor,
        GetCurrentSharpnessValue(),
        SegmentValue,
        SegmentMax);

    return SegmentValue;
}

float AMHPlayerCharacter::GetCurrentSharpnessSegmentMax() const
{
    if (!EquippedWeapon)
    {
        return 0.0f;
    }

    float SegmentValue = 0.0f;
    float SegmentMax = 0.0f;
    GetSharpnessSegmentValues(
        EquippedWeapon->GetAttackStats().SharpnessLength,
        CurrentSharpnessColor,
        GetCurrentSharpnessValue(),
        SegmentValue,
        SegmentMax);

    return SegmentMax;
}

EMHHitResultType AMHPlayerCharacter::HandleWeaponAttackHit(
    AActor* Target,
    AMHWeaponInstance* Weapon)
{
    (void)Target;

    // 1. Sharpness
    if (!Weapon || Weapon != EquippedWeapon)
    {
        return EMHHitResultType::NormalHit;
    }

    if (GetMaxSharpnessValue() <= 0.0f)
    {
        return EMHHitResultType::NormalHit;
    }

    if (GetCurrentSharpnessValue() <= 0.0f)
    {
        UE_LOG(
            LogMHPlayerCharacter,
            Log,
            TEXT("[Sharpness] Bounce Weapon=%s Current=%.1f Max=%.1f Color=%s"),
            *GetNameSafe(Weapon),
            GetCurrentSharpnessValue(),
            GetMaxSharpnessValue(),
            ResolveSharpnessColorText(CurrentSharpnessColor));
        return EMHHitResultType::Bounced;
    }

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Sharpness] Hit Before Current=%.1f Max=%.1f Color=%s"),
        GetCurrentSharpnessValue(),
        GetMaxSharpnessValue(),
        ResolveSharpnessColorText(CurrentSharpnessColor));
    ConsumeSharpness(5.f);
    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Sharpness] Hit After Current=%.1f Max=%.1f Color=%s"),
        GetCurrentSharpnessValue(),
        GetMaxSharpnessValue(),
        ResolveSharpnessColorText(CurrentSharpnessColor));
    return EMHHitResultType::NormalHit;

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

    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSlash;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return HasMovementInputForCombat() ? InputPattern_LS_DrawAdvancingSlash : InputPattern_LS_DrawOnly;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

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

    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

    if (bAttackSecondaryHeld)
    {
        return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

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

    if (IsInLongSwordSpecialSheatheState())
    {
        return FGameplayTag::EmptyTag;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return FGameplayTag::EmptyTag;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bWeaponSpecialHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    if (bAttackPrimaryHeld)
    {
        return ShouldUseLateralFadeSlashPattern() ? InputPattern_LS_LateralFadeSlash : InputPattern_LS_FadeSlash;
    }

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


    if (IsInLongSwordSpecialSheatheState())
    {
        return InputPattern_LS_IaiSpiritSlash;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Sheathed)
    {
        return InputPattern_LS_DrawSpiritSlash1;
    }

    const bool bAllowFollowupDuringUnsheathing = CanResolveLongSwordFollowupDuringUnsheathing();
    if (WeaponSheathState != EMHWeaponSheathState::Unsheathed && !bAllowFollowupDuringUnsheathing)
    {
        return FGameplayTag::EmptyTag;
    }

    if (bAttackSecondaryHeld)
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

    if (bWeaponSpecialHeld && bDodgeHeld)
    {
        return InputPattern_LS_SpecialSheathe;
    }

    if (bWeaponSpecialHeld && bAttackSecondaryHeld)
    {
        return InputPattern_LS_ForesightSlash;
    }

    if (bWeaponSpecialHeld && bAttackPrimaryHeld)
    {
        return InputPattern_LS_SpiritThrust;
    }

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
    const EMHDirectionalVariant DirectionalVariant = ResolveDirectionalVariantFromInput(false);

    return DirectionalVariant == EMHDirectionalVariant::Left
        || DirectionalVariant == EMHDirectionalVariant::Right;
}

bool AMHPlayerCharacter::ShouldUseLateralFadeSlashPattern() const
{
    return IsLongSwordFollowupContext() && ShouldUseDirectionalLateralFadeSlash();
}

FGameplayTag AMHPlayerCharacter::GetCurrentWeaponTypeGameplayTag() const
{
    switch (CurrentWeaponType)
    {
    case EMHWeaponType::LongSword:
        return MHCombatStateGameplayTags::WeaponType_LongSword;
    case EMHWeaponType::GreatSword:
        return MHCombatStateGameplayTags::WeaponType_GreatSword;
    default:
        return FGameplayTag::EmptyTag;
    }
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

    const bool bAllowFollowupDuringUnsheathing =
        bComboActive && WeaponSheathState == EMHWeaponSheathState::Unsheathing;

    if (WeaponSheathState == EMHWeaponSheathState::Sheathing)
    {
        return false;
    }

    if (WeaponSheathState == EMHWeaponSheathState::Unsheathing && !bAllowFollowupDuringUnsheathing)
    {
        return false;
    }

    if (!bComboActive && WeaponSheathState == EMHWeaponSheathState::Sheathed && !bDrawEntryFromSheathed)
    {
        return false;
    }

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

bool AMHPlayerCharacter::IsEquippedWeaponPrimaryAbilityActive() const
{
    if (!AbilitySystemComponent || !EquippedWeapon)
    {
        return false;
    }

    const TSubclassOf<UGameplayAbility> AbilityClass = EquippedWeapon->GetPrimaryAttackAbilityClass();
    if (!AbilityClass)
    {
        return false;
    }

    const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
    return Spec && Spec->IsActive();
}

bool AMHPlayerCharacter::EndActiveEquippedWeaponAttackAbility(bool bWasCancelled)
{
    if (!AbilitySystemComponent || !EquippedWeapon)
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
    if (UMHGA_LongSwordCombo* ComboAbility = Cast<UMHGA_LongSwordCombo>(ActiveAbility))
    {
        ComboAbility->RequestExternalEndAbility(bWasCancelled);
        return true;
    }

    if (UMHGA_GreatSwordAttack* GreatSwordAbility = Cast<UMHGA_GreatSwordAttack>(ActiveAbility))
    {
        GreatSwordAbility->RequestExternalEndAbility(bWasCancelled);
        return true;
    }

    return false;
}

bool AMHPlayerCharacter::CancelSharpenAbilityIfActive()
{
    if (!AbilitySystemComponent || !SharpenAbilityClass)
    {
        return false;
    }

    FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(SharpenAbilityClass);
    if (!Spec || !Spec->IsActive())
    {
        return false;
    }

    UGameplayAbility* ActiveAbility = Spec->GetPrimaryInstance();
    UGA_MHSharpen* SharpenAbility = Cast<UGA_MHSharpen>(ActiveAbility);
    if (!SharpenAbility)
    {
        return false;
    }

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Sharpen] Cancelled by external action"));
    SharpenAbility->RequestExternalEndAbility(true);
    return true;
}

bool AMHPlayerCharacter::CanStartSharpenItemUse() const
{
    if (!EquippedWeapon || GetMaxSharpnessValue() <= 0.0f)
    {
        return false;
    }

    if (WeaponSheathState != EMHWeaponSheathState::Sheathed)
    {
        return false;
    }
    if (GetCurrentSharpnessValue() >= GetMaxSharpnessValue())
    {
        return false;
    }

    if (GetVelocity().Size2D() > 3.0f)
    {
        return false;
    }

    if (bRollMontagePlaying)
    {
        return false;
    }

    return !IsEquippedWeaponPrimaryAbilityActive();
}

void AMHPlayerCharacter::HandleSharpnessBounce()
{
    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Sharpness] Handle bounce. WeaponType=%d"),
        static_cast<int32>(CurrentWeaponType)
    );

    EndActiveEquippedWeaponAttackAbility(true);

    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(EquippedWeapon))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            ComboComp->ResetCombo();
        }

        HandleComboMontageStateTransition(true);
    }
    else if (AMHGreatSwordInstance* GreatSword = Cast<AMHGreatSwordInstance>(EquippedWeapon))
    {
        if (UMHGreatSwordActionComponent* ActionComponent = GreatSword->GetActionComponent())
        {
            ActionComponent->NotifyActionFinished();
        }
    }

    if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        if (ActiveSharpnessBounceMontage)
        {
            UAnimMontage* PreviousBounceMontage = ActiveSharpnessBounceMontage;
            ActiveSharpnessBounceMontage = nullptr;
            SetSharpnessBounceInputLock(false);
            AnimInstance->Montage_Stop(0.05f, PreviousBounceMontage);
        }

        AnimInstance->Montage_Stop(0.05f);

        if (SharpnessBounceMontage)
        {
            const float PlayedLength = AnimInstance->Montage_Play(SharpnessBounceMontage);
            if (PlayedLength > 0.0f)
            {
                ActiveSharpnessBounceMontage = SharpnessBounceMontage;
                SetSharpnessBounceInputLock(true);

                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &AMHPlayerCharacter::HandleSharpnessBounceMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, SharpnessBounceMontage);
            }
        }
    }
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

#pragma region AttributeDelegate

void AMHPlayerCharacter::NormalizeSharpnessStateFromAttribute()
{
    if (!PlayerAttributeSet)
    {
        CurrentSharpnessColor = EMHSharpnessColor::Red;
        CurrentSharpnessValue = 0.0f;
        UpdateSharpnessModifierFromCurrentColor();
        return;
    }

    const float MaxSharpness = FMath::Max(0.0f, PlayerAttributeSet->GetMaxSharpness());
    const float CurrentSharpness = FMath::Clamp(PlayerAttributeSet->GetSharpness(), 0.0f, MaxSharpness);

    CurrentSharpnessValue = CurrentSharpness;

    if (EquippedWeapon && MaxSharpness > 0.0f)
    {
        CurrentSharpnessColor = ResolveSharpnessColorFromValue(
            EquippedWeapon->GetAttackStats().SharpnessLength,
            CurrentSharpness);
    }
    else
    {
        CurrentSharpnessColor = EMHSharpnessColor::Red;
    }

    UpdateSharpnessModifierFromCurrentColor();
}

EMHSharpnessColor AMHPlayerCharacter::GetLowerSharpnessColor(EMHSharpnessColor InColor) const
{
    switch (InColor)
    {
    case EMHSharpnessColor::White:  return EMHSharpnessColor::Blue;
    case EMHSharpnessColor::Blue:   return EMHSharpnessColor::Green;
    case EMHSharpnessColor::Green:  return EMHSharpnessColor::Yellow;
    case EMHSharpnessColor::Yellow: return EMHSharpnessColor::Orange;
    case EMHSharpnessColor::Orange: return EMHSharpnessColor::Red;
    case EMHSharpnessColor::Red:
    default:
        return EMHSharpnessColor::Red;
    }
}

EMHSharpnessColor AMHPlayerCharacter::GetHigherSharpnessColor(EMHSharpnessColor InColor) const
{
    switch (InColor)
    {
    case EMHSharpnessColor::Red:    return EMHSharpnessColor::Orange;
    case EMHSharpnessColor::Orange: return EMHSharpnessColor::Yellow;
    case EMHSharpnessColor::Yellow: return EMHSharpnessColor::Green;
    case EMHSharpnessColor::Green:  return EMHSharpnessColor::Blue;
    case EMHSharpnessColor::Blue:   return EMHSharpnessColor::White;
    case EMHSharpnessColor::White:
    default:
        return EMHSharpnessColor::White;
    }
}

bool AMHPlayerCharacter::CanUpgradeSharpnessColor() const
{
    if (!EquippedWeapon)
    {
        return false;
    }

    return CurrentSharpnessColor != EquippedWeapon->GetAttackStats().MaxSharpnessColor;
}

void AMHPlayerCharacter::Input_ItemSelectSharpen(const FInputActionValue& InputActionValue)
{
    (void)InputActionValue;

    SelectedConsumable = EMHConsumableSelection::Sharpen;
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Item] Selected=Sharpen"));
}

void AMHPlayerCharacter::Input_ItemSelectPotion(const FInputActionValue& InputActionValue)
{
    (void)InputActionValue;

    SelectedConsumable = EMHConsumableSelection::Potion;
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Item] Selected=Potion"));
}

void AMHPlayerCharacter::Input_ItemUseStarted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    (void)InputActionValue;

    bItemUseHeld = true;
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Item] Use Started Selected=%d"), static_cast<int32>(SelectedConsumable));
    TryUseSelectedItem();
}

void AMHPlayerCharacter::Input_ItemUseCompleted(const FInputActionValue& InputActionValue)
{
    if (bActionInputLocked)
    {
        return;
    }

    (void)InputActionValue;

    bItemUseHeld = false;
    UE_LOG(LogMHPlayerCharacter, Log, TEXT("[Item] Use Completed"));
}

void AMHPlayerCharacter::TryUseSelectedItem()
{
    if (bActionInputLocked)
    {
        return;
    }

    if (!AbilitySystemComponent)
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("[Item] TryUseSelectedItem failed: ASC null"));
        return;
    }

    TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

    switch (SelectedConsumable)
    {
    case EMHConsumableSelection::Sharpen:
        if (!CanStartSharpenItemUse())
        {
            UE_LOG(
                LogMHPlayerCharacter,
                Log,
                TEXT("[Sharpen] TryUse rejected Current=%.1f Max=%.1f Velocity=%.2f AttackActive=%d Roll=%d"),
                GetCurrentSharpnessValue(),
                GetMaxSharpnessValue(),
                GetVelocity().Size2D(),
                IsEquippedWeaponPrimaryAbilityActive() ? 1 : 0,
                bRollMontagePlaying ? 1 : 0
            );
            return;
        }

        AbilityClass = SharpenAbilityClass;
        break;
    case EMHConsumableSelection::Potion:
        AbilityClass = PotionAbilityClass;
        break;
    case EMHConsumableSelection::None:
    default:
        break;
    }

    if (!AbilityClass)
    {
        UE_LOG(LogMHPlayerCharacter, Warning, TEXT("[Item] TryUseSelectedItem failed: AbilityClass null"));
        return;
    }

    const bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Item] TryUseSelectedItem Ability=%s Activated=%d"),
        *GetNameSafe(AbilityClass),
        bActivated ? 1 : 0);
}

void AMHPlayerCharacter::RefreshSharpnessState()
{
    NormalizeSharpnessStateFromAttribute();
    OnSharpnessChanged.Broadcast(GetCurrentSharpnessValue(), GetMaxSharpnessValue());

    UE_LOG(
        LogMHPlayerCharacter,
        Log,
        TEXT("[Sharpness] Refresh Current=%.1f Max=%.1f Color=%s Modifier=%.2f"),
        GetCurrentSharpnessValue(),
        GetMaxSharpnessValue(),
        ResolveSharpnessColorText(CurrentSharpnessColor),
        CombatAttributeSet ? CombatAttributeSet->GetSharpnessModifier() : -1.0f);
}

void AMHPlayerCharacter::InitializeAbilitySystem()
{
    Super::InitializeAbilitySystem();
    if (AbilitySystemComponent && HasAuthority())
    {
        if (SharpenAbilityClass && !AbilitySystemComponent->FindAbilitySpecFromClass(SharpenAbilityClass))
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SharpenAbilityClass, 1, INDEX_NONE, this));
        }

        if (PotionAbilityClass && !AbilitySystemComponent->FindAbilitySpecFromClass(PotionAbilityClass))
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PotionAbilityClass, 1, INDEX_NONE, this));
        }
    }
    BindAttributeDelegates();
    BroadcastInitialAttributeSnapshot();
}

void AMHPlayerCharacter::BindAttributeDelegates()
{
    if (!AbilitySystemComponent || bAttributeDelegatesBound)
    {
        return;
    }

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHHealthAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &ThisClass::HandleHealthAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHHealthAttributeSet::GetMaxHealthAttribute()
    ).AddUObject(this, &ThisClass::HandleMaxHealthAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHHealthAttributeSet::GetHealableHealthAttribute()
    ).AddUObject(this, &ThisClass::HandleHealableHealthAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHPlayerAttributeSet::GetStaminaAttribute()
    ).AddUObject(this, &ThisClass::HandleStaminaAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHPlayerAttributeSet::GetSharpnessAttribute()
    ).AddUObject(this, &ThisClass::HandleShapnessAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHPlayerAttributeSet::GetMaxSharpnessAttribute()
    ).AddUObject(this, &ThisClass::HandleMaxShapnessAttributeChanged);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        UMHPlayerAttributeSet::GetMaxStaminaAttribute()
    ).AddUObject(this, &ThisClass::HandleMaxStaminaAttributeChanged);
    
    
    // AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    //     UMHHealthAttributeSet::GetMaxHealthAttribute()
    // ).AddUObject(this, &ThisClass::HandleMaxHealthAttributeChanged);
    //
    // AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    //     UMHPlayerAttributeSet::GetMaxStaminaAttribute()
    // ).AddUObject(this, &ThisClass::HandleMaxStaminaAttributeChanged);

    bAttributeDelegatesBound = true;
}

void AMHPlayerCharacter::BroadcastInitialAttributeSnapshot()
{
    OnHealthChanged.Broadcast(GetCurrentHealthValue(), GetMaxHealthValue());
    OnHealableHealthChanged.Broadcast(GetCurrentHealableHealthValue(), GetMaxHealthValue());
    OnStaminaChanged.Broadcast(GetCurrentStaminaValue(), GetMaxStaminaValue());
    OnSpiritGaugeChanged.Broadcast(GetCurrentSpiritGaugeValue(), GetMaxSpiritGaugeValue());
    BroadcastSpiritLevelChanged();
    BroadcastSpiritLevelTimerChanged();
    OnSharpnessChanged.Broadcast(GetCurrentSharpnessValue(), GetMaxSharpnessValue());
}

void AMHPlayerCharacter::HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnHealthChanged.Broadcast(ChangeData.NewValue, GetMaxHealthValue());
}

void AMHPlayerCharacter::HandleMaxHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnHealthChanged.Broadcast(GetCurrentHealthValue(), ChangeData.NewValue);
}

void AMHPlayerCharacter::HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnStaminaChanged.Broadcast(ChangeData.NewValue, GetMaxStaminaValue());
}

void AMHPlayerCharacter::HandleMaxStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnStaminaChanged.Broadcast(GetCurrentStaminaValue(), ChangeData.NewValue);
}

void AMHPlayerCharacter::HandleHealableHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnHealableHealthChanged.Broadcast(ChangeData.NewValue, GetMaxHealthValue());
}

void AMHPlayerCharacter::HandleShapnessAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    (void)ChangeData;

    if (bSyncingSharpnessState)
    {
        return;
    }

    RefreshSharpnessState();
    // OnSharpnessChanged.Broadcast(ChangeData.NewValue, GetCurrentSharpnessGaugeValue());
}

void AMHPlayerCharacter::HandleMaxShapnessAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    (void)ChangeData;

    if (bSyncingSharpnessState)
    {
        return;
    }

    RefreshSharpnessState();
    // OnSharpnessChanged.Broadcast(ChangeData.NewValue, GetMaxSharpnessGaugeValue());
}

void AMHPlayerCharacter::HandleSpiritAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnSpiritGaugeChanged.Broadcast(ChangeData.NewValue, GetMaxSpiritGaugeValue());
}

void AMHPlayerCharacter::HandleMaxSpiritAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    OnSpiritGaugeChanged.Broadcast(GetCurrentSpiritGaugeValue(), ChangeData.NewValue);
}

void AMHPlayerCharacter::RefreshSpiritLevelDecayState(const bool bResetTimer)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (CurrentSpiritLevel <= 0)
    {
        SpiritLevelRemainingTime = 0.0f;
        World->GetTimerManager().ClearTimer(SpiritLevelDecayTimerHandle);
        BroadcastSpiritLevelTimerChanged();
        return;
    }

    if (bResetTimer || SpiritLevelRemainingTime <= 0.0f)
    {
        SpiritLevelRemainingTime = GetSpiritLevelDuration();
    }

    if (!World->GetTimerManager().IsTimerActive(SpiritLevelDecayTimerHandle))
    {
        World->GetTimerManager().SetTimer(
            SpiritLevelDecayTimerHandle,
            this,
            &ThisClass::HandleSpiritLevelDecayTick,
            FMath::Max(0.01f, SpiritLevelDecayTickInterval),
            true
        );
    }

    BroadcastSpiritLevelTimerChanged();
}

void AMHPlayerCharacter::HandleSpiritLevelDecayTick()
{
    if (CurrentSpiritLevel <= 0)
    {
        RefreshSpiritLevelDecayState(false);
        return;
    }

    SpiritLevelRemainingTime = FMath::Max(
        0.0f,
        SpiritLevelRemainingTime - FMath::Max(0.01f, SpiritLevelDecayTickInterval)
    );
    BroadcastSpiritLevelTimerChanged();

    if (SpiritLevelRemainingTime > 0.0f)
    {
        return;
    }

    DecreaseSpiritLevel();
}

void AMHPlayerCharacter::BroadcastSpiritLevelChanged()
{
    OnSpiritLevelChanged.Broadcast(GetCurrentSpiritLevelValue(), GetMaxSpiritLevelValue());
}

void AMHPlayerCharacter::BroadcastSpiritLevelTimerChanged()
{
    OnSpiritLevelTimerChanged.Broadcast(GetSpiritLevelRemainingTime(), GetSpiritLevelDuration());
}

#pragma endregion



