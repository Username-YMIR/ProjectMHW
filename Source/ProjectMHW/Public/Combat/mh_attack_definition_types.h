#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "mh_attack_definition_types.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHAttackDefinitionRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    FGameplayTag AttackTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float PhysicalScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float FireScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float WaterScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float ThunderScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float IceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    float DragonScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter")
    bool bCanBeForesightCountered = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter")
    bool bCanBeSpecialSheatheCountered = true;
};
