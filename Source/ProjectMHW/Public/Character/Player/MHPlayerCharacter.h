#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Character/MHCharacterBase.h"
#include "Type/MHPlayerStructType.h"
#include "Type/MHItemStructType.h"
#include "Type/MHWeaponAnimStructType.h"
#include "Weapons/Common/MHWeaponComboTypes.h" //손승우 추가
#include "MHPlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerCharacter, Log, All);

// 어트리뷰트셋 값 변경 시 사용하는 델리게이트 _이건주
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMHOnVitalChanged, float, CurrentValue, float, MaxValue);

UENUM(BlueprintType)
enum class EMHLongSwordCounterWindowType : uint8
{
    None                    UMETA(DisplayName = "None"),
    Foresight               UMETA(DisplayName = "Foresight"),
    SpecialSheatheSlash     UMETA(DisplayName = "SpecialSheatheSlash"),
    SpecialSheatheSpirit    UMETA(DisplayName = "SpecialSheatheSpirit")
};

// 태도 기술별 자원 반영 시점을 구분하기 위한 내부 enum
UENUM()
enum class EMHLongSwordResourceCommitType : uint8
{
    None,
    FirstValidHit,
    FinisherHit,
    CounterSuccess
};

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
class UDataTable;
class AMHWeaponInstance;
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
    // 종료 처리
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 무브먼트 업데이트 처리
    UFUNCTION()
    void HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

    // 이동 입력
    void Input_Move(const FInputActionValue& InputActionValue);

    // 시점 입력
    void Input_Look(const FInputActionValue& InputActionValue);

    // 스프린트 시작
    void Input_SprintStarted(const FInputActionValue& InputActionValue);

    // 스프린트 종료
    void Input_SprintCompleted(const FInputActionValue& InputActionValue);

    // 회피 입력
    void Input_Dodge(const FInputActionValue& InputActionValue);

    // 기본 공격 입력
    void Input_AttackPrimary(const FInputActionValue& InputActionValue);

    // 보조 공격 입력
    void Input_AttackSecondary(const FInputActionValue& InputActionValue);

    // 무기 특수 입력
    void Input_WeaponSpecial(const FInputActionValue& InputActionValue); //손승우 추가

    void Input_AttackPrimaryCompleted(const FInputActionValue& InputActionValue);
    void Input_AttackSecondaryCompleted(const FInputActionValue& InputActionValue);
    void Input_WeaponSpecialCompleted(const FInputActionValue& InputActionValue);
    void Input_DodgeCompleted(const FInputActionValue& InputActionValue);

    // 동시 공격 입력
    void Input_AttackSimultaneous(const FInputActionValue& InputActionValue); //손승우 추가

    // 조준/홀드 시작
    void Input_AimHoldStarted(const FInputActionValue& InputActionValue); //손승우 추가

    // 조준/홀드 종료
    void Input_AimHoldCompleted(const FInputActionValue& InputActionValue); //손승우 추가

    // 키보드 4번 입력으로 플레이어에게 디버그 피격을 발생시켜 카운터 동작을 검증한다.
    void Input_DebugIncomingDamageKeyPressed();

protected:
    /** 플레이어는 공통 DamageSpec을 직접 적용하지 않고, 플레이어 전용 Damage GE로 재구성해 적용한다. */
    virtual bool ApplyIncomingDamageSpec(
        const FGameplayEffectSpecHandle& DamageSpecHandle
    ) override;

    /** 전달받은 DamageSpec을 플레이어 전용 Damage GE Spec으로 변환해 적용한다. */
    bool ApplyIncomingPlayerDamageSpec(
        const FGameplayEffectSpec& IncomingSpec
    );

public:
    // 기본 공격
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void UsePrimaryAction();

    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage);

    UFUNCTION(BlueprintCallable, Category = "Debug|Damage")
    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage, const FGameplayTag& InAttackTag);

#pragma region WeaponAndLongSwordAPI
    // 콤보 몽타주 종료 시 납도/발도 상태 확정
    void HandleComboMontageStateTransition(bool bInterrupted); //손승우 추가

    // 노티파이: 무기 손 소켓으로 이동
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Notify_AttachWeaponToHand();

    // 노티파이: 무기 등 소켓으로 이동
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Notify_AttachWeaponToBack();

    // 노티파이: 콤보 입력 윈도우 시작
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginComboChainWindow();

    // 노티파이: 콤보 입력 윈도우 종료
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndComboChainWindow();

    // 노티파이: 조기 전환 윈도우 시작
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginEarlyTransitionWindow();

    // 노티파이: 조기 전환 윈도우 종료
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndEarlyTransitionWindow();

    // 노티파이: 입력 방향 기준 회전 조정 윈도우 시작
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_BeginDirectionalTurnWindow(float InMaxYawDeltaDegrees, float InRotationInterpSpeed);

    // 노티파이: 입력 방향 기준 회전 조정 윈도우 종료
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_EndDirectionalTurnWindow();
    
    // 현재 장착 중인 무기 인스턴스 반환
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

    // 태도 공격이 실제 유효 타격으로 확정된 순간 자원 반영을 처리한다.
    void Notify_LongSwordAttackHitConfirmed(const FGameplayTag& InMoveTag);

    // 태도 카운터 성공 시점에 자원 반영이 필요한 기술을 처리한다.
    void Notify_LongSwordCounterCommitSuccess(EMHLongSwordCounterWindowType InCounterWindowType);

    // 현재 기인 레벨과 공격 메타를 반영한 최종 배율을 계산한다.
    float ResolveLongSwordDamageMultiplier(const FGameplayTag& InMoveTag) const;

    // 기술 시작 전에 현재 기인 게이지로 진입 가능한지 확인한다.
    bool CanStartLongSwordMove(const FGameplayTag& InMoveTag) const;

#pragma endregion

    // UI/HUD에서 현재 체력 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentHealthValue() const;

    // UI/HUD에서 최대 체력 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxHealthValue() const;

    // UI/HUD에서 체력 비율을 바로 사용할 수 있게 반환한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetHealthRatio() const;

    // UI/HUD에서 현재 스태미너 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentStaminaValue() const;

    // UI/HUD에서 최대 스태미너 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxStaminaValue() const;

    // UI/HUD에서 스태미너 비율을 바로 사용할 수 있게 반환한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetStaminaRatio() const;

    // UI/HUD에서 현재 기인 게이지 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetCurrentSpiritGaugeValue() const;

    // UI/HUD에서 최대 기인 게이지 값을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetMaxSpiritGaugeValue() const;

    // UI/HUD에서 기인 게이지 비율을 바로 사용할 수 있게 반환한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    float GetSpiritGaugeRatio() const;

    // UI/HUD에서 현재 기인 레벨을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    int32 GetCurrentSpiritLevelValue() const;

    // UI/HUD에서 최대 기인 레벨을 조회할 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    int32 GetMaxSpiritLevelValue() const;

    // UI/HUD에서 한 번에 체력/스태미너 값을 가져올 때 사용한다.
    UFUNCTION(BlueprintPure, Category = "UI|PlayerStatus")
    void GetPlayerVitalStatus(float& OutCurrentHealth, float& OutMaxHealth, float& OutCurrentStamina, float& OutMaxStamina) const;

    // UI/HUD에서 한 번에 기인 게이지/레벨 값을 가져올 때 사용한다.
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

    bool TryStartAutoSheatheAfterLongSwordMove(const FGameplayTag& CompletedMoveTag);

    /**
     * 현재 MoveTag와 입력 방향을 기준으로 Variant 몽타주를 선택한다.
     * - 기본적으로 ComboGraph에 지정된 몽타주를 우선 사용한다.
     * - Fade / LateralFade처럼 방향 Variant가 필요한 경우에만 무기 AnimConfig의 전용 몽타주로 교체한다.
     */
    UAnimMontage* ResolveLongSwordMoveMontageOverride(const FGameplayTag& InMoveTag, UAnimMontage* InDefaultMontage) const;

protected:
// 동일 카테고리 오브젝트가 3개 이상이면 region으로 구분
#pragma region Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom; // 스프링암

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera; // 카메라
#pragma endregion

    // ===== Movement =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FMHPlayerMovementConfig MovementConfig; // 이동 설정

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EMHPlayerLocomotionState LocomotionState = EMHPlayerLocomotionState::Idle; // 로코모션 상태
    // ===== End Movement =====

    // ===== Stamina =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
    FMHPlayerStaminaConfig StaminaConfig; // 스태미나 설정

    // ===== End Stamina =====

    // ===== Inputs =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset; // 입력 설정

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_LSInputPatternSet> LongSwordInputPatternSet; // 롱소드 입력 패턴 DA
    // ===== End Inputs =====

#pragma region WeaponState
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTag; // 무기 타입 태그

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FMHWeaponSocketConfig WeaponSocketConfig; // 무기 소켓 설정

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    EMHWeaponSheathState WeaponSheathState = EMHWeaponSheathState::Sheathed; // 무기 납도/발도 상태

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    EMHWeaponType CurrentWeaponType = EMHWeaponType::None; // 무기 타입

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AMHWeaponInstance> DefaultWeaponClass; // 기본 무기 BP

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AMHWeaponInstance> EquippedWeapon; // 장착 무기

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UAnimMontage> SheathedRollMontage; // 납도 구르기 몽타주(루트모션)
#pragma endregion
    
#pragma region Weapon Stat (GAS)_이건주
    // === Weapon Stat (GAS) ===
public:
    void HandleWeaponAttackHit(AActor* Target, AMHWeaponInstance* Weapon);
protected:

    // 현재 장착 무기 스탯 GE 핸들
    UPROPERTY(Transient)
    FActiveGameplayEffectHandle EquippedWeaponStatEffectHandle;

    // 공통 무기 스탯 GameplayEffect 클래스
    UPROPERTY(EditDefaultsOnly, Category="Weapon|Stat")
    TSubclassOf<UGameplayEffect> WeaponStatEffectClass;
    
    // 현재 예리도 상태
    UPROPERTY(Transient)
    EMHSharpnessColor CurrentSharpnessColor;

    UPROPERTY(Transient)
    float CurrentSharpnessValue;
    
    UPROPERTY(Transient)
    float CurrentSharpnessLength;
    
    float GetCurrentSharpnessValue() const { return CurrentSharpnessValue; }
    float GetCurrentSharpnessLength() const { return CurrentSharpnessLength; }    
    
    UPROPERTY(Transient)
    FGameplayTag CurrentWeaponElementTag;


    // === Weapon Stat Functions ===
protected:

    // 현재 EquippedWeapon 기준으로 GE 적용
    void ApplyEquippedWeaponStatEffect();

    // 기존 GE 제거
    void RemoveEquippedWeaponStatEffect();

    // 교체/초기 장착 시 호출
    void RefreshEquippedWeaponStatEffect();
    // 예리도 값
    float GetMaxSharpnessValueFromColor(const FMHSharpnessData& Data, EMHSharpnessColor Color) const;
    
    // 예리도 소모
    void ConsumeSharpness(float Amount);
    // 예리도 단계 하락
    bool DowngradeSharpnessColor();
    

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
    // 플레이어 어트리뷰트 셋 _이건주
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHHealthAttributeSet> HealthAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHCombatAttributeSet> CombatAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHResistanceAttributeSet> ResistanceAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
    TObjectPtr<UMHPlayerAttributeSet> PlayerAttributeSet;
#pragma endregion 

    
// 동일 카테고리 오브젝트가 3개 이상이면 region으로 구분
#pragma region Visual
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh; // 기본 스켈레톤 메쉬

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftClassPtr<UAnimInstance> DefaultAnimClass; // 기본 AnimBP

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FVector DefaultMeshRelativeLocation = FVector(0.f, 0.f, -7.f); // 메쉬 위치 보정

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FRotator DefaultMeshRelativeRotation = FRotator(0.f, -90.f, 0.f); // 메쉬 회전 보정
#pragma endregion

private:
    // 스프린트 입력 유지 여부
    bool bSprintHeld = false;

    // 현재 스프린트 상태 여부
    bool bIsSprinting = false;

    // 조준 홀드 상태
    bool bAimHeld = false; //손승우 추가

    bool bAttackPrimaryHeld = false;
    bool bAttackSecondaryHeld = false;
    bool bWeaponSpecialHeld = false;
    bool bDodgeHeld = false;

    bool bLongSwordForesightCounterSuccess = false;
    bool bLongSwordSpecialSheatheSlashCounterSuccess = false;
    bool bLongSwordSpecialSheatheSpiritCounterSuccess = false;
    bool bIgnoreDamageUntilCurrentActionEnd = false;

    EMHLongSwordCounterWindowType ActiveLongSwordCounterWindowType = EMHLongSwordCounterWindowType::None;
    FGameplayTag DamageIgnoreUntilCurrentMoveTag;

    /** 현재 프레임 이동 입력 값 */
    FVector2D CachedMoveInput2D = FVector2D::ZeroVector;

    /** 마지막으로 유효했던 이동 입력 값 */
    FVector2D LastNonZeroMoveInput2D = FVector2D::ZeroVector;

    /** 마지막으로 해석된 회피 컨텍스트 */
    EMHDodgeContext LastResolvedDodgeContext = EMHDodgeContext::Sheathed;

    /** 마지막으로 해석된 회피 방향 Variant */
    EMHDirectionalVariant LastResolvedDodgeVariant = EMHDirectionalVariant::None;

#pragma region DirectionalTurnWindow
    // 회전 조정 노티파이 스테이트 활성 여부
    bool bDirectionalTurnWindowActive = false;

    // 회전 조정 시작 시점 기준 최대 허용 yaw 편차
    float DirectionalTurnWindowMaxYawDeltaDegrees = 0.0f;

    // 회전 조정 속도(도/초). 0 이하면 목표 각도로 즉시 반영
    float DirectionalTurnWindowRotationInterpSpeed = 0.0f;

    // 회전 조정이 시작될 때의 기준 yaw
    float DirectionalTurnWindowBaseYaw = 0.0f;
#pragma endregion

    // 납도 상태 특수 진입 후 첫 몽타주 종료 대기 여부
    bool bPendingUnsheatheFromComboEntry = false; //손승우 추가

    // 비주얼 적용
    void ApplyPlayerVisuals();

    // 이동 설정 적용
    void ApplyMovementProfile(EMHPlayerMoveProfile InProfile);

    // 로코모션 상태 갱신
    void UpdateLocomotionState();

    // 스태미나 갱신
    void UpdateStamina(float DeltaSeconds);

    // 스프린트 유지/해제 평가
    void EvaluateSprintState();

    // 스프린트 시작 가능 여부
    bool CanStartSprint() const;

    // 스프린트 설정값을 ASC 스태미나 속성에 동기화한다.
    void SyncStaminaAttributesFromConfig();

    // 현재 스태미나 값을 ASC 스태미나 속성에 반영한다.
    void SetCurrentStaminaAttributeValue(float InNewValue);

    // 최대 스태미나 값을 ASC 스태미나 속성에 반영한다.
    void SetMaxStaminaAttributeValue(float InNewValue);

    // 플레이어 기본 체력, 방어력, 스태미나 값을 ASC 속성에 하드코딩 적용한다.
    void ApplyDefaultPlayerAttributes();

    // 현재 체력 값을 ASC 체력 속성에 반영한다.
    void SetCurrentHealthAttributeValue(float InNewValue);

    // 최대 체력 값을 ASC 체력 속성에 반영한다.
    void SetMaxHealthAttributeValue(float InNewValue);

    // 방어력 값을 ASC 전투 속성에 반영한다.
    void SetDefenseAttributeValue(float InNewValue);

    // 현재 예리도 단계에 대응하는 보정값을 ASC 전투 속성에 반영한다.
    void SetSharpnessModifierAttributeValue(float InNewValue);
    void SyncSharpnessModifierAttribute();
    float ResolveSharpnessModifierFromColor(EMHSharpnessColor InColor) const;

    // ===== Terrain Hooks =====
    // ===== End Terrain Hooks =====

#pragma region WeaponRuntimeFunctions
    // 무기 메쉬 적용
    void SpawnAndEquipDefaultWeapon();
    
    //============================
    // 무기 장착/해제 함수 _ 이건주
    bool EquipWeaponInstance(AMHWeaponInstance* InWeapon, bool bDestroyPreviousWeapon = true);
    void UnequipCurrentWeapon(bool bDestroyWeapon = true);
    //============================

    // 무기 액터를 등 소켓에 부착
    void AttachWeaponActorToBack();

    // 무기 칼 메쉬 반환
    USkeletalMeshComponent* GetWeaponBladeMesh() const;

    // 무기 애니 설정 반환
    const FMHWeaponAnimConfig* GetEquippedWeaponAnimConfig() const;

    // 무기 납도 상태로 부착
    void AttachWeaponToBack();

    // 무기 발도 상태로 부착
    void AttachWeaponToHand();

    // 좌클릭 입력을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForPrimaryInput() const;

    // 우클릭 입력을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForSecondaryInput() const;

    // Mouse4(실제 기인 축) 입력을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForWeaponSpecialInput() const;

    // 스페이스 입력을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForDodgeInput() const;

    // 동시 입력 조합을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForCompositeInput() const;

    // Mouse5(실제 베어내리기 계열 축) 단일 입력을 기준으로 태도 패턴을 해석한다.
    FGameplayTag ResolveLongSwordPatternForAttackSimultaneousInput() const;
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

    // 기술 태그를 기준으로 자원을 언제 확정할지 결정한다.
    EMHLongSwordResourceCommitType ResolveLongSwordResourceCommitType(const FGameplayTag& InMoveTag) const;

    // 확정 시점이 도달했을 때 게이지/기인 레벨 변화를 한곳에서 적용한다.
    void CommitLongSwordResourceDelta(const FGameplayTag& InMoveTag, EMHLongSwordResourceCommitType InCommitType);

    float GetCurrentSpiritDamageMultiplier() const;
    
    
    // 기인 게이지 브로드캐스트 포함
    void SetSpiritGaugeValues(const float InSpiritValue, const float InMaxSpiritValue);
    void SetCurrentSpiritGauge(const float InSpiritValue);
    void SetMaxSpiritGuage(const float InMaxSpiritValue);
    
    void AddSpiritGauge(float InAmount);
    void ConsumeSpiritGauge(float InAmount);
    void IncreaseSpiritLevel(int32 InAmount = 1);
    void DecreaseSpiritLevel(int32 InAmount = 1);
    FMHHitAcknowledge BuildLongSwordInvulnerableHitAcknowledge() const;

    // 발도 상태에서 첫 시작 공격을 선택하는 문맥인지 확인한다.
    bool IsLongSwordStartAttackContext() const;

    // 현재 콤보가 진행 중인 파생 문맥인지 확인한다.
    bool IsLongSwordFollowupContext() const;

    // 베어내리기 계열에서 좌우 이동베기 Variant를 사용할 수 있는지 확인한다.
    bool ShouldUseDirectionalLateralFadeSlash() const;
    bool ShouldUseLateralFadeSlashPattern() const;

#pragma endregion


    //롤 입력 차단을 위한
    bool bRollMontagePlaying = false;
public:
    FGameplayTag GetCurrentWeaponTypeGameplayTag() const;
    FGameplayTag GetCurrentWeaponSheathGameplayTag() const;
    FGameplayTag GetCurrentCombatStateGameplayTag() const;

protected:

    /** 최근 이동 입력을 우선 사용하고, 없다면 마지막 유효 입력을 fallback으로 사용한다. */
    FVector2D GetPreferredMoveInput2D() const;

    /** 입력 방향을 전/후/좌/우 Variant로 양자화한다. */
    EMHDirectionalVariant ResolveDirectionalVariantFromInput(bool bPreserveActorFacing) const;

    /** 납도 롤 / 발도 중립 롤에서 방향 입력 쪽으로 캐릭터를 회전시킨다. */
    bool TryRotateActorTowardsMoveInput();

    /** 노티파이 스테이트로 열어둔 회전 조정 구간에서 입력 방향 기준 회전을 갱신한다. */
    void UpdateDirectionalTurnWindow(float DeltaSeconds);

    /** 입력 방향과 회전 제한값을 이용해 실제 회전을 적용한다. */
    bool TryApplyDirectionalTurnWindowRotation(float DeltaSeconds);

    /** 공격 후 연계 회피 컨텍스트인지 판정한다. */
    bool IsLongSwordAttackChainDodgeContext() const;

    /** 현재 상태에 맞는 납도 롤 몽타주 반환 */
    UAnimMontage* ResolveSheathedRollMontage() const;

    /** 현재 상태에 맞는 발도 롤 몽타주 반환 */
    UAnimMontage* ResolveUnsheathedRollMontage() const;

    /** 루트모션 롤 몽타주를 공통 방식으로 재생한다. */
    bool TryPlayRollMontage(UAnimMontage* InMontage);

    bool IsLongSwordDrawEntryPattern(const FGameplayTag& InPatternTag) const;
    bool TryResolveAndHandleLongSwordPattern(const FGameplayTag& PreferredPatternTag = FGameplayTag());
    bool TryHandleWeaponComboInput(const FGameplayTag& InPatternTag);
    bool TryRequestLongSwordEarlyTransition();

    // 납도 시작 가능 여부
    bool CanStartSheathe() const;

    // 납도 몽타주 재생
    void StartSheathe();

    // 납도 몽타주 종료 처리
    void HandleSheatheMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 구르기 몽타주 종료 처리
    void HandleRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma region WeaponAnimationLayerFunctions
    // 현재 장착 무기의 애님 레이어 갱신
    void RefreshWeaponAnimationLayerState();

    // 현재 장착 무기의 애님 레이어 연결
    void LinkCurrentWeaponAnimLayer();

    // 현재 장착 무기의 애님 레이어 해제
    void UnlinkCurrentWeaponAnimLayer();

    // 현재 장착 무기의 링크 대상 레이어 클래스 반환
    TSoftClassPtr<UAnimInstance> GetCurrentWeaponLinkedAnimLayerClass() const;
#pragma endregion

    UPROPERTY(Transient)
    bool bWeaponAnimLayerLinked = false;
    
    
    
#pragma region Attribute Delegate _이건주
public:
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnStaminaChanged;
    
    UPROPERTY(BlueprintAssignable, Category="UI|Attributes")
    FMHOnVitalChanged OnSpiritGaugeChanged;
    
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

    // 중복 바인딩 가드 플래그
    bool bAttributeDelegatesBound = false;
#pragma endregion
};
