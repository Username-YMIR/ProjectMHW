#include "Weapons/LongSword/MHLongSwordComboGraph.h"

#include "GameplayTags/MHLongSwordGameplayTags.h"

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

const TArray<FGameplayTag>& UMHLongSwordComboGraph::GetNextList(const FMHLongSwordComboNode& Node, EMHComboInputType InputType)
{
	switch (InputType)
	{
	case EMHComboInputType::Primary:
		return Node.PrimaryNextMoves;
	case EMHComboInputType::Secondary:
		return Node.SecondaryNextMoves;
	case EMHComboInputType::Special:
		return Node.SpecialNextMoves;
	default:
		return Node.PrimaryNextMoves;
	}
}

const TArray<FGameplayTag>& UMHLongSwordComboGraph::GetEntryList(EMHComboInputType InputType) const
{
	switch (InputType)
	{
	case EMHComboInputType::Primary:
		return EntryMoves_Primary;
	case EMHComboInputType::Secondary:
		return EntryMoves_Secondary;
	case EMHComboInputType::Special:
		return EntryMoves_Special;
	default:
		return EntryMoves_Primary;
	}
}

static FMHLongSwordComboNode MakeNode(const FGameplayTag& InTag, const TArray<FGameplayTag>& InPrimaryNext)
{
	FMHLongSwordComboNode Node;
	Node.MoveTag = InTag;
	Node.PrimaryNextMoves = InPrimaryNext;
	Node.SectionName = NAME_None;
	return Node;
}

void UMHLongSwordComboGraph::PopulateDefaults_LongSword()
{
	// 시작기(발도 상태 시작공격)
	EntryMoves_Primary =
	{
		MHLongSwordGameplayTags::Move_LS_FallingSlash,
		MHLongSwordGameplayTags::Move_LS_StepSlash,
		MHLongSwordGameplayTags::Move_LS_Thrust,
		MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
	};

	// 시작기(무기 장착 공격 2개)
	EntryMoves_Special =
	{
		MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
		MHLongSwordGameplayTags::Move_LS_StepSlash,
	};

	EntryMoves_Secondary.Reset();

	Nodes.Reset();

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_StepSlash,
		{
			MHLongSwordGameplayTags::Move_LS_VerticalSlash,
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_VerticalSlash,
		{
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_Thrust,
		{
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_RisingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_RisingSlash,
		{
			MHLongSwordGameplayTags::Move_LS_VerticalSlash,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_FallingSlash,
		{
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_SpiritStepSlash,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
		{
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash2,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritSlash2,
		{
			MHLongSwordGameplayTags::Move_LS_RisingSlash,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash3,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritSlash3,
		{
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritRoundslash,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritRoundslash,
		{
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritStepSlash,
		{
			MHLongSwordGameplayTags::Move_LS_RisingSlash,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash3,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpiritThrust,
		{
			MHLongSwordGameplayTags::Move_LS_HelmBreaker,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_HelmBreaker,
		{
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		{
			MHLongSwordGameplayTags::Move_LS_IaiSlash,
			MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_IaiSlash,
		{
			MHLongSwordGameplayTags::Move_LS_VerticalSlash,
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_SpiritSlash1,
			MHLongSwordGameplayTags::Move_LS_ForesightSlash,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_IaiSpiritSlash,
		{
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
		}));

	Nodes.Add(MakeNode(
		MHLongSwordGameplayTags::Move_LS_ForesightSlash,
		{
			MHLongSwordGameplayTags::Move_LS_StepSlash,
			MHLongSwordGameplayTags::Move_LS_FallingSlash,
			MHLongSwordGameplayTags::Move_LS_Thrust,
			MHLongSwordGameplayTags::Move_LS_SpiritThrust,
			MHLongSwordGameplayTags::Move_LS_SpecialSheathe,
		}));

	// Secondary/Special NextMoves는 이번 단계에서 비움
	for (FMHLongSwordComboNode& Node : Nodes)
	{
		Node.SecondaryNextMoves.Reset();
		Node.SpecialNextMoves.Reset();
	}
}
