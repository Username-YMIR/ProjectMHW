#pragma once

//손승우 추가: 롱소드 입력 패턴 DA / 공격 메타 DT 자동 채우기 유틸

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MHCombatDataPopulatorLibrary.generated.h"

UCLASS()
class PROJECTMHW_API UMHCombatDataPopulatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MH|Combat|Populate")
	static bool PopulateLongSwordInputPatternSet(UObject* InputPatternAsset);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MH|Combat|Populate")
	static bool PopulateLongSwordAttackMetaTable(UObject* AttackMetaTableAsset);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MH|Combat|Populate")
	static bool PopulateLongSwordCombatData(UObject* InputPatternAsset, UObject* AttackMetaTableAsset);
};