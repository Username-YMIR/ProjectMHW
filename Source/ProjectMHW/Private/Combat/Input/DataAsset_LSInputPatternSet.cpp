#include "Combat/Input/DataAsset_LSInputPatternSet.h"

//손승우 추가: 롱소드 입력 패턴 데이터 조회 구현

DEFINE_LOG_CATEGORY(LogLSInputPatternSet);

UDataAsset_LSInputPatternSet::UDataAsset_LSInputPatternSet()
{
}

const FMHInputPatternDefinition* UDataAsset_LSInputPatternSet::FindPatternDefinition(const FGameplayTag& PatternTag) const
{
	if (!PatternTag.IsValid())
	{
		UE_LOG(LogLSInputPatternSet, Warning, TEXT("%s : Invalid PatternTag"), *GetName());
		return nullptr;
	}

	for (const FMHInputPatternDefinition& PatternDefinition : PatternDefinitions)
	{
		if (PatternDefinition.PatternTag == PatternTag)
		{
			return &PatternDefinition;
		}
	}

	UE_LOG(LogLSInputPatternSet, Verbose, TEXT("%s : PatternTag not found -> %s"), *GetName(), *PatternTag.ToString());
	return nullptr;
}
