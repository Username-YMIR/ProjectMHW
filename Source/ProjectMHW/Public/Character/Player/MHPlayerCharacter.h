#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Character/MHCharacterBase.h"
#include "Type/MHPlayerStructType.h"
#include "Type/MHItemStructType.h"
#include "Type/MHWeaponAnimStructType.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHPlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMHOnVitalChanged, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMHOnSpiritLevelChanged, int32, CurrentLevel, int32, MaxLevel);

UENUM(BlueprintType)
enum class EMHConsumableSelection : uint8
{
    None,
    Sharpen,
    Potion
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMHOnConsumableSelectionChanged, EMHConsumableSelection, NewSelection);

UENUM(BlueprintType)
enum class EMHLongSwordCounterWindowType : uint8
{
    None                    UMETA(DisplayName = "None"),
    Foresight               UMETA(DisplayName = "Foresight"),
    SpecialSheatheSlash     UMETA(DisplayName = "SpecialSheatheSlash"),
    SpecialSheatheSpirit    UMETA(DisplayName = "SpecialSheatheSpirit")
};

enum class EMHHitResultType : uint8;


class UMHHealthAttributeSet;
class UMHCombatAttributeSet;
class UMHResistanceAttributeSet;
class UMHPlayerAttributeSet;
class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
class UDataAsset_LSInputPatternSet;
class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class UGameplayEffect;
class UGameplayAbility;
class UDataTable;
class AMHWeaponInstance;
class AMHGreatSwordInstance;
class UMHGreatSwordActionComponent;
class UMHGA_Potion;
class UNiagaraComponent;
class UNiagaraSystem;
struct FInputActionValue;
struct FMHAttackDefinitionRow;
struct FMHAttackMetaRow;

UCLASS()
class PROJECTMHW_API AMHPlayerCharacter : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHPlayerCharacter();

protected:
    virtual void BeginPlay() override;

    void InitializeCapsuleSettings();
    
    
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

    void Input_Move(const FInputActionValue& InputActionValue);

    void Input_Look(const FInputActionValue& InputActionValue);

    void Input_SprintStarted(const FInputActionValue& InputActionValue);

    void Input_SprintCompleted(const FInputActionValue& InputActionValue);

    void Input_Dodge(const FInputActionValue& InputActionValue);

    void Input_AttackPrimary(const FInputActionValue& InputActionValue);

    void Input_AttackSecondary(const FInputActionValue& InputActionValue);

    // 현재 장착 무기에 맞는 무기 특수 입력을 처리한다.
    void Input_WeaponSpecial(const FInputActionValue& InputActionValue);

    void Input_AttackPrimaryCompleted(const FInputActionValue& InputActionValue);
    void Input_AttackSecondaryCompleted(const FInputActionValue& InputActionValue);
    void Input_WeaponSpecialCompleted(const FInputActionValue& InputActionValue);
    void Input_DodgeCompleted(const FInputActionValue& InputActionValue);

    // 현재 장착 무기에 맞는 동시 공격 입력을 처리한다.
    void Input_AttackSimultaneous(const FInputActionValue& InputActionValue);

    void Input_AimHoldStarted(const FInputActionValue& InputActionValue);

    void Input_AimHoldCompleted(const FInputActionValue& InputActionValue);

    void Input_DebugIncomingDamageKeyPressed();

protected:
    virtual bool ApplyIncomingDamageSpec(
        const FGameplayEffectSpecHandle& DamageSpecHandle
    ) override;

    bool ApplyIncomingPlayerDamageSpec(
        const FGameplayEffectSpec& IncomingSpec
    );

public:
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void UsePrimaryAction();

    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage);

    UFUNCTION(BlueprintCallable, Category = "Debug|Damage")
    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage, const FGameplayTag& InAttackTag);

#pragma region WeaponAndLongSwordAPI
    void HandleComboMontageStateTransition(bool bInterrupted);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Notify_AttachWeaponToHand();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Notify_AttachWeaponToBack();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Notify_AttachWeaponToSocket(FName InSocketName);

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginComboChainWindow();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndComboChainWindow();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginEarlyTransitionWindow();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndEarlyTransitionWindow();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginDirectionalTurnWindow(float InMaxYawDeltaDegrees, float InRotationInterpSpeed);

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndDirectionalTurnWindow();
    // ===== 대검 몽타주 노티파이 =====
    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_GreatSwordChargeLevelReached(int32 InChargeLevel);

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_GreatSwordChargeAutoRelease();

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_BeginGreatSwordAttackRollWindow();

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_EndGreatSwordAttackRollWindow();

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_BeginGreatSwordEarlyTransitionWindow();

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_EndGreatSwordEarlyTransitionWindow();

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_BeginGreatSwordChargeFollowUpWindow(FGameplayTag InSourceMoveTag);

    UFUNCTION(BlueprintCallable, Category = "GreatSword")
    void Notify_EndGreatSwordChargeFollowUpWindow();
    // ===== 대검 몽타주 노티파이 끝 =====

    // 현재 장착 중인 무기 인스턴스를 반환한다.
    AMHWeaponInstance* GetEquippedWeapon() const { return EquippedWeapon; }

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_LongSwordForesightCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearLongSwordForesightCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    bool HasLongSwordForesightCounterSuccess() const { return bLongSwordForesightCounterSuccess; }

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_LongSwordSpecialSheatheSlashCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearLongSwordSpecialSheatheSlashCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    bool HasLongSwordSpecialSheatheSlashCounterSuccess() const { return bLongSwordSpecialSheatheSlashCounterSuccess; }

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_LongSwordSpecialSheatheSpiritCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearLongSwordSpecialSheatheSpiritCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    bool HasLongSwordSpecialSheatheSpiritCounterSuccess() const { return bLongSwordSpecialSheatheSpiritCounterSuccess; }


    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearLongSwordCounterSuccessFlagsForMoveExit(FGameplayTag InMoveTag);

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearAllLongSwordCounterSuccessFlags();

    void Notify_LongSwordMoveStarted(const FGameplayTag& InMoveTag);

    void Notify_LongSwordAttackHitConfirmed(const FGameplayTag& InMoveTag);

    void Notify_LongSwordCounterCommitSuccess(EMHLongSwordCounterWindowType InCounterWindowType);

    float ResolveLongSwordDamageMultiplier(const FGameplayTag& InMoveTag) const;
    bool CanStartLongSwordMove(const FGameplayTag& InMoveTag) const;

    bool DoesLongSwordMoveRequireAttackMeta(const FGameplayTag& InMoveTag) const;

    bool DoesLongSwordMoveBuildDamageSpec(const FGameplayTag& InMoveTag) const;

    void PlayLongSwordHitCameraShake(const FGameplayTag& InMoveTag) const;
#pragma endregion

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentHealthValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxHealthValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetHealthRatio() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentStaminaValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxStaminaValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetStaminaRatio() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentSpiritGaugeValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxSpiritGaugeValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetSpiritGaugeRatio() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    int32 GetCurrentSpiritLevelValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    int32 GetMaxSpiritLevelValue() const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    void GetPlayerVitalStatus(float& OutCurrentHealth, float& OutMaxHealth, float& OutCurrentStamina, float& OutMaxStamina) const;

    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    void GetLongSwordSpiritStatus(float& OutCurrentSpiritGauge, float& OutMaxSpiritGauge, int32& OutCurrentSpiritLevel, int32& OutMaxSpiritLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginLongSwordCounterWindow(EMHLongSwordCounterWindowType InCounterWindowType);

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndLongSwordCounterWindow(EMHLongSwordCounterWindowType InCounterWindowType);

    virtual FMHHitAcknowledge ReceiveDamageSpec_Implementation(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FGameplayEffectSpecHandle& DamageSpecHandle,
        const FHitResult& HitResult
    ) override;

    virtual bool CanReceiveDamage(
        AActor* SourceActor,
        FGameplayTag AttackTag,
        const FGameplayEffectSpecHandle& DamageSpecHandle,
        const FHitResult& HitResult
    ) const override;

    virtual void HandleDamageAccepted(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    ) override;

    virtual bool IsDead() const override;
    virtual void HandleDeath() override;

    void BeginPotionUse(UMHGA_Potion* InPotionAbility);
    void EndPotionUse(UMHGA_Potion* InPotionAbility);

    bool TryStartAutoSheatheAfterLongSwordMove(const FGameplayTag& CompletedMoveTag);

    /**
     */
    UAnimMontage* ResolveLongSwordMoveMontageOverride(const FGameplayTag& InMoveTag, UAnimMontage* InDefaultMontage) const;

protected:
#pragma region Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;
#pragma endregion

    // ===== Movement =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FMHPlayerMovementConfig MovementConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EMHPlayerLocomotionState LocomotionState = EMHPlayerLocomotionState::Idle;
    // ===== End Movement =====

    // ===== Stamina =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
    FMHPlayerStaminaConfig StaminaConfig;

    // ===== End Stamina =====

    // ===== Inputs =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_LSInputPatternSet> LongSwordInputPatternSet;

    // ===== End Inputs =====

#pragma region WeaponState
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FMHWeaponSocketConfig WeaponSocketConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    EMHWeaponSheathState WeaponSheathState = EMHWeaponSheathState::Sheathed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    EMHWeaponType CurrentWeaponType = EMHWeaponType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AMHWeaponInstance> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMHWeaponInstance> EquippedWeapon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UAnimMontage> SheathedRollMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitReact", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> DamageHitReactMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Death", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> DeathMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Burn", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UNiagaraSystem> BurningLoopNiagara;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Burn", meta = (AllowPrivateAccess = "true"))
    float BurnDamagePerTick = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Burn", meta = (AllowPrivateAccess = "true"))
    float BurnTickInterval = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Burn", meta = (AllowPrivateAccess = "true"))
    int32 BurnRequiredRollCount = 3;
#pragma endregion
    
#pragma region WeaponStatGas
    // === Weapon Stat (GAS) ===
public:
    EMHHitResultType HandleWeaponAttackHit(AActor* Target, AMHWeaponInstance* Weapon);
protected:

    UPROPERTY(Transient)
    FActiveGameplayEffectHandle EquippedWeaponStatEffectHandle;

    UPROPERTY(EditDefaultsOnly, Category="Weapon|Stat")
    TSubclassOf<UGameplayEffect> WeaponStatEffectClass;
    
    UPROPERTY(Transient)
    EMHSharpnessColor CurrentSharpnessColor;

    UPROPERTY(Transient)
    float CurrentSharpnessValue = 0.0f;

    UPROPERTY(Transient)
    FGameplayTag CurrentWeaponElementTag;


    // === Weapon Stat Functions ===
protected:

    void ApplyEquippedWeaponStatEffect();

    void RemoveEquippedWeaponStatEffect();

    void RefreshEquippedWeaponStatEffect();
    void SetSharpnessAttributeValues(float InCurrentSharpness, float InMaxSharpness);
    void UpdateSharpnessModifierFromCurrentColor();
    
    void ConsumeSharpness(float Amount);



#pragma endregion

#pragma region LongSwordRuntimeState
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> LongSwordAttackMetaTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float MaxSpiritGauge = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|LongSword", meta = (AllowPrivateAccess = "true"))
    float CurrentSpiritGauge = 50.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0", ClampMax = "3", AllowPrivateAccess = "true"))
    int32 CurrentSpiritLevel = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float SpiritLevelMultiplierLv0 = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float SpiritLevelMultiplierLv1 = 1.05f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float SpiritLevelMultiplierLv2 = 1.10f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LongSword", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float SpiritLevelMultiplierLv3 = 1.20f;
#pragma endregion

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UGameplayEffect> PlayerIncomingDamageEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sharpness", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> SharpnessBounceMontage = nullptr;

#pragma region Debug
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug|Damage", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UGameplayEffect> DebugDamageEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug|Damage", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> AttackDefinitionTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage", meta = (AllowPrivateAccess = "true"))
    float DebugIncomingPhysicalDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage", meta = (AllowPrivateAccess = "true"))
    FGameplayTag DebugIncomingAttackTag;
#pragma endregion

#pragma region GAS
    // GAS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHHealthAttributeSet> HealthAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHCombatAttributeSet> CombatAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHResistanceAttributeSet> ResistanceAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHPlayerAttributeSet> PlayerAttributeSet;
#pragma endregion 

    
#pragma region Visual
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftClassPtr<UAnimInstance> DefaultAnimClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FVector DefaultMeshRelativeLocation = FVector(0.f, 0.f, -7.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FRotator DefaultMeshRelativeRotation = FRotator(0.f, -90.f, 0.f);
#pragma endregion

private:
    bool bDamageHitReactMontagePlaying = false;
    bool bActionInputLocked = false;
    bool bActionInputLockedByDamageHitReact = false;
    bool bActionInputLockedBySharpnessBounce = false;
    bool bPotionInUse = false;
    bool bDeathStateActive = false;
    bool bBurningActive = false;

    int32 BurnRollCount = 0;

    UAnimMontage* ActiveDamageHitReactMontage = nullptr;
    UAnimMontage* ActiveSharpnessBounceMontage = nullptr;
    UAnimMontage* ActiveDeathMontage = nullptr;
    TWeakObjectPtr<UMHGA_Potion> ActivePotionAbility;

    UPROPERTY(Transient)
    TObjectPtr<UNiagaraComponent> BurningLoopNiagaraComponent;

    FTimerHandle BurnDamageTimerHandle;

    bool IsDamageHitReactActive() const;
    bool ResolveDamageHitReactFacingYaw(AActor* SourceActor, const FHitResult& HitResult, FRotator& OutFacingRotation) const;
    bool TryPlayDamageHitReactMontage(AActor* SourceActor, const FHitResult& HitResult);
    bool TryPlayDeathMontage();
    void HandleDamageHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void HandleSharpnessBounceMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    void RefreshActionInputLockState();
    void SetDamageHitReactInputLock(bool bEnable);
    void SetSharpnessBounceInputLock(bool bEnable);
    void CancelActivePotionUseOnDamageTaken();
    void TryIgniteBurning();
    void HandleBurnDamageTick();
    void HandleBurnRollSucceeded();
    void ClearBurningState();

    bool bSprintHeld = false;

    bool bIsSprinting = false;

    bool bAimHeld = false;

    bool bAttackPrimaryHeld = false;
    bool bAttackSecondaryHeld = false;
    bool bWeaponSpecialHeld = false;
    bool bDodgeHeld = false;

    bool bLongSwordForesightCounterSuccess = false;
    bool bLongSwordSpecialSheatheSlashCounterSuccess = false;
    bool bLongSwordSpecialSheatheSpiritCounterSuccess = false;
    bool bLongSwordSpiritThrustHelmbreakerReady = false;
    bool bLongSwordForesightFreeSpiritRoundslashReady = false;
    bool bIgnoreDamageUntilCurrentActionEnd = false;

    EMHLongSwordCounterWindowType ActiveLongSwordCounterWindowType = EMHLongSwordCounterWindowType::None;
    FGameplayTag DamageIgnoreUntilCurrentMoveTag;

    FVector2D CachedMoveInput2D = FVector2D::ZeroVector;

    FVector2D LastNonZeroMoveInput2D = FVector2D::ZeroVector;

    EMHDodgeContext LastResolvedDodgeContext = EMHDodgeContext::Sheathed;

    EMHDirectionalVariant LastResolvedDodgeVariant = EMHDirectionalVariant::None;

#pragma region DirectionalTurnWindow
    bool bDirectionalTurnWindowActive = false;

    float DirectionalTurnWindowMaxYawDeltaDegrees = 0.0f;

    float DirectionalTurnWindowRotationInterpSpeed = 0.0f;

    float DirectionalTurnWindowBaseYaw = 0.0f;
#pragma endregion

    // 납도 상태에서 시작한 대검 입력이 발도 진입으로 확정되면 설정한다.
    bool bPendingUnsheatheFromComboEntry = false;

    // 몽타주로 구동되는 대검 유틸리티 기술의 현재 몽타주다.
    UAnimMontage* ActiveGreatSwordUtilityMontage = nullptr;

    // 현재 재생 중인 대검 유틸리티 몽타주와 짝을 이루는 이동 태그다.
    FGameplayTag ActiveGreatSwordUtilityMoveTag;

    void ApplyPlayerVisuals();

    void ApplyMovementProfile(EMHPlayerMoveProfile InProfile);

    void UpdateLocomotionState();

    void UpdateStamina(float DeltaSeconds);

    void EvaluateSprintState();

    bool CanStartSprint() const;

    void SyncStaminaAttributesFromConfig();

    void SetCurrentStaminaAttributeValue(float InNewValue);

    void SetMaxStaminaAttributeValue(float InNewValue);

    void ApplyDefaultPlayerAttributes();

    void SetCurrentHealthAttributeValue(float InNewValue);

    void SetMaxHealthAttributeValue(float InNewValue);

    void SetDefenseAttributeValue(float InNewValue);

    // ===== Terrain Hooks =====
    // ===== End Terrain Hooks =====

#pragma region WeaponRuntimeFunctions
    void SpawnAndEquipDefaultWeapon();
    
    //============================
    bool EquipWeaponInstance(AMHWeaponInstance* InWeapon, bool bDestroyPreviousWeapon = true);
    void UnequipCurrentWeapon(bool bDestroyWeapon = true);
    //============================

    void AttachWeaponActorToBack();

    USkeletalMeshComponent* GetWeaponBladeMesh() const;

    const FMHWeaponAnimConfig* GetEquippedWeaponAnimConfig() const;

    void AttachWeaponToBack();

    void AttachWeaponToHand();

    void AttachWeaponToSocket(const FName& InSocketName);

    FGameplayTag ResolveLongSwordPatternForPrimaryInput() const;

    FGameplayTag ResolveLongSwordPatternForSecondaryInput() const;

    FGameplayTag ResolveLongSwordPatternForWeaponSpecialInput() const;

    FGameplayTag ResolveLongSwordPatternForDodgeInput() const;

    FGameplayTag ResolveLongSwordPatternForCompositeInput() const;

    FGameplayTag ResolveLongSwordPatternForAttackSimultaneousInput() const;
#pragma endregion


#pragma region GreatSwordRuntimeFunctions
protected:
    bool IsGreatSwordEquipped() const;

    // 해석이 끝난 대검 입력 패턴을 공용 경로로 처리한다.
    bool TryHandleGreatSwordPatternInput(const FGameplayTag& InPatternTag, bool bInPromoteSheathedToUnsheathing);

    bool TryHandleGreatSwordPrimaryInput();
    bool TryHandleGreatSwordPrimaryRelease();
    bool TryHandleGreatSwordSecondaryInput();
    bool TryHandleGreatSwordWeaponSpecialInput();
    bool TryHandleGreatSwordWeaponSpecialRelease();
    bool TryHandleGreatSwordSimultaneousInput();

    // 대기 중인 대검 기술을 실행하고 실패 시 저장해 둔 상태를 복구한다.
    bool TryExecuteGreatSwordPendingMove();

    bool TryActivateGreatSwordPrimaryAbility();
    bool TryPlayGreatSwordUtilityMontage(const FGameplayTag& InMoveTag);
    void HandleGreatSwordUtilityMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    bool IsGreatSwordAttackChainDodgeContext() const;
#pragma endregion

#pragma region LongSwordRuntimeFunctions
    bool IsLongSwordEquipped() const;
    bool HasMovementInputForCombat() const;
    bool IsStandingStillForCombat() const;
    bool IsInLongSwordSpecialSheatheState() const;
    bool CanResolveLongSwordFollowupDuringUnsheathing() const;

    FGameplayTag GetCurrentLongSwordMoveTag() const;
    void ClearExpiredLongSwordDamageIgnoreState();
    bool CanTriggerLongSwordForesightCounter() const;
    bool CanTriggerLongSwordSpecialSheatheSlashCounter() const;
    bool CanTriggerLongSwordSpecialSheatheSpiritCounter() const;
    bool IsAttackAllowedForForesightCounter(const FGameplayTag& InAttackTag) const;
    bool IsAttackAllowedForSpecialSheatheCounter(const FGameplayTag& InAttackTag) const;
    const FMHAttackDefinitionRow* FindAttackDefinitionRow(const FGameplayTag& InAttackTag) const;
    bool FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const;

    void ApplyLongSwordMoveStartCost(const FGameplayTag& InMoveTag);

    void ApplyLongSwordMoveHitReward(const FGameplayTag& InMoveTag);

    void ApplyLongSwordCounterSuccessReward(const FGameplayTag& InMoveTag, EMHLongSwordCounterWindowType InCounterWindowType);

    float GetCurrentSpiritDamageMultiplier() const;
    
    
    void SetSpiritGaugeValues(const float InSpiritValue, const float InMaxSpiritValue);
    void SetCurrentSpiritGauge(const float InSpiritValue);
    void SetMaxSpiritGuage(const float InMaxSpiritValue);
    
    void AddSpiritGauge(float InAmount);
    void ConsumeSpiritGauge(float InAmount);
    void IncreaseSpiritLevel(int32 InAmount = 1);
    void DecreaseSpiritLevel(int32 InAmount = 1);
    FMHHitAcknowledge BuildLongSwordInvulnerableHitAcknowledge() const;

    bool IsLongSwordStartAttackContext() const;

    bool IsLongSwordFollowupContext() const;

    bool ShouldUseDirectionalLateralFadeSlash() const;
    bool ShouldUseLateralFadeSlashPattern() const;
    
public:
    float GetSpiritLevelRemainingTime() const;
    float GetSpiritLevelDuration() const;
#pragma endregion


    bool bRollMontagePlaying = false;
public:
    FGameplayTag GetCurrentWeaponTypeGameplayTag() const;
    FGameplayTag GetCurrentWeaponSheathGameplayTag() const;
    FGameplayTag GetCurrentCombatStateGameplayTag() const;

protected:

    FVector2D GetPreferredMoveInput2D() const;

    EMHDirectionalVariant ResolveDirectionalVariantFromInput(bool bPreserveActorFacing) const;

    bool TryRotateActorTowardsMoveInput();

    void UpdateDirectionalTurnWindow(float DeltaSeconds);

    bool TryApplyDirectionalTurnWindowRotation(float DeltaSeconds);

    bool IsLongSwordAttackChainDodgeContext() const;

    UAnimMontage* ResolveSheathedRollMontage() const;

    UAnimMontage* ResolveUnsheathedRollMontage() const;

    bool TryPlayRollMontage(UAnimMontage* InMontage);

    bool IsLongSwordDrawEntryPattern(const FGameplayTag& InPatternTag) const;
    bool TryResolveAndHandleLongSwordPattern(const FGameplayTag& PreferredPatternTag = FGameplayTag());
    bool TryHandleWeaponComboInput(const FGameplayTag& InPatternTag);
    bool TryRequestLongSwordEarlyTransition();

    bool CanStartSheathe() const;

    void StartSheathe();

    void HandleSheatheMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void HandleRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma region WeaponAnimationLayerFunctions
    void RefreshWeaponAnimationLayerState();

    void LinkCurrentWeaponAnimLayer();

    void UnlinkCurrentWeaponAnimLayer();

    TSoftClassPtr<UAnimInstance> GetCurrentWeaponLinkedAnimLayerClass() const;
#pragma endregion

    UPROPERTY(Transient)
    bool bWeaponAnimLayerLinked = false;
    
    
    
#pragma region AttributeDelegate
public:
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnStaminaChanged;
    
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnSpiritGaugeChanged;

    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnSpiritLevelChanged OnSpiritLevelChanged;

    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnSpiritLevelTimerChanged;
    
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnSharpnessChanged;

protected:
    virtual void InitializeAbilitySystem() override;

    
    void BindAttributeDelegates();
    void BroadcastInitialAttributeSnapshot();

    void HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleMaxHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleMaxStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData);
    
    void HandleShapnessAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleMaxShapnessAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleSpiritAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void HandleMaxSpiritAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void RefreshSpiritLevelDecayState(bool bResetTimer);
    void HandleSpiritLevelDecayTick();
    void BroadcastSpiritLevelChanged();
    void BroadcastSpiritLevelTimerChanged();

    bool bAttributeDelegatesBound = false;
#pragma endregion

public:
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnHealableHealthChanged;
    UPROPERTY(BlueprintAssignable, Category="UI|Items")
    FMHOnConsumableSelectionChanged OnConsumableSelectionChanged;

    bool IsItemUseHeld() const { return bItemUseHeld; }
    UFUNCTION(BlueprintPure, Category="UI|Items")
    EMHConsumableSelection GetSelectedConsumable() const { return SelectedConsumable; }
    float GetCurrentHealableHealthValue() const;
    float GetCurrentSharpnessValue() const;
    float GetMaxSharpnessValue() const;
    float GetCurrentSharpnessSegmentValue() const;
    float GetCurrentSharpnessSegmentMax() const;
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    EMHSharpnessColor GetCurrentSharpnessColor() const { return CurrentSharpnessColor; }
    void RefreshSharpnessState();
    void HandleSharpnessBounce();
    bool CanStartSharpenItemUse() const;

protected:
    void SetSelectedConsumable(EMHConsumableSelection InSelection);
    void Input_ItemSelectSharpen(const FInputActionValue& InputActionValue);
    void Input_ItemSelectPotion(const FInputActionValue& InputActionValue);
    void Input_ItemUseStarted(const FInputActionValue& InputActionValue);
    void Input_ItemUseCompleted(const FInputActionValue& InputActionValue);
    void HandleHealableHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
    
    void TryUseSelectedItem();
   
protected:
    void NormalizeSharpnessStateFromAttribute();
    EMHSharpnessColor GetLowerSharpnessColor(EMHSharpnessColor InColor) const;
    EMHSharpnessColor GetHigherSharpnessColor(EMHSharpnessColor InColor) const;
    bool CanUpgradeSharpnessColor() const;
    bool CancelSharpenAbilityIfActive();
    bool EndActiveEquippedWeaponAttackAbility(bool bWasCancelled);
    bool IsEquippedWeaponPrimaryAbilityActive() const;

private:
    bool bSyncingSharpnessState = false;
    FTimerHandle SpiritLevelDecayTimerHandle;
    float SpiritLevelRemainingTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|LongSword", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
    float SpiritLevelDuration = 60.0f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|LongSword", meta = (ClampMin = "0.01", AllowPrivateAccess = "true"))
    float SpiritLevelDecayTickInterval = 0.1f;
    

private:
    EMHConsumableSelection SelectedConsumable = EMHConsumableSelection::Sharpen;
    bool bItemUseHeld = false;

    UPROPERTY(EditDefaultsOnly, Category="Ability|Items")
    TSubclassOf<UGameplayAbility> SharpenAbilityClass;

    UPROPERTY(EditDefaultsOnly, Category="Ability|Items")
    TSubclassOf<UGameplayAbility> PotionAbilityClass;
        
};





