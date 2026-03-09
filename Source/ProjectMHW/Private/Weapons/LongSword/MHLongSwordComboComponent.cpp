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

bool UMHLongSwordComboComponent::BufferInput(EMHComboInputType InputType)
{
    if (InputType == EMHComboInputType::None)
    {
        return false;
    }

    // 콤보 중에는 체인 윈도우 안에서만 다음 입력을 승인한다. //손승우 추가
    if (bComboActive && !bChainWindowOpen)
    {
        return false;
    }

    BufferedInput = InputType;
    bBufferedInputAccepted = true;
    return true;
}

EMHComboInputType UMHLongSwordComboComponent::ConsumeBufferedInput()
{
    const EMHComboInputType Result = BufferedInput;
    BufferedInput = EMHComboInputType::None;
    bBufferedInputAccepted = false;
    return Result;
}

bool UMHLongSwordComboComponent::HasBufferedInput() const
{
    return BufferedInput != EMHComboInputType::None;
}

bool UMHLongSwordComboComponent::HasAcceptedBufferedInput() const
{
    return HasBufferedInput() && bBufferedInputAccepted;
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

    // 새 모션 진입 시 다음 입력은 노티파이가 열어줄 때까지 닫아둔다. //손승우 수정
    bChainWindowOpen = false;
    bBufferedInputAccepted = false;
    BufferedInput = EMHComboInputType::None;
}

void UMHLongSwordComboComponent::ResetCombo()
{
    CurrentMoveTag = FGameplayTag();
    bComboActive = false;
    bChainWindowOpen = false;
    bBufferedInputAccepted = false;
    BufferedInput = EMHComboInputType::None;
}
