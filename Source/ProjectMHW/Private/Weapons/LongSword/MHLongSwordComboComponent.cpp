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

void UMHLongSwordComboComponent::BufferInput(EMHComboInputType InputType)
{
    if (InputType == EMHComboInputType::None)
    {
        return;
    }

    BufferedInput = InputType;
}

EMHComboInputType UMHLongSwordComboComponent::ConsumeBufferedInput()
{
    const EMHComboInputType Result = BufferedInput;
    BufferedInput = EMHComboInputType::None;
    return Result;
}

bool UMHLongSwordComboComponent::HasBufferedInput() const
{
    return BufferedInput != EMHComboInputType::None;
}

void UMHLongSwordComboComponent::SetChainWindowOpen(bool bOpen)
{
    bChainWindowOpen = bOpen;
}

const FMHLongSwordComboNode* UMHLongSwordComboComponent::GetCurrentNode() const
{
    if (!ComboGraph || !bComboActive)
    {
        return nullptr;
    }

    return ComboGraph->FindNode(CurrentMoveTag);
}

const FMHLongSwordComboNode* UMHLongSwordComboComponent::SelectNextNode(EMHComboInputType InputType) const
{
    if (!ComboGraph)
    {
        return nullptr;
    }

    // 콤보 시작
    if (!bComboActive)
    {
        const TArray<FGameplayTag>& EntryList = ComboGraph->GetEntryList(InputType);
        for (const FGameplayTag& Tag : EntryList)
        {
            if (const FMHLongSwordComboNode* Node = ComboGraph->FindNode(Tag))
            {
                return Node;
            }
        }
        return nullptr;
    }

    // 콤보 중
    const FMHLongSwordComboNode* CurrentNode = GetCurrentNode();
    if (!CurrentNode)
    {
        return nullptr;
    }

    const TArray<FGameplayTag>& NextList = UMHLongSwordComboGraph::GetNextList(*CurrentNode, InputType);
    for (const FGameplayTag& Tag : NextList)
    {
        if (const FMHLongSwordComboNode* Node = ComboGraph->FindNode(Tag))
        {
            return Node;
        }
    }

    return nullptr;
}

void UMHLongSwordComboComponent::CommitMove(const FMHLongSwordComboNode& Node)
{
    CurrentMoveTag = Node.MoveTag;
    bComboActive = true;

    // 노티파이 기반 사용 시, 기본은 닫힘으로 시작(노티파이로 열기)
    bChainWindowOpen = true;
}

void UMHLongSwordComboComponent::ResetCombo()
{
    CurrentMoveTag = FGameplayTag();
    bComboActive = false;
    bChainWindowOpen = true;
    BufferedInput = EMHComboInputType::None;
}
