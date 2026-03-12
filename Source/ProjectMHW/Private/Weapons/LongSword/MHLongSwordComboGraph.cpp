#include "Weapons/LongSword/MHLongSwordComboGraph.h"

#include <initializer_list>

#include "GameplayTags/MHInputPatternGameplayTags.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHLongSwordComboGraph);

namespace
{
    static FMHLongSwordComboBranch MakeBranch(const FGameplayTag& InPatternTag, const FGameplayTag& InMoveTag, int32 InPriority = 0, bool bInRequiresCounterSuccess = false, EMHLongSwordForesightPhase InRequiredForesightPhase = EMHLongSwordForesightPhase::None)
    {
        FMHLongSwordComboBranch Branch;
        Branch.RequiredInputPatternTag = InPatternTag;
        Branch.NextMoveTag = InMoveTag;
        Branch.BranchPriority = InPriority;
        Branch.bRequiresCounterSuccess = bInRequiresCounterSuccess;
        Branch.RequiredForesightPhase = InRequiredForesightPhase;
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
    const FGameplayTag& InCurrentMoveTag,
    const FGameplayTag& InPatternTag,
    bool bInCounterSuccess,
    EMHLongSwordForesightPhase InForesightPhase) const
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

        if (InCurrentMoveTag == MHLongSwordGameplayTags::Move_LS_ForesightSlash
            && Branch.RequiredForesightPhase != EMHLongSwordForesightPhase::None
            && Branch.RequiredForesightPhase != InForesightPhase)
        {
            UE_LOG(LogMHLongSwordComboGraph, Verbose, TEXT("간파 구간 불일치로 브랜치를 제외합니다. Pattern=%s RequiredPhase=%d CurrentPhase=%d"), *InPatternTag.ToString(), static_cast<int32>(Branch.RequiredForesightPhase), static_cast<int32>(InForesightPhase));
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
    return SelectBestNodeFromBranches(EntryBranches, FGameplayTag::EmptyTag, InPatternTag, bInCounterSuccess, EMHLongSwordForesightPhase::None);
}

const FMHLongSwordComboNode* UMHLongSwordComboGraph::FindBestNextNode(
    const FGameplayTag& InCurrentMoveTag,
    const FGameplayTag& InPatternTag,
    bool bInCounterSuccess,
    EMHLongSwordForesightPhase InForesightPhase) const
{
    const FMHLongSwordComboNode* CurrentNode = FindNode(InCurrentMoveTag);
    if (!CurrentNode)
    {
        return nullptr;
    }

    return SelectBestNodeFromBranches(CurrentNode->Branches, InCurrentMoveTag, InPatternTag, bInCounterSuccess, InForesightPhase);
}

void UMHLongSwordComboGraph::PopulateDefaults_LongSword()
{
    using namespace MHInputPatternGameplayTags;
    using namespace MHLongSwordGameplayTags;

    // ===== Entry Branches =====
    // 태도 입력 키 txt 기준 시작 공격 목록
    // - 좌클릭 : 내딛어베기
    // - Mouse5 : 베어내리기
    // - 우클릭 : 찌르기
    // - Mouse4 : 기인베기1
    // - Mouse4 + 좌클릭 : 기인찌르기
    EntryBranches =
    {
        MakeBranch(InputPattern_LS_DrawOnly, Move_LS_DrawOnly, 10),
        MakeBranch(InputPattern_LS_DrawAdvancingSlash, Move_LS_DrawAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_DrawSpiritSlash1, Move_LS_DrawSpiritSlash1, 30),
        MakeBranch(InputPattern_LS_AdvancingSlash, Move_LS_AdvancingSlash, 40),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 50),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 60),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 70),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 80),
    };
    // ===== End Entry Branches =====

    Nodes.Reset();

    // ===== Draw / Entry =====
    Nodes.Add(MakeNode(Move_LS_DrawOnly,
    {
        MakeBranch(InputPattern_LS_AdvancingSlash, Move_LS_AdvancingSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_DrawAdvancingSlash,
    {
        MakeBranch(InputPattern_LS_VerticalSlash, Move_LS_VerticalSlash, 10),
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
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));
    // ===== End Draw / Entry =====

    // ===== Normal =====
    Nodes.Add(MakeNode(Move_LS_AdvancingSlash,
    {
        MakeBranch(InputPattern_LS_VerticalSlash, Move_LS_VerticalSlash, 10),
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
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_Thrust,
    {
        MakeBranch(InputPattern_LS_RisingSlash, Move_LS_RisingSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_RisingSlash,
    {
        MakeBranch(InputPattern_LS_VerticalSlash, Move_LS_VerticalSlash, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash1, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_FadeSlash,
    {
        // 베어내리기 / 좌우이동베기 이후에는 좌클릭과 우클릭 모두 찌르기로 이어질 수 있다.
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 30),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));

    Nodes.Add(MakeNode(Move_LS_LateralFadeSlash,
    {
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritAdvancingSlash, 20),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 30),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 40),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50),
    }));
    // ===== End Normal =====

    // ===== Spirit =====
    Nodes.Add(MakeNode(Move_LS_SpiritSlash1,
    {
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 10),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21),
        MakeBranch(InputPattern_LS_Spirit, Move_LS_SpiritSlash2, 30),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40),
        MakeBranch(InputPattern_LS_ForesightSlash, Move_LS_ForesightSlash, 50),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 60),
    }));

    Nodes.Add(MakeNode(Move_LS_SpiritSlash2,
    {
        MakeBranch(InputPattern_LS_RisingSlash, Move_LS_RisingSlash, 10),
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
        MakeBranch(InputPattern_LS_RisingSlash, Move_LS_RisingSlash, 10),
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
    // ===== End Spirit =====

    // ===== Counter / Special =====
    Nodes.Add(MakeNode(Move_LS_ForesightSlash,
    {
        // 간파베기는 입력 타이밍을 1차/2차로 분리한다.
        // 1차 구간: 특수납도 / 기인찌르기
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 40, false, EMHLongSwordForesightPhase::Primary),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 50, false, EMHLongSwordForesightPhase::Primary),

        // 2차 구간: 후반 베기 이후 파생
        MakeBranch(InputPattern_LS_AdvancingSlash, Move_LS_AdvancingSlash, 10, false, EMHLongSwordForesightPhase::Secondary),
        MakeBranch(InputPattern_LS_FadeSlash, Move_LS_FadeSlash, 20, false, EMHLongSwordForesightPhase::Secondary),
        MakeBranch(InputPattern_LS_LateralFadeSlash, Move_LS_LateralFadeSlash, 21, false, EMHLongSwordForesightPhase::Secondary),
        MakeBranch(InputPattern_LS_Thrust, Move_LS_Thrust, 30, false, EMHLongSwordForesightPhase::Secondary),
        MakeBranch(InputPattern_LS_SpiritThrust, Move_LS_SpiritThrust, 41, false, EMHLongSwordForesightPhase::Secondary),
        MakeBranch(InputPattern_LS_SpecialSheathe, Move_LS_SpecialSheathe, 51, false, EMHLongSwordForesightPhase::Secondary),
    }));

    Nodes.Add(MakeNode(Move_LS_SpecialSheathe,
    {
        MakeBranch(InputPattern_LS_IaiSlash, Move_LS_IaiSlash, 10),
        MakeBranch(InputPattern_LS_IaiSpiritSlash, Move_LS_IaiSpiritSlash, 20),
    }));

    Nodes.Add(MakeNode(Move_LS_IaiSlash,
    {
        MakeBranch(InputPattern_LS_VerticalSlash, Move_LS_VerticalSlash, 10),
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
    // ===== End Counter / Special =====
}

