#include "Weapons/LongSword/MHLongSwordComboGraph.h"

const FMHLongSwordComboNode* UMHLongSwordComboGraph::FindNode(const FGameplayTag& InMoveTag) const
{
    for (const FMHLongSwordComboNode& Node : Nodes)
    {
        if (Node.MoveTag == InMoveTag)
        {
            return &Node;
        }
    }
    return nullptr;
}

const TArray<FGameplayTag>& UMHLongSwordComboGraph::GetNextList(const FMHLongSwordComboNode& Node, EMHComboInputType InputType)
{
    switch (InputType)
    {
    case EMHComboInputType::Primary:
        return Node.PrimaryNextMoves;
    case EMHComboInputType::Secondary:
        return Node.SecondaryNextMoves;
    case EMHComboInputType::Special:
        return Node.SpecialNextMoves;
    default:
        break;
    }

    static const TArray<FGameplayTag> Empty;
    return Empty;
}

const TArray<FGameplayTag>& UMHLongSwordComboGraph::GetEntryList(EMHComboInputType InputType) const
{
    switch (InputType)
    {
    case EMHComboInputType::Primary:
        return EntryMoves_Primary;
    case EMHComboInputType::Secondary:
        return EntryMoves_Secondary;
    case EMHComboInputType::Special:
        return EntryMoves_Special;
    default:
        break;
    }

    static const TArray<FGameplayTag> Empty;
    return Empty;
}
