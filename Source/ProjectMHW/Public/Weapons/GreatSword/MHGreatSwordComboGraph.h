#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapons/GreatSword/MHGreatSwordComboTypes.h"
#include "MHGreatSwordComboGraph.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGreatSwordComboGraph, Log, All);

// ===== Combo Data =====
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHGreatSwordComboBranch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag RequiredInputPatternTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    EMHGreatSwordTransitionPhase RequiredPhase = EMHGreatSwordTransitionPhase::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FMHGreatSwordActionDirective NextDirective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    int32 BranchPriority = 0;
};

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHGreatSwordComboNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FGameplayTag MoveTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TArray<FMHGreatSwordComboBranch> Branches;
};

// ===== Graph =====
UCLASS(BlueprintType)
class PROJECTMHW_API UMHGreatSwordComboGraph : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Entry")
    TArray<FMHGreatSwordComboBranch> EntryBranches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TArray<FMHGreatSwordComboNode> Nodes;

public:
    bool FindBestDirective(const FGameplayTag& InPatternTag, const FMHGreatSwordTransitionContext& InContext, FMHGreatSwordActionDirective& OutDirective) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Combo|Tools")
    void PopulateDefaults_GreatSword();

private:
    const FMHGreatSwordComboNode* FindNode(const FGameplayTag& InMoveTag) const;
    bool SelectBestDirectiveFromBranches(const TArray<FMHGreatSwordComboBranch>& InBranches, const FGameplayTag& InPatternTag, EMHGreatSwordTransitionPhase InPhase, FMHGreatSwordActionDirective& OutDirective) const;
};
