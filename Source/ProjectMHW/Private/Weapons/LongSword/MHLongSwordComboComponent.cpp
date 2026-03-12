#include "Weapons/LongSword/MHLongSwordComboComponent.h"

#include "Weapons/LongSword/MHLongSwordComboGraph.h"

UMHLongSwordComboComponent::UMHLongSwordComboComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMHLongSwordComboComponent::SetComboGraph(UMHLongSwordComboGraph* InGraph)
{
    ComboGraph = InGraph;
    ResetCombo();
}

bool UMHLongSwordComboComponent::BufferInputPattern(const FGameplayTag& InPatternTag)
{
    if (!InPatternTag.IsValid())
    {
        return false;
    }

    if (bComboActive && !bChainWindowOpen)
    {
        return false;
    }

    BufferedInputPatternTag = InPatternTag;
    bBufferedInputAccepted = true;
    return true;
}

FGameplayTag UMHLongSwordComboComponent::ConsumeBufferedInputPattern()
{
    const FGameplayTag Result = BufferedInputPatternTag;
    BufferedInputPatternTag = FGameplayTag::EmptyTag;
    bBufferedInputAccepted = false;
    return Result;
}

bool UMHLongSwordComboComponent::HasBufferedInputPattern() const
{
    return BufferedInputPatternTag.IsValid();
}

bool UMHLongSwordComboComponent::HasAcceptedBufferedInputPattern() const
{
    return HasBufferedInputPattern() && bBufferedInputAccepted;
}

void UMHLongSwordComboComponent::SetChainWindowOpen(bool bOpen)
{
    bChainWindowOpen = bOpen;
}

const FMHLongSwordComboNode* UMHLongSwordComboComponent::SelectEntryNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const
{
    return ComboGraph ? ComboGraph->FindBestEntryNode(InPatternTag, bInCounterSuccess) : nullptr;
}

const FMHLongSwordComboNode* UMHLongSwordComboComponent::SelectNextNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const
{
    if (!ComboGraph)
    {
        return nullptr;
    }

    if (!bComboActive)
    {
        return SelectEntryNode(InPatternTag, bInCounterSuccess);
    }

    return ComboGraph->FindBestNextNode(CurrentMoveTag, InPatternTag, bInCounterSuccess);
}

void UMHLongSwordComboComponent::CommitMove(const FMHLongSwordComboNode& Node)
{
    CurrentMoveTag = Node.MoveTag;
    bComboActive = true;
    bChainWindowOpen = false;
    bBufferedInputAccepted = false;
    BufferedInputPatternTag = FGameplayTag::EmptyTag;
}

void UMHLongSwordComboComponent::ResetCombo()
{
    CurrentMoveTag = FGameplayTag::EmptyTag;
    bComboActive = false;
    bChainWindowOpen = false;
    bBufferedInputAccepted = false;
    BufferedInputPatternTag = FGameplayTag::EmptyTag;
}
