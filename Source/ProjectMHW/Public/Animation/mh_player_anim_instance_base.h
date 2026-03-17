#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "mh_player_anim_instance_base.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHPlayerAnimInstanceBase, Log, All);

class AMHPlayerCharacter;

/**
 * 플레이어 애님 인스턴스 공통 베이스
 * - 플레이어 캐릭터 캐시
 * - 이동 / 공중 / 무기 / 전투 상태 읽기
 * - 마스터 AnimBP와 Linked Layer가 공통으로 참조할 값 보관
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTMHW_API UMHPlayerAnimInstanceBase : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Anim")
    AMHPlayerCharacter* GetMHPlayerCharacter() const;

protected:
#pragma region Locomotion
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bHasAcceleration = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir = false;
#pragma endregion

#pragma region Weapon
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    FGameplayTag CurrentEquippedWeaponTag;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    FGameplayTag CurrentWeaponTypeTag;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    FGameplayTag CurrentWeaponSheathStateTag;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Weapon")
    bool bUseArmedLocomotion = false;
#pragma endregion

#pragma region Combat
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    FGameplayTag CurrentActionTag;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    FGameplayTag CurrentMoveTag;
#pragma endregion

private:
    TWeakObjectPtr<AMHPlayerCharacter> CachedPlayerCharacter;
};
