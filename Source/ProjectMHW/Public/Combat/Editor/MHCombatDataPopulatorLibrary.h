#pragma once

//손승우 추가: 롱소드 입력 패턴 DA / 공격 메타 DT / 공격 정의 DT 자동 채우기 유틸

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MHCombatDataPopulatorLibrary.generated.h"

class UGameplayEffect;

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
	static bool PopulateLongSwordAttackDefinitionTable(UObject* AttackDefinitionTableAsset, TSubclassOf<UGameplayEffect> DefaultDamageEffectClass);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MH|Combat|Populate")
	static bool PopulateLongSwordCombatData(UObject* InputPatternAsset, UObject* AttackMetaTableAsset);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MH|Combat|Populate")
	static bool PopulateLongSwordCombatDataExtended(UObject* InputPatternAsset, UObject* AttackMetaTableAsset, UObject* AttackDefinitionTableAsset, TSubclassOf<UGameplayEffect> DefaultDamageEffectClass);
};
