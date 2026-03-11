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

class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
class UDataAsset_LSInputPatternSet;
class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class AMHWeaponInstance;
struct FInputActionValue;

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

    // 기본 공격
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void UsePrimaryAction();

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
    
    // 현재 장착 중인 무기 인스턴스 반환
    AMHWeaponInstance* GetEquippedWeapon() const { return EquippedWeapon; }

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void Notify_LongSwordForesightCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearLongSwordForesightCounterSuccess();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    bool HasLongSwordForesightCounterSuccess() const { return bLongSwordForesightCounterSuccess; }

    bool TryStartAutoSheatheAfterLongSwordMove(const FGameplayTag& CompletedMoveTag);
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

    FGameplayTag ResolveLongSwordPatternForPrimaryInput() const;
    FGameplayTag ResolveLongSwordPatternForSecondaryInput() const;
    FGameplayTag ResolveLongSwordPatternForWeaponSpecialInput() const;
    FGameplayTag ResolveLongSwordPatternForDodgeInput() const;
    FGameplayTag ResolveLongSwordPatternForCompositeInput() const;

    bool IsLongSwordEquipped() const;
    bool HasMovementInputForCombat() const;
    bool IsStandingStillForCombat() const;
    bool IsInLongSwordSpecialSheatheState() const;

    FGameplayTag GetCurrentWeaponTypeGameplayTag() const;
    FGameplayTag GetCurrentWeaponSheathGameplayTag() const;
    FGameplayTag GetCurrentCombatStateGameplayTag() const;

    bool IsLongSwordDrawEntryPattern(const FGameplayTag& InPatternTag) const;
    bool TryResolveAndHandleLongSwordPattern(const FGameplayTag& PreferredPatternTag = FGameplayTag());
    bool TryHandleWeaponComboInput(const FGameplayTag& InPatternTag);

    // 납도 시작 가능 여부
    bool CanStartSheathe() const;

    // 납도 몽타주 재생
    void StartSheathe();

    // 납도 몽타주 종료 처리
    void HandleSheatheMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 구르기 몽타주 종료 처리
    void HandleRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 납도/발도 상태에 따른 AnimBP 적용
    void UpdateAnimClassByWeaponState();
};
