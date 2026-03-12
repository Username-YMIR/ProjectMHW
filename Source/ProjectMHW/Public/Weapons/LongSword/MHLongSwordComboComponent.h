#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MHLongSwordComboComponent.generated.h"

class UMHLongSwordComboGraph;
struct FMHLongSwordComboNode;

DECLARE_LOG_CATEGORY_EXTERN(LogMHLongSwordComboComponent, Log, All);

UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class PROJECTMHW_API UMHLongSwordComboComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMHLongSwordComboComponent();

public:
    void SetComboGraph(UMHLongSwordComboGraph* InGraph);
    const UMHLongSwordComboGraph* GetComboGraph() const { return ComboGraph; }

    bool BufferInputPattern(const FGameplayTag& InPatternTag);
    FGameplayTag ConsumeBufferedInputPattern();

    /** 현재 버퍼에 저장된 입력 패턴을 조회한다. */
    const FGameplayTag& PeekBufferedInputPattern() const { return BufferedInputPatternTag; }

    /** 입력 패턴 버퍼가 비어 있지 않은지 확인한다. */
    bool HasBufferedInputPattern() const;
    bool HasAcceptedBufferedInputPattern() const;

    void SetChainWindowOpen(bool bOpen);
    bool IsChainWindowOpen() const { return bChainWindowOpen; }

    const FGameplayTag& GetCurrentMoveTag() const { return CurrentMoveTag; }
    bool IsComboActive() const { return bComboActive; }

    const FMHLongSwordComboNode* SelectEntryNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const;
    const FMHLongSwordComboNode* SelectNextNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const;

    void CommitMove(const FMHLongSwordComboNode& Node);
    void ResetCombo();

private:
    UPROPERTY(Transient)
    TObjectPtr<UMHLongSwordComboGraph> ComboGraph;

    UPROPERTY(Transient)
    FGameplayTag CurrentMoveTag;

    UPROPERTY(Transient)
    bool bComboActive = false;

    UPROPERTY(Transient)
    bool bChainWindowOpen = false;

    UPROPERTY(Transient)
    bool bBufferedInputAccepted = false;

    UPROPERTY(Transient)
    FGameplayTag BufferedInputPatternTag;
};
