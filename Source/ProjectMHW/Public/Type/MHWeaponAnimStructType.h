#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "MHWeaponAnimStructType.generated.h"

class UAnimMontage;
class UAnimInstance;

/** 무기별 애니메이션 설정 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHWeaponAnimConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> DrawMontage; // 발도 몽타주

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> SheatheMontage; // 납도 몽타주

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> UnsheathedRollMontage; // 발도 상태 구르기 몽타주

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftObjectPtr<UAnimMontage> UnsheathedDodgeMontage; // 발도 상태 회피(예비)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TSoftClassPtr<UAnimInstance> UnsheathedAnimClass; // 발도 상태 AnimBP
};
