#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/MHCharacterBase.h"
#include "Type/MHPlayerStructType.h"
#include "MHPlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerCharacter, Log, All)

class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
class USkeletalMesh;
class UAnimInstance;
struct FInputActionValue;

UCLASS()
class PROJECTMHW_API AMHPlayerCharacter : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHPlayerCharacter();

protected:
    virtual void BeginPlay() override;

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

public:
    // 상호작용
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void Interact();

    // 기본 공격
    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void UsePrimaryAction();

protected:
    // ===== Camera =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom; // 스프링암

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera; // 카메라
    // ===== End Camera =====

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
    // ===== End Inputs =====

    // ===== Weapon =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTag; // 무기 타입 태그
    // ===== End Weapon =====

// 동일 카테고리 오브젝트가 3개 이상이면 region으로 구분
#pragma region Visual
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh; // 기본 스켈레톤 메쉬

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftClassPtr<UAnimInstance> DefaultAnimClass; // 기본 AnimBP

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FVector DefaultMeshRelativeLocation = FVector(0.f, 0.f, -90.f); // 메쉬 위치 보정

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    FRotator DefaultMeshRelativeRotation = FRotator(0.f, -90.f, 0.f); // 메쉬 회전 보정
#pragma endregion

private:
    // 스프린트 입력 유지 여부
    bool bSprintHeld = false;

    // 현재 스프린트 상태 여부
    bool bIsSprinting = false;

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
};
