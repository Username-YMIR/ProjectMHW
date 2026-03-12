#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MHLongSwordComboComponent.generated.h"

class UMHLongSwordComboGraph;
struct FMHLongSwordComboNode;

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
