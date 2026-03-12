#include "Combat/Editor/MHCombatDataPopulatorLibrary.h"

#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Input/DataAsset_LSInputPatternSet.h"
#include "Combat/Input/MHCombatInputTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"

// 손승우 추가: 태도 입력 패턴 DA / 공격 메타 DT 자동 채우기 구현

namespace MHCombatDataPopulatorLibrary_Private
{
	static FGameplayTag RequestTagChecked(const TCHAR* InTagName)
	{
		return UGameplayTagsManager::Get().RequestGameplayTag(FName(InTagName), false);
	}

	static FMHInputPatternDefinition MakePattern(
		const TCHAR* InPatternTag,
		const TCHAR* InDisplayName,
		const TArray<EMHCombatInputButton>& InRequiredButtons,
		int32 InPriority,
		const TCHAR* InAllowedWeaponTypeTag,
		const TCHAR* InAllowedSheathStateTag,
		const TCHAR* InAllowedCombatStateTag,
		bool bInRequireStandingStill,
		bool bInRequireMoveInput,
		bool bInAllowMoving,
		bool bInRequireSimultaneous = false,
		float InMaxSimultaneousGap = 0.12f,
		bool bInAllowHold = false,
		float InMinHoldTime = 0.0f)
	{
		FMHInputPatternDefinition PatternDefinition;
		PatternDefinition.PatternTag = RequestTagChecked(InPatternTag);
		PatternDefinition.DisplayName = FText::FromString(InDisplayName);
		PatternDefinition.RequiredButtons = InRequiredButtons;
		PatternDefinition.OptionalButtons.Reset();
		PatternDefinition.bRequireSimultaneous = bInRequireSimultaneous;
		PatternDefinition.MaxSimultaneousGap = InMaxSimultaneousGap;
		PatternDefinition.bAllowHold = bInAllowHold;
		PatternDefinition.MinHoldTime = InMinHoldTime;
		PatternDefinition.Priority = InPriority;
		PatternDefinition.AllowedWeaponTypeTag = RequestTagChecked(InAllowedWeaponTypeTag);
		PatternDefinition.AllowedSheathStateTag = RequestTagChecked(InAllowedSheathStateTag);
		PatternDefinition.AllowedCombatStateTag = RequestTagChecked(InAllowedCombatStateTag);
		PatternDefinition.bRequireStandingStill = bInRequireStandingStill;
		PatternDefinition.bRequireMoveInput = bInRequireMoveInput;
		PatternDefinition.bAllowMoving = bInAllowMoving;
		return PatternDefinition;
	}

	static FName MakeRowNameFromTag(const TCHAR* InMoveTag)
	{
		FString RowNameString(InMoveTag);
		RowNameString.ReplaceInline(TEXT("."), TEXT("_"));
		return FName(*RowNameString);
	}

	static void AddAttackMetaRow(
		UDataTable* InAttackMetaTable,
		const TCHAR* InMoveTag,
		float InDamageMultiplier,
		float InPartBreakMultiplier,
		float InHitStopSeconds,
		const TCHAR* InAttackType,
		bool bInCanSever = true)
	{
		if (!IsValid(InAttackMetaTable))
		{
			return;
		}

		FMHAttackMetaRow AttackMetaRow;
		AttackMetaRow.MoveTag = RequestTagChecked(InMoveTag);
		AttackMetaRow.DamageMultiplier = InDamageMultiplier;
		AttackMetaRow.PartBreakMultiplier = InPartBreakMultiplier;
		AttackMetaRow.HitStopSeconds = InHitStopSeconds;
		AttackMetaRow.AttackType = FName(InAttackType);
		AttackMetaRow.bCanSever = bInCanSever;

		InAttackMetaTable->AddRow(MakeRowNameFromTag(InMoveTag), AttackMetaRow);
	}
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordInputPatternSet(UObject* InputPatternAsset)
{
	UDataAsset_LSInputPatternSet* InputPatternSet = Cast<UDataAsset_LSInputPatternSet>(InputPatternAsset);
	if (!IsValid(InputPatternSet))
	{
		return false;
	}

	using namespace MHCombatDataPopulatorLibrary_Private;

	TArray<FMHInputPatternDefinition> PatternDefinitions;

	// ===== Draw =====
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.DrawOnly"), TEXT("LS Draw Only"), { EMHCombatInputButton::LMB }, 10, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Sheathed"), TEXT("CombatState.None"), true, false, false));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.DrawAdvancingSlash"), TEXT("LS Draw Advancing Slash"), { EMHCombatInputButton::LMB }, 20, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Sheathed"), TEXT("CombatState.None"), false, true, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.DrawSpiritSlash1"), TEXT("LS Draw Spirit Slash 1"), { EMHCombatInputButton::Mouse4 }, 30, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Sheathed"), TEXT("CombatState.None"), false, false, true));
	// ===== End Draw =====

	// ===== Unsheathed =====
	// 좌클릭은 현재 콤보 문맥에 따라 실제 일반 공격 패턴으로 해석된다.
	// 시작 공격 / 파생 공격에서 좌클릭이 어떤 기술이 되는지는 Player 입력 해석과 ComboGraph가 함께 결정한다.
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.AdvancingSlash"), TEXT("LS Advancing Slash"), { EMHCombatInputButton::LMB }, 40, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.VerticalSlash"), TEXT("LS Vertical Slash"), { EMHCombatInputButton::LMB }, 41, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.RisingSlash"), TEXT("LS Rising Slash"), { EMHCombatInputButton::LMB }, 42, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.Thrust"), TEXT("LS Thrust"), { EMHCombatInputButton::RMB }, 50, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.Spirit"), TEXT("LS Spirit"), { EMHCombatInputButton::Mouse4 }, 60, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));

	// 실제 입력 키 txt 기준으로 4번/5번은 서로 치환해서 읽는다.
	// 따라서 베어내리기 계열의 실제 단축 입력은 Mouse5다.
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.FadeSlash"), TEXT("LS Fade Slash"), { EMHCombatInputButton::Mouse5 }, 70, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	// 좌우이동베기는 시작 공격이 아니라 후속 파생에서만 허용되는 패턴이다.
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.LateralFadeSlash"), TEXT("LS Lateral Fade Slash"), { EMHCombatInputButton::Mouse5 }, 71, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, true, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.SpiritThrust"), TEXT("LS Spirit Thrust"), { EMHCombatInputButton::Mouse4, EMHCombatInputButton::LMB }, 80, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.ForesightSlash"), TEXT("LS Foresight Slash"), { EMHCombatInputButton::Mouse4, EMHCombatInputButton::RMB }, 90, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.SpecialSheathe"), TEXT("LS Special Sheathe"), { EMHCombatInputButton::Mouse4, EMHCombatInputButton::Space }, 100, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true, true));
	// ===== End Unsheathed =====

	// ===== Special Sheathe =====
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.IaiSlash"), TEXT("LS Iai Slash"), { EMHCombatInputButton::LMB }, 110, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.SpecialSheathe"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.IaiSpiritSlash"), TEXT("LS Iai Spirit Slash"), { EMHCombatInputButton::Mouse4 }, 120, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.SpecialSheathe"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.Helmbreaker"), TEXT("LS Helmbreaker"), { EMHCombatInputButton::LMB }, 130, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	// ===== End Special Sheathe =====

	InputPatternSet->ReplacePatternDefinitions(PatternDefinitions);
	InputPatternSet->MarkPackageDirty();
	return true;
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordAttackMetaTable(UObject* AttackMetaTableAsset)
{
	UDataTable* AttackMetaTable = Cast<UDataTable>(AttackMetaTableAsset);
	if (!IsValid(AttackMetaTable))
	{
		return false;
	}

	if (AttackMetaTable->GetRowStruct() != FMHAttackMetaRow::StaticStruct())
	{
		return false;
	}

	using namespace MHCombatDataPopulatorLibrary_Private;

	AttackMetaTable->EmptyTable();

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawOnly"), 0.0f, 0.0f, 0.00f, TEXT("Utility"), false);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawAdvancingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Draw"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawSpiritSlash1"), 1.05f, 1.05f, 0.05f, TEXT("DrawSpirit"));

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.AdvancingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.VerticalSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.Thrust"), 0.95f, 0.95f, 0.03f, TEXT("Normal"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.RisingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.FadeSlash"), 0.95f, 0.95f, 0.04f, TEXT("Normal"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.LateralFadeSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"));

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash1"), 1.05f, 1.05f, 0.05f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash2"), 1.10f, 1.10f, 0.05f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash3"), 1.15f, 1.10f, 0.06f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritRoundslash"), 1.25f, 1.20f, 0.07f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritAdvancingSlash"), 1.10f, 1.05f, 0.05f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritThrust"), 1.00f, 1.00f, 0.04f, TEXT("Spirit"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritHelmbreaker"), 2.00f, 1.50f, 0.10f, TEXT("SpiritFinisher"));

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpecialSheathe"), 0.0f, 0.0f, 0.00f, TEXT("Utility"), false);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.IaiSlash"), 1.30f, 1.20f, 0.06f, TEXT("Special"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.IaiSpiritSlash"), 1.60f, 1.30f, 0.08f, TEXT("Special"));
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.ForesightSlash"), 0.95f, 1.00f, 0.05f, TEXT("Counter"));

	AttackMetaTable->MarkPackageDirty();
	return true;
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordCombatData(UObject* InputPatternAsset, UObject* AttackMetaTableAsset)
{
	const bool bInputPatternResult = PopulateLongSwordInputPatternSet(InputPatternAsset);
	const bool bAttackMetaResult = PopulateLongSwordAttackMetaTable(AttackMetaTableAsset);
	return bInputPatternResult && bAttackMetaResult;
}
