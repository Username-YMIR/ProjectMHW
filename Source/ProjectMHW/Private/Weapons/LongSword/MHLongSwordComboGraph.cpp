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

    // 태도 최신 콤보 규칙 반영:
    // - 시작 공격: 베어내리기 / 내딛어베기 / 찌르기 / 기인베기1 / 기인찌르기
    // - 좌우이동베기는 시작 공격으로 허용하지 않고, 후속 파생에서만 허용
    // - DownwardSlash는 현재 설계에서 실제 기술로 사용하지 않는다.
    EntryBranches =
    {
        MakeBranch(InputPattern_LS_DrawOnly, Move_LS_DrawOnly, 10),
        MakeBranch(InputPattern_LS_DrawAdvancingSlash, Move_LS_DrawAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_DrawSpiritSlash1, Move_LS_DrawSpiritSlash1, 30),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 50),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 60),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 70),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 80),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 90),
    };

    Nodes.Reset();

    Nodes.Add(MakeNode(Move_LS_DrawOnly,
    {
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 40),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 50),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 60),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 70),
    }));

    Nodes.Add(MakeNode(Move_LS_DrawAdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 60),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 70),
    }));

    Nodes.Add(MakeNode(Move_LS_DrawSpiritSlash1,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 11),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_AdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 60),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 70),
    }));

    Nodes.Add(MakeNode(Move_LS_VerticalSlash,
    {
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 50),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 60),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 70),
    }));

    Nodes.Add(MakeNode(Move_LS_Thrust,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_RisingSlash, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_RisingSlash, 11),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_RisingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_FadeSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 11),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 30),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_LateralFadeSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 11),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 30),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash1,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 11),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash2,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_RisingSlash, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_RisingSlash, 11),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash3, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash3,
    {
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritRoundslash, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritRoundslash,
    {
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 10),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritAdvancingSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_RisingSlash, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_RisingSlash, 11),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash3, 30),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritThrust,
    {
        MakeBranch(InputPattern_LS_Helmbreaker, Move_LS_SpiritHelmbreaker, 100),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritHelmbreaker,
    {
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 10),
    }));

    Nodes.Add(MakeNode(Move_LS_ForesightSlash,
    {
        // 간파베기는 입력 타이밍이 두 구간이지만, 현재 데이터 구조상 후속 선택지만 우선 정리한다.
        MakeBranch(InputPattern_LS_Basic, Move_LS_AdvancingSlash, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 20),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 30),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 31),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_SpecialSheathe,
    {
        MakeBranch(InputPattern_LS_IaiSlash, Move_LS_IaiSlash, 10),
        MakeBranch(InputPattern_LS_IaiSpiritSlash, Move_LS_IaiSpiritSlash, 20),
    }));

    Nodes.Add(MakeNode(Move_LS_IaiSlash,
    {
        MakeBranch(InputPattern_LS_Basic, Move_LS_VerticalSlash, 10),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 20),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 30),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 31),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_IaiSpiritSlash,
    {
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 10),
    }));
}
