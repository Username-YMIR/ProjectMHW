#include "Combat/Data/MHCombatDataLibrary.h"

#include "Engine/DataTable.h"

//손승우 추가: 전투 메타 데이터 조회 구현

DEFINE_LOG_CATEGORY(LogMHCombatDataLibrary);

bool UMHCombatDataLibrary::FindAttackMetaRowByTag(
	const UDataTable* InAttackMetaTable,
	const FGameplayTag& InMoveTag,
	FMHAttackMetaRow& OutAttackMetaRow)
{
	if (!IsValid(InAttackMetaTable))
	{
		UE_LOG(LogMHCombatDataLibrary, Warning, TEXT("FindAttackMetaRowByTag : AttackMetaTable is invalid"));
		return false;
	}

	if (!InMoveTag.IsValid())
	{
		UE_LOG(LogMHCombatDataLibrary, Warning, TEXT("FindAttackMetaRowByTag : MoveTag is invalid"));
		return false;
	}

	static const FString ContextString = TEXT("FindAttackMetaRowByTag");

	TArray<FMHAttackMetaRow*> AttackMetaRows;
	InAttackMetaTable->GetAllRows<FMHAttackMetaRow>(ContextString, AttackMetaRows);

	for (const FMHAttackMetaRow* AttackMetaRow : AttackMetaRows)
	{
		if (AttackMetaRow == nullptr)
		{
			continue;
		}

		if (AttackMetaRow->MoveTag == InMoveTag)
		{
			OutAttackMetaRow = *AttackMetaRow;
			return true;
		}
	}

	UE_LOG(LogMHCombatDataLibrary, Verbose, TEXT("FindAttackMetaRowByTag : Row not found -> %s"), *InMoveTag.ToString());
	return false;
}
