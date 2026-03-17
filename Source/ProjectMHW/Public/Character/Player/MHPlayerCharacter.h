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

UENUM(BlueprintType)
enum class EMHLongSwordCounterWindowType : uint8
{
    None                    UMETA(DisplayName = "None"),
    Foresight               UMETA(DisplayName = "Foresight"),
    SpecialSheatheSlash     UMETA(DisplayName = "SpecialSheatheSlash"),
    SpecialSheatheSpirit    UMETA(DisplayName = "SpecialSheatheSpirit")
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

UCLASS()
class PROJECTMHW_API AMHPlayerCharacter : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHPlayerCharacter();

protected:
    virtual void BeginPlay() override;

    void InitializeCapsuleSettings();
    
    void InitializeMeshCollisionSettings();
    
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

    // 상호작용 입력
    void Input_Interact(const FInputActionValue& InputActionValue);

    // 기본 공격 입력
    void Input_AttackPrimary(const FInputActionValue& InputActionValue);

    // 보조 공격 입력
    void Input_AttackSecondary(const FInputActionValue& InputActionValue);

    // 무기 특수 입력
    void Input_WeaponSpecial(const FInputActionValue& InputActionValue); //손승우 추가

    // 디버그 자기 대미지 입력(키보드 1)
    void Input_DebugSelfDamage();

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

public:
    // 상호작용
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void Interact();

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

    UFUNCTION(BlueprintCallable, Category = "Debug|Damage")
    void ApplyDebugDamageToSelf();

    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage);

    UFUNCTION(BlueprintCallable, Category = "Debug|Damage")
    void ApplyDebugDamageFromSource(AActor* InSourceActor, float InPhysicalDamage, const FGameplayTag& InAttackTag);

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (AllowPrivateAccess = "true"))
    float CurrentStamina = 0.0f; // 현재 스태미나
    // ===== End Stamina =====

    // ===== Inputs =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset; // 입력 설정

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_LSInputPatternSet> LongSwordInputPatternSet; // 롱소드 입력 패턴 DA
    // ===== End Inputs =====

    // ===== Weapon =====
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
    // ===== End Weapon =====

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

    // ===== Terrain Hooks =====
    bool TryEnterSlide();
    bool TryStartClimb();
    bool TryJumpOffLedge();
    void HandleLandingState();
    // ===== End Terrain Hooks =====

    // 무기 메쉬 적용
    void SpawnAndEquipDefaultWeapon();

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
    FMHHitAcknowledge BuildLongSwordInvulnerableHitAcknowledge() const;

    // 발도 상태에서 첫 시작 공격을 선택하는 문맥인지 확인한다.
    bool IsLongSwordStartAttackContext() const;

    // 현재 콤보가 진행 중인 파생 문맥인지 확인한다.
    bool IsLongSwordFollowupContext() const;

    // 베어내리기 계열에서 좌우 이동베기 Variant를 사용할 수 있는지 확인한다.
    bool ShouldUseDirectionalLateralFadeSlash() const;
    bool ShouldUseLateralFadeSlashPattern() const;

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

    // 현재 장착 무기의 애님 레이어 갱신
    void RefreshWeaponAnimationLayerState();

    // 현재 장착 무기의 애님 레이어 연결
    void LinkCurrentWeaponAnimLayer();

    // 현재 장착 무기의 애님 레이어 해제
    void UnlinkCurrentWeaponAnimLayer();

    // 현재 장착 무기의 링크 대상 레이어 클래스 반환
    TSoftClassPtr<UAnimInstance> GetCurrentWeaponLinkedAnimLayerClass() const;

    UPROPERTY(Transient)
    bool bWeaponAnimLayerLinked = false;
};
