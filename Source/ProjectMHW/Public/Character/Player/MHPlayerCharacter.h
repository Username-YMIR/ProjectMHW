#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Character/MHCharacterBase.h"
#include "MHPlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerCharacter, Log, All)

class USpringArmComponent;
class UCameraComponent;
class UDataAsset_InputConfig;
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
    float WalkSpeed = 220.0f; // 걷기 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float RunSpeed = 420.0f; // 달리기 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float SprintSpeed = 560.0f; // 질주 속도
    // ===== End Movement =====

    // ===== Inputs =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset_InputConfig> InputConfigDataAsset; // 입력 설정
    // ===== End Inputs =====

    // ===== Weapon =====
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTag; // 무기 타입 태그
    // ===== End Weapon =====
};
