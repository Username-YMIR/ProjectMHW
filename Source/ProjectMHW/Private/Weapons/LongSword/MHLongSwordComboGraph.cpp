#include "Weapons/LongSword/MHLongSwordComboGraph.h"

#include <initializer_list>

#include "GameplayTags/MHInputPatternGameplayTags.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"

namespace
{
    static FMHLongSwordComboBranch MakeBranch(const FGameplayTag& InPatternTag, const FGameplayTag& InMoveTag, int32 InPriority = 0, bool bInRequiresCounterSuccess = false)
    {
        FMHLongSwordComboBranch Branch;
        Branch.RequiredInputPatternTag = InPatternTag;
        Branch.NextMoveTag = InMoveTag;
        Branch.BranchPriority = InPriority;
        Branch.bRequiresCounterSuccess = bInRequiresCounterSuccess;
        return Branch;
    }

    static FMHLongSwordComboNode MakeNode(const FGameplayTag& InMoveTag, std::initializer_list<FMHLongSwordComboBranch> InBranches)
    {
        FMHLongSwordComboNode Node;
        Node.MoveTag = InMoveTag;
        Node.SectionName = NAME_None;
        for (const FMHLongSwordComboBranch& Branch : InBranches)
        {
            Node.Branches.Add(Branch);
        }
        return Node;
    }
}

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

const FMHLongSwordComboNode* UMHLongSwordComboGraph::SelectBestNodeFromBranches(
    const TArray<FMHLongSwordComboBranch>& InBranches,
    const FGameplayTag& InPatternTag,
    bool bInCounterSuccess) const
{
    const FMHLongSwordComboBranch* BestBranch = nullptr;

    for (const FMHLongSwordComboBranch& Branch : InBranches)
    {
        if (!Branch.RequiredInputPatternTag.IsValid() || Branch.RequiredInputPatternTag != InPatternTag)
        {
            continue;
        }

        if (Branch.bRequiresCounterSuccess && !bInCounterSuccess)
        {
            continue;
        }

        if (BestBranch == nullptr || Branch.BranchPriority > BestBranch->BranchPriority)
        {
            BestBranch = &Branch;
        }
    }

    return BestBranch ? FindNode(BestBranch->NextMoveTag) : nullptr;
}

const FMHLongSwordComboNode* UMHLongSwordComboGraph::FindBestEntryNode(const FGameplayTag& InPatternTag, bool bInCounterSuccess) const
{
    return SelectBestNodeFromBranches(EntryBranches, InPatternTag, bInCounterSuccess);
}

const FMHLongSwordComboNode* UMHLongSwordComboGraph::FindBestNextNode(
    const FGameplayTag& InCurrentMoveTag,
    const FGameplayTag& InPatternTag,
    bool bInCounterSuccess) const
{
    const FMHLongSwordComboNode* CurrentNode = FindNode(InCurrentMoveTag);
    if (!CurrentNode)
    {
        return nullptr;
    }

    return SelectBestNodeFromBranches(CurrentNode->Branches, InPatternTag, bInCounterSuccess);
}

void UMHLongSwordComboGraph::PopulateDefaults_LongSword()
{
    using namespace MHInputPatternGameplayTags;
    using namespace MHLongSwordGameplayTags;

    EntryBranches =
    {
        MakeBranch(InputPattern_LS_DrawOnly, Move_LS_DrawOnly, 10),
        MakeBranch(InputPattern_LS_DrawAdvancingSlash, Move_LS_DrawAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_DrawSpiritSlash1, Move_LS_DrawSpiritSlash1, 30),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 70),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 71),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    };

    Nodes.Reset();

    Nodes.Add(MakeNode(Move_LS_DrawOnly,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 70),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 71),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_DrawAdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 70),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 71),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_DrawSpiritSlash1,
    {
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 50),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_AdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_VerticalSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_Thrust,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 70),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 71),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_RisingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_DownwardSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 70),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 71),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_FadeSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_LateralFadeSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash1,
    {
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 50),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash2,
    {
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash3, 50),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash3,
    {
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritRoundslash, 50),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritRoundslash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 90),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritAdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritThrust,
    {
        MakeBranch(InputPattern_LS_Helmbreaker, Move_LS_SpiritHelmbreaker, 130),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritHelmbreaker, {}));

    Nodes.Add(MakeNode(Move_LS_ForesightSlash,
    {
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritRoundslash, 200, true),
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 45),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpecialSheathe,
    {
        MakeBranch(InputPattern_LS_IaiSlash, Move_LS_IaiSlash, 110),
        MakeBranch(InputPattern_LS_IaiSpiritSlash, Move_LS_IaiSpiritSlash, 120),
    }));

    Nodes.Add(MakeNode(Move_LS_IaiSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_IaiSpiritSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_DownwardSlash, 40),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 100),
    }));
}
