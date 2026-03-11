#pragma once

//손승우 추가: 롱소드 입력 패턴 정의용 DataAsset

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Combat/Input/MHCombatInputTypes.h"
#include "DataAsset_LSInputPatternSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLSInputPatternSet, Log, All);

UCLASS(BlueprintType)
class PROJECTMHW_API UDataAsset_LSInputPatternSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UDataAsset_LSInputPatternSet();

	const TArray<FMHInputPatternDefinition>& GetPatternDefinitions() const
	{
		return PatternDefinitions;
	}

	const FMHInputPatternDefinition* FindPatternDefinition(const FGameplayTag& PatternTag) const;

	void ReplacePatternDefinitions(const TArray<FMHInputPatternDefinition>& InPatternDefinitions);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	TArray<FMHInputPatternDefinition> PatternDefinitions;
};
