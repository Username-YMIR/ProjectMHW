#include "Weapons/LongSword/MHLongSwordComboComponent.h"

#include "Weapons/LongSword/MHLongSwordComboGraph.h"

DEFINE_LOG_CATEGORY(LogMHLongSwordComboComponent);

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
        UE_LOG(LogMHLongSwordComboComponent, Verbose, TEXT("유효하지 않은 입력 패턴은 버퍼에 저장하지 않습니다."));
        return false;
    }

    // 이미 콤보가 진행 중이라면 체인 윈도우가 열려 있을 때만 다음 입력을 예약한다.
    if (bComboActive && !bChainWindowOpen)
    {
        UE_LOG(LogMHLongSwordComboComponent, Verbose, TEXT("체인 윈도우가 닫힌 상태라 입력 패턴을 저장하지 않습니다. Pattern=%s"), *InPatternTag.ToString());
        return false;
    }

    BufferedInputPatternTag = InPatternTag;
    bBufferedInputAccepted = true;
    UE_LOG(LogMHLongSwordComboComponent, Verbose, TEXT("입력 패턴을 버퍼에 저장했습니다. Pattern=%s"), *InPatternTag.ToString());
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
    // 새 기술이 실제로 커밋되면 기존 예약 입력은 비우고, 다음 윈도우가 열릴 때까지 입력 대기를 닫는다.
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
