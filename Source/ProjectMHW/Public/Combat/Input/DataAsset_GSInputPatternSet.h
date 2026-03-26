#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/Input/MHCombatInputTypes.h"
#include "DataAsset_GSInputPatternSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDataAsset_GSInputPatternSet, Log, All);

UCLASS(BlueprintType)
class PROJECTMHW_API UDataAsset_GSInputPatternSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UDataAsset_GSInputPatternSet();

// ===== Input Patterns =====
public:
    const TArray<FMHInputPatternDefinition>& GetPatternDefinitions() const
    {
        return PatternDefinitions;
    }

    const FMHInputPatternDefinition* FindPatternDefinition(const FGameplayTag& PatternTag) const;
    void ReplacePatternDefinitions(const TArray<FMHInputPatternDefinition>& InPatternDefinitions);

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
    TArray<FMHInputPatternDefinition> PatternDefinitions;
};
