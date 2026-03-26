#include "Combat/Input/DataAsset_GSInputPatternSet.h"

DEFINE_LOG_CATEGORY(LogDataAsset_GSInputPatternSet);

UDataAsset_GSInputPatternSet::UDataAsset_GSInputPatternSet()
{
}

const FMHInputPatternDefinition* UDataAsset_GSInputPatternSet::FindPatternDefinition(const FGameplayTag& PatternTag) const
{
    if (!PatternTag.IsValid())
    {
        UE_LOG(LogDataAsset_GSInputPatternSet, Warning, TEXT("%s : 잘못된 입력 패턴 태그를 조회했습니다."), *GetName());
        return nullptr;
    }

    for (const FMHInputPatternDefinition& PatternDefinition : PatternDefinitions)
    {
        if (PatternDefinition.PatternTag == PatternTag)
        {
            return &PatternDefinition;
        }
    }

    UE_LOG(LogDataAsset_GSInputPatternSet, Verbose, TEXT("%s : 입력 패턴을 찾지 못했습니다. Pattern=%s"), *GetName(), *PatternTag.ToString());
    return nullptr;
}

void UDataAsset_GSInputPatternSet::ReplacePatternDefinitions(const TArray<FMHInputPatternDefinition>& InPatternDefinitions)
{
    PatternDefinitions = InPatternDefinitions;
    UE_LOG(LogDataAsset_GSInputPatternSet, Verbose, TEXT("%s : 입력 패턴 정의를 교체했습니다. Count=%d"), *GetName(), PatternDefinitions.Num());
}
