#include "Combat/mh_attack_definition_library.h"

#include "Engine/DataTable.h"

bool UMHAttackDefinitionLibrary::FindAttackDefinitionRow(const UDataTable* InAttackDefinitionTable, FGameplayTag InAttackTag, FMHAttackDefinitionRow& OutRow)
{
    if (const FMHAttackDefinitionRow* Row = FindAttackDefinitionRowPtr(InAttackDefinitionTable, InAttackTag))
    {
        OutRow = *Row;
        return true;
    }

    return false;
}

const FMHAttackDefinitionRow* UMHAttackDefinitionLibrary::FindAttackDefinitionRowPtr(const UDataTable* InAttackDefinitionTable, const FGameplayTag& InAttackTag)
{
    if (!InAttackDefinitionTable || !InAttackTag.IsValid())
    {
        return nullptr;
    }

    const FName DirectRowName(*InAttackTag.ToString());
    if (const FMHAttackDefinitionRow* DirectRow = InAttackDefinitionTable->FindRow<FMHAttackDefinitionRow>(DirectRowName, TEXT("FindAttackDefinitionRowPtr"), false))
    {
        return DirectRow;
    }

    const FMHAttackDefinitionRow* MatchedRow = nullptr;
    InAttackDefinitionTable->ForeachRow<FMHAttackDefinitionRow>(TEXT("FindAttackDefinitionRowPtr"), [&InAttackTag, &MatchedRow](const FName& RowName, const FMHAttackDefinitionRow& Row)
    {
        if (!MatchedRow && Row.AttackTag == InAttackTag)
        {
            MatchedRow = &Row;
        }
    });

    return MatchedRow;
}
