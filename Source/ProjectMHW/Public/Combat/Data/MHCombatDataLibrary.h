#pragma once

//손승우 추가: 전투 데이터 조회 유틸

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/Data/MHAttackMetaTypes.h"
#include "MHCombatDataLibrary.generated.h"

class UDataTable;

DECLARE_LOG_CATEGORY_EXTERN(LogMHCombatDataLibrary, Log, All);

UCLASS()
class PROJECTMHW_API UMHCombatDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MH|Combat|Data")
	static bool FindAttackMetaRowByTag(
		const UDataTable* InAttackMetaTable,
		const FGameplayTag& InMoveTag,
		FMHAttackMetaRow& OutAttackMetaRow);
};
