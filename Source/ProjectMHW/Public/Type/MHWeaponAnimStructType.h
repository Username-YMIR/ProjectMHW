#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "MHWeaponAnimStructType.generated.h"

class UAnimInstance;
class UAnimMontage;

/**
 * 방향 Variant 분기용 Enum
 * - None      : 방향 해석 실패 또는 기본 Variant 사용
 * - Forward   : 전방 Variant
 * - Backward  : 후방 Variant
 * - Left      : 좌측 Variant
 * - Right     : 우측 Variant
 */
UENUM(BlueprintType)
enum class EMHDirectionalVariant : uint8
{
    None,
    Forward,
    Backward,
    Left,
    Right,
};

/**
 * 회피 입력이 어떤 문맥에서 들어왔는지 표현하는 Enum
 * - Sheathed          : 납도 상태 회피
 * - UnsheathedNeutral : 발도 중립 상태 회피
 * - AttackChain       : 공격 후 연계 회피
 */
UENUM(BlueprintType)
enum class EMHDodgeContext : uint8
{
    Sheathed,
    UnsheathedNeutral,
    AttackChain,
};

/**
 * 방향별 몽타주 Variant 세트
 *
 * 기본적으로 DefaultMontage를 fallback으로 사용하고,
 * 필요할 때만 방향별 전용 몽타주를 추가로 지정한다.
 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHDirectionalMontageSet
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> DefaultMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> ForwardMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> BackwardMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> LeftMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> RightMontage;

    /** 방향 Variant에 대응하는 몽타주 Soft Pointer 반환 */
    TSoftObjectPtr<UAnimMontage> GetMontageByVariant(const EMHDirectionalVariant InVariant) const
    {
        switch (InVariant)
        {
        case EMHDirectionalVariant::Forward:
            return !ForwardMontage.IsNull() ? ForwardMontage : DefaultMontage;
        case EMHDirectionalVariant::Backward:
            return !BackwardMontage.IsNull() ? BackwardMontage : DefaultMontage;
        case EMHDirectionalVariant::Left:
            return !LeftMontage.IsNull() ? LeftMontage : DefaultMontage;
        case EMHDirectionalVariant::Right:
            return !RightMontage.IsNull() ? RightMontage : DefaultMontage;
        default:
            return DefaultMontage;
        }
    }
};

/**
 * 무기별 애니메이션 설정
 *
 * AnimBP 교체 구조를 버리고,
 * 마스터 AnimBP + Linked Anim Layer + FullBodyActionSlot 기준으로 확장할 수 있게 설계한다.
 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHWeaponAnimConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> DrawMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> SheatheMontage;

    /** 레거시 호환용 발도 상태 롤 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> UnsheathedRollMontage;

    /** 레거시 호환용 발도 상태 회피 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> UnsheathedDodgeMontage;

    /** 레거시 호환용 발도 상태 AnimBP */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftClassPtr<UAnimInstance> UnsheathedAnimClass;

    /** 마스터 AnimBP에 링크할 무기별 Anim Layer 클래스 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftClassPtr<UAnimInstance> LinkedWeaponAnimLayerClass;

    /** 발도 중립 상태에서 사용하는 전방 롤 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation|Roll")
    TSoftObjectPtr<UAnimMontage> NeutralUnsheathedForwardRollMontage;

    /** 공격 후 연계 회피용 방향 Variant 세트 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation|Roll")
    FMHDirectionalMontageSet ChainRollMontages;

    /** 뒤로 빠지며 베는 기본 Fade Slash 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation|Variant")
    TSoftObjectPtr<UAnimMontage> FadeSlashBackwardMontage;

    /** 좌측으로 빠지며 베는 Lateral Fade Slash 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation|Variant")
    TSoftObjectPtr<UAnimMontage> LateralFadeSlashLeftMontage;

    /** 우측으로 빠지며 베는 Lateral Fade Slash 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation|Variant")
    TSoftObjectPtr<UAnimMontage> LateralFadeSlashRightMontage;
};
