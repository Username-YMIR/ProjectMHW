#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHLongSwordComboGraph.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHLongSwordComboGraph, Log, All);

class UAnimMontage;

USTRUCT(BlueprintType)
struct FMHLongSwordComboBranch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag RequiredInputPatternTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag NextMoveTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    int32 BranchPriority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    bool bRequiresCounterSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    EMHLongSwordForesightPhase RequiredForesightPhase = EMHLongSwordForesightPhase::None;
};

USTRUCT(BlueprintType)
struct FMHLongSwordComboNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag MoveTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FName SectionName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition")
    bool bAllowEarlyTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition")
    bool bUseNotifyDrivenEarlyTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition", meta = (ClampMin = "0.0"))
    float EarlyTransitionLeadTime = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition", meta = (ClampMin = "0.0"))
    float TransitionBlendOutTime = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition", meta = (ClampMin = "0.0"))
    float TransitionBlendInTime = 0.10f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinCommittedNormalizedTime = 0.50f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Transition")
    bool bAllowDodgeCancel = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Next")
    TArray<FMHLongSwordComboBranch> Branches;
};

UCLASS(BlueprintType)
class PROJECTMHW_API UMHLongSwordComboGraph : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Entry")
    TArray<FMHLongSwordComboBranch> EntryBranches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TArray<FMHLongSwordComboNode> Nodes;
 
public:
    const FMHLongSwordComboNode* FindNode(const FGameplayTag& InMoveTag) const;
    const FMHLongSwordComboNode* FindBestEntryNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const;
    const FMHLongSwordComboNode* FindBestNextNode(const FGameplayTag& InCurrentMoveTag, const FGameplayTag& InPatternTag, bool bInCounterSuccess, EMHLongSwordForesightPhase InForesightPhase) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Combo|Tools")
    void PopulateDefaults_LongSword();

private:
    const FMHLongSwordComboNode* SelectBestNodeFromBranches(const TArray<FMHLongSwordComboBranch>& InBranches, const FGameplayTag& InCurrentMoveTag, const FGameplayTag& InPatternTag, bool bInCounterSuccess, EMHLongSwordForesightPhase InForesightPhase) const;
};
