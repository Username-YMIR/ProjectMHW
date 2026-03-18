#include "Combat/Editor/MHCombatDataPopulatorLibrary.h"

#include "Combat/Data/MHAttackMetaTypes.h"
#include "Combat/Input/DataAsset_LSInputPatternSet.h"
#include "Combat/Input/MHCombatInputTypes.h"
#include "Combat/mh_attack_definition_types.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"

// 손승우 추가: 태도 입력 패턴 DA / 공격 메타 DT / 공격 정의 DT 자동 채우기 구현

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

	static FName MakeRowNameFromTag(const TCHAR* InTagName)
	{
		FString RowNameString(InTagName);
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
		float InSpiritGaugeGain = 0.0f,
		float InSpiritGaugeConsume = 0.0f,
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
		AttackMetaRow.SpiritGaugeGain = InSpiritGaugeGain;
		AttackMetaRow.SpiritGaugeConsume = InSpiritGaugeConsume;
		AttackMetaRow.bCanSever = bInCanSever;

		InAttackMetaTable->AddRow(MakeRowNameFromTag(InMoveTag), AttackMetaRow);
	}

	static void AddAttackDefinitionRow(
		UDataTable* InAttackDefinitionTable,
		const TCHAR* InAttackTag,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		float InPhysicalScale,
		float InFireScale,
		float InWaterScale,
		float InThunderScale,
		float InIceScale,
		float InDragonScale,
		bool bInCanBeForesightCountered,
		bool bInCanBeSpecialSheatheCountered)
	{
		if (!IsValid(InAttackDefinitionTable))
		{
			return;
		}

		FMHAttackDefinitionRow AttackDefinitionRow;
		AttackDefinitionRow.AttackTag = RequestTagChecked(InAttackTag);
		AttackDefinitionRow.DamageEffectClass = InDamageEffectClass;
		AttackDefinitionRow.PhysicalScale = InPhysicalScale;
		AttackDefinitionRow.FireScale = InFireScale;
		AttackDefinitionRow.WaterScale = InWaterScale;
		AttackDefinitionRow.ThunderScale = InThunderScale;
		AttackDefinitionRow.IceScale = InIceScale;
		AttackDefinitionRow.DragonScale = InDragonScale;
		AttackDefinitionRow.bCanBeForesightCountered = bInCanBeForesightCountered;
		AttackDefinitionRow.bCanBeSpecialSheatheCountered = bInCanBeSpecialSheatheCountered;

		InAttackDefinitionTable->AddRow(MakeRowNameFromTag(InAttackTag), AttackDefinitionRow);
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
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.AdvancingSlash"), TEXT("LS Advancing Slash"), { EMHCombatInputButton::LMB }, 40, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.VerticalSlash"), TEXT("LS Vertical Slash"), { EMHCombatInputButton::LMB }, 41, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.RisingSlash"), TEXT("LS Rising Slash"), { EMHCombatInputButton::LMB }, 42, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.Thrust"), TEXT("LS Thrust"), { EMHCombatInputButton::RMB }, 50, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.Spirit"), TEXT("LS Spirit"), { EMHCombatInputButton::Mouse4 }, 60, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
	PatternDefinitions.Add(MakePattern(TEXT("InputPattern.LS.FadeSlash"), TEXT("LS Fade Slash"), { EMHCombatInputButton::Mouse5 }, 70, TEXT("WeaponType.LongSword"), TEXT("WeaponSheath.Unsheathed"), TEXT("CombatState.None"), false, false, true));
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

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawOnly"), 0.0f, 0.0f, 0.00f, TEXT("Utility"), 0.0f, 0.0f, false);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawAdvancingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Draw"), 8.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.DrawSpiritSlash1"), 1.05f, 1.05f, 0.05f, TEXT("DrawSpirit"), 10.0f, 0.0f);

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.AdvancingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"), 10.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.VerticalSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"), 8.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.Thrust"), 0.95f, 0.95f, 0.03f, TEXT("Normal"), 7.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.RisingSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"), 9.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.FadeSlash"), 0.95f, 0.95f, 0.04f, TEXT("Normal"), 8.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.LateralFadeSlash"), 1.00f, 1.00f, 0.04f, TEXT("Normal"), 8.0f, 0.0f);

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash1"), 1.05f, 1.05f, 0.05f, TEXT("Spirit"), 0.0f, 10.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash2"), 1.10f, 1.10f, 0.05f, TEXT("Spirit"), 0.0f, 10.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritSlash3"), 1.15f, 1.10f, 0.06f, TEXT("Spirit"), 0.0f, 10.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritRoundslash"), 1.25f, 1.20f, 0.07f, TEXT("Spirit"), 0.0f, 10.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritAdvancingSlash"), 1.10f, 1.05f, 0.05f, TEXT("Spirit"), 0.0f, 10.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritThrust"), 1.00f, 1.00f, 0.04f, TEXT("Spirit"), 5.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpiritHelmbreaker"), 2.00f, 1.50f, 0.10f, TEXT("SpiritFinisher"), 0.0f, 0.0f);

	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.SpecialSheathe"), 0.0f, 0.0f, 0.00f, TEXT("Utility"), 0.0f, 0.0f, false);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.IaiSlash"), 1.30f, 1.20f, 0.06f, TEXT("Special"), 8.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.IaiSpiritSlash"), 1.60f, 1.30f, 0.08f, TEXT("Special"), 0.0f, 0.0f);
	AddAttackMetaRow(AttackMetaTable, TEXT("Move.LS.ForesightSlash"), 0.95f, 1.00f, 0.05f, TEXT("Counter"), 0.0f, 10.0f);

	AttackMetaTable->MarkPackageDirty();
	return true;
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordAttackDefinitionTable(UObject* AttackDefinitionTableAsset, TSubclassOf<UGameplayEffect> DefaultDamageEffectClass)
{
	UDataTable* AttackDefinitionTable = Cast<UDataTable>(AttackDefinitionTableAsset);
	if (!IsValid(AttackDefinitionTable))
	{
		return false;
	}

	if (AttackDefinitionTable->GetRowStruct() != FMHAttackDefinitionRow::StaticStruct())
	{
		return false;
	}

	using namespace MHCombatDataPopulatorLibrary_Private;

	AttackDefinitionTable->EmptyTable();

	// ===== LongSword Player Attacks =====
	// 현재 프로젝트는 MoveTag를 사실상 공격 식별자로 사용 중이므로 1차 자동 채움은 MoveTag 기반으로 맞춘다.
	// 추후 Attack.LongSword.* 태그로 분리하더라도 이 함수만 수정하면 된다.
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.DrawOnly"), DefaultDamageEffectClass, 0.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.DrawAdvancingSlash"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.DrawSpiritSlash1"), DefaultDamageEffectClass, 1.05f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);

	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.AdvancingSlash"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.VerticalSlash"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.Thrust"), DefaultDamageEffectClass, 0.95f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.RisingSlash"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.FadeSlash"), DefaultDamageEffectClass, 0.95f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.LateralFadeSlash"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);

	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritSlash1"), DefaultDamageEffectClass, 1.05f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritSlash2"), DefaultDamageEffectClass, 1.10f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritSlash3"), DefaultDamageEffectClass, 1.15f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritRoundslash"), DefaultDamageEffectClass, 1.25f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritAdvancingSlash"), DefaultDamageEffectClass, 1.10f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritThrust"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpiritHelmbreaker"), DefaultDamageEffectClass, 2.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);

	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.SpecialSheathe"), DefaultDamageEffectClass, 0.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.IaiSlash"), DefaultDamageEffectClass, 1.30f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.IaiSpiritSlash"), DefaultDamageEffectClass, 1.60f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Move.LS.ForesightSlash"), DefaultDamageEffectClass, 0.95f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);

	// ===== Debug / Test Incoming Attacks =====
	// 플레이어 카운터 테스트용 기본 Row
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Attack.Debug.Generic"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, true, true);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Attack.Debug.Counterable"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, true, true);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Attack.Debug.Uncounterable"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Attack.Monster.Test.Counterable"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, true, true);
	AddAttackDefinitionRow(AttackDefinitionTable, TEXT("Attack.Monster.Test.Uncounterable"), DefaultDamageEffectClass, 1.00f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, false, false);

	AttackDefinitionTable->MarkPackageDirty();
	return true;
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordCombatData(UObject* InputPatternAsset, UObject* AttackMetaTableAsset)
{
	const bool bInputPatternResult = PopulateLongSwordInputPatternSet(InputPatternAsset);
	const bool bAttackMetaResult = PopulateLongSwordAttackMetaTable(AttackMetaTableAsset);
	return bInputPatternResult && bAttackMetaResult;
}

bool UMHCombatDataPopulatorLibrary::PopulateLongSwordCombatDataExtended(UObject* InputPatternAsset, UObject* AttackMetaTableAsset, UObject* AttackDefinitionTableAsset, TSubclassOf<UGameplayEffect> DefaultDamageEffectClass)
{
	const bool bInputPatternResult = PopulateLongSwordInputPatternSet(InputPatternAsset);
	const bool bAttackMetaResult = PopulateLongSwordAttackMetaTable(AttackMetaTableAsset);
	const bool bAttackDefinitionResult = PopulateLongSwordAttackDefinitionTable(AttackDefinitionTableAsset, DefaultDamageEffectClass);
	return bInputPatternResult && bAttackMetaResult && bAttackDefinitionResult;
}
