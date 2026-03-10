#pragma once

//손승우 수정: 롱소드 입력 패턴 정의용 DataAsset, 자동 채우기용 교체 함수 추가

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

	//손승우 추가: 자동 채우기 라이브러리에서 전체 패턴 정의를 교체할 때 사용
	void ReplacePatternDefinitions(const TArray<FMHInputPatternDefinition>& InPatternDefinitions);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	TArray<FMHInputPatternDefinition> PatternDefinitions;
};