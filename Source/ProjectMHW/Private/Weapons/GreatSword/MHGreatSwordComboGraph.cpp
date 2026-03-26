#include "Weapons/GreatSword/MHGreatSwordComboGraph.h"

#include "GameplayTags/MHGreatSwordGameplayTags.h"
#include "GameplayTags/MHInputPatternGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHGreatSwordComboGraph);

namespace
{
    static FMHGreatSwordActionDirective MakePlayMove(const FGameplayTag& InMoveTag)
    {
        FMHGreatSwordActionDirective Directive;
        Directive.Type = EMHGreatSwordDirectiveType::PlayMove;
        Directive.MoveTag = InMoveTag;
        return Directive;
    }

    static FMHGreatSwordActionDirective MakeBeginCharge(const EMHGreatSwordChargeFamily InFamily, const bool bInStartedFromSheathedForward = false, const bool bInUsePostTackleChargeFamily = false)
    {
        FMHGreatSwordActionDirective Directive;
        Directive.Type = EMHGreatSwordDirectiveType::BeginCharge;
        Directive.ChargeFamilyHint = InFamily;
        Directive.bStartedFromSheathedForward = bInStartedFromSheathedForward;
        Directive.bUsePostTackleChargeFamily = bInUsePostTackleChargeFamily;
        return Directive;
    }

    static FMHGreatSwordActionDirective MakeEnterGuard(const FGameplayTag& InMoveTag)
    {
        FMHGreatSwordActionDirective Directive;
        Directive.Type = EMHGreatSwordDirectiveType::EnterGuard;
        Directive.MoveTag = InMoveTag;
        return Directive;
    }

    static FMHGreatSwordComboBranch MakeBranch(const FGameplayTag& InPatternTag, const EMHGreatSwordTransitionPhase InPhase, const FMHGreatSwordActionDirective& InDirective, const int32 InPriority)
    {
        FMHGreatSwordComboBranch Branch;
        Branch.RequiredInputPatternTag = InPatternTag;
        Branch.RequiredPhase = InPhase;
        Branch.NextDirective = InDirective;
        Branch.BranchPriority = InPriority;
        return Branch;
    }

    static FMHGreatSwordComboNode MakeNode(const FGameplayTag& InMoveTag, const TArray<FMHGreatSwordComboBranch>& InBranches)
    {
        FMHGreatSwordComboNode Node;
        Node.MoveTag = InMoveTag;
        Node.Branches = InBranches;
        return Node;
    }
}

const FMHGreatSwordComboNode* UMHGreatSwordComboGraph::FindNode(const FGameplayTag& InMoveTag) const
{
    for (const FMHGreatSwordComboNode& Node : Nodes)
    {
        if (Node.MoveTag == InMoveTag)
        {
            return &Node;
        }
    }

    return nullptr;
}

bool UMHGreatSwordComboGraph::SelectBestDirectiveFromBranches(const TArray<FMHGreatSwordComboBranch>& InBranches, const FGameplayTag& InPatternTag, const EMHGreatSwordTransitionPhase InPhase, FMHGreatSwordActionDirective& OutDirective) const
{
    const FMHGreatSwordComboBranch* BestBranch = nullptr;

    for (const FMHGreatSwordComboBranch& Branch : InBranches)
    {
        if (!Branch.RequiredInputPatternTag.IsValid() || Branch.RequiredInputPatternTag != InPatternTag)
        {
            continue;
        }

        if (Branch.RequiredPhase != EMHGreatSwordTransitionPhase::None && Branch.RequiredPhase != InPhase)
        {
            continue;
        }

        if (!Branch.NextDirective.IsValid())
        {
            continue;
        }

        if (BestBranch == nullptr || Branch.BranchPriority > BestBranch->BranchPriority)
        {
            BestBranch = &Branch;
        }
    }

    if (BestBranch == nullptr)
    {
        OutDirective.Reset();
        return false;
    }

    OutDirective = BestBranch->NextDirective;
    return true;
}

bool UMHGreatSwordComboGraph::FindBestDirective(const FGameplayTag& InPatternTag, const FMHGreatSwordTransitionContext& InContext, FMHGreatSwordActionDirective& OutDirective) const
{
    OutDirective.Reset();

    if (!InPatternTag.IsValid())
    {
        return false;
    }

    if (InContext.Phase == EMHGreatSwordTransitionPhase::Entry)
    {
        return SelectBestDirectiveFromBranches(EntryBranches, InPatternTag, InContext.Phase, OutDirective);
    }

    const FGameplayTag LookupMoveTag = InContext.Phase == EMHGreatSwordTransitionPhase::ChargeFollowUp
        ? InContext.ChargeFollowUpSourceMoveTag
        : InContext.CurrentMoveTag;

    const FMHGreatSwordComboNode* Node = FindNode(LookupMoveTag);
    if (!Node)
    {
        UE_LOG(LogMHGreatSwordComboGraph, Verbose, TEXT("대검 콤보 노드를 찾지 못했습니다. Move=%s Pattern=%s Phase=%d"), *LookupMoveTag.ToString(), *InPatternTag.ToString(), static_cast<int32>(InContext.Phase));
        return false;
    }

    if (!SelectBestDirectiveFromBranches(Node->Branches, InPatternTag, InContext.Phase, OutDirective))
    {
        UE_LOG(LogMHGreatSwordComboGraph, Verbose, TEXT("대검 콤보 분기를 찾지 못했습니다. Move=%s Pattern=%s Phase=%d"), *LookupMoveTag.ToString(), *InPatternTag.ToString(), static_cast<int32>(InContext.Phase));
        return false;
    }

    return true;
}

void UMHGreatSwordComboGraph::PopulateDefaults_GreatSword()
{
    using namespace MHGreatSwordGameplayTags;
    using namespace MHInputPatternGameplayTags;

    EntryBranches =
    {
        MakeBranch(InputPattern_GS_DrawOnly, EMHGreatSwordTransitionPhase::Entry, MakePlayMove(Move_GS_DrawOnly), 10),
        MakeBranch(InputPattern_GS_DrawCharge, EMHGreatSwordTransitionPhase::Entry, MakeBeginCharge(EMHGreatSwordChargeFamily::Charge, true), 20),
        MakeBranch(InputPattern_GS_DrawGuard, EMHGreatSwordTransitionPhase::Entry, MakeEnterGuard(Move_GS_DrawGuard), 30),
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::Entry, MakeBeginCharge(EMHGreatSwordChargeFamily::Charge), 40),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::Entry, MakePlayMove(Move_GS_OverheadSlash), 50),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::Entry, MakePlayMove(Move_GS_RisingSlash), 60),
        MakeBranch(InputPattern_GS_WeaponSpecial, EMHGreatSwordTransitionPhase::Entry, MakeEnterGuard(Move_GS_Guard), 70)
    };

    Nodes.Reset();

    Nodes.Add(MakeNode(Move_GS_DrawForwardSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_WideSlash), 10),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::Strong), 20),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_OverheadSlash), 30),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_RisingSlash), 40)
    }));

    Nodes.Add(MakeNode(Move_GS_OverheadSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::Charge), 10),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_Tackle), 20),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_RisingSlash), 30)
    }));

    Nodes.Add(MakeNode(Move_GS_RisingSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::Charge), 10),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_OverheadSlash), 20)
    }));

    Nodes.Add(MakeNode(Move_GS_WideSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::Charge), 10),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_OverheadSlash), 20),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_RisingSlash), 30)
    }));

    Nodes.Add(MakeNode(Move_GS_Tackle,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_WideSlash), 10),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::None, false, true), 20),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_JumpingWideSlash), 30)
    }));

    Nodes.Add(MakeNode(Move_GS_JumpingWideSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_WideSlash), 10)
    }));

    Nodes.Add(MakeNode(Move_GS_ChargeSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_WideSlash), 10),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakeBeginCharge(EMHGreatSwordChargeFamily::Strong), 20),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_OverheadSlash), 30),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_RisingSlash), 40),
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_WideSlash), 110),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::Strong), 120),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_OverheadSlash), 130),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_RisingSlash), 140)
    }));

    Nodes.Add(MakeNode(Move_GS_StrongChargeSlash,
    {
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_WideSlash), 10),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakeBeginCharge(EMHGreatSwordChargeFamily::TrueCharge), 20),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_OverheadSlash), 30),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::ChargeFollowUp, MakePlayMove(Move_GS_RisingSlash), 40),
        MakeBranch(InputPattern_GS_Primary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_WideSlash), 110),
        MakeBranch(InputPattern_GS_ForwardPrimary, EMHGreatSwordTransitionPhase::EarlyTransition, MakeBeginCharge(EMHGreatSwordChargeFamily::TrueCharge), 120),
        MakeBranch(InputPattern_GS_Secondary, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_OverheadSlash), 130),
        MakeBranch(InputPattern_GS_Simultaneous, EMHGreatSwordTransitionPhase::EarlyTransition, MakePlayMove(Move_GS_RisingSlash), 140)
    }));

    Nodes.Add(MakeNode(Move_GS_TrueChargeSlash, {}));

    UE_LOG(LogMHGreatSwordComboGraph, Verbose, TEXT("대검 기본 콤보 그래프를 초기화했습니다. Entry=%d Nodes=%d"), EntryBranches.Num(), Nodes.Num());
}
