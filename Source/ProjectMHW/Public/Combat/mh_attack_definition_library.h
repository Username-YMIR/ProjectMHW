#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/mh_attack_definition_types.h"
#include "mh_attack_definition_library.generated.h"

class UDataTable;

UCLASS()
class PROJECTMHW_API UMHAttackDefinitionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Combat|AttackDefinition")
    static bool FindAttackDefinitionRow(const UDataTable* InAttackDefinitionTable, FGameplayTag InAttackTag, FMHAttackDefinitionRow& OutRow);

    static const FMHAttackDefinitionRow* FindAttackDefinitionRowPtr(const UDataTable* InAttackDefinitionTable, const FGameplayTag& InAttackTag);
};
