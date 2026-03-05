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

    // 플레이어 비주얼 적용
    void ApplyPlayerVisuals();

    // 이동 파라미터 적용
    void ApplyMovementConfig();

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 이동 입력
    void Input_Move(const FInputActionValue& InputActionValue);

    // 시점 입력
    void Input_Look(const FInputActionValue& InputActionValue);

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

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    EMHPlayerLocomotionState LocomotionState = EMHPlayerLocomotionState::Idle; // 로코모션 상태
    // ===== End Movement =====

    // ===== Visual =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh; // 기본 스켈레톤 메쉬

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
    TSoftClassPtr<UAnimInstance> DefaultAnimClass; // 기본 애님 BP 클래스
    // ===== End Visual =====

    // ===== Inputs =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset; // 입력 설정
    // ===== End Inputs =====

    // ===== Weapon =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTag; // 무기 타입 태그
    // ===== End Weapon =====
};
