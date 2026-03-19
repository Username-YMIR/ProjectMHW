// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Execution/MHDamageExecutionCalculation.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

#include "Combat/Attributes/MHCombatAttributeSet.h"
#include "Combat/Attributes/MHResistanceAttributeSet.h"
#include "Combat/Attributes/MHHealthAttributeSet.h"
#include "MHGameplayTags.h"

namespace MHDamageStatics
{
	struct FDamageCaptureDefs
	{
		// Source
		FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
		FGameplayEffectAttributeCaptureDefinition CriticalRateDef;
		FGameplayEffectAttributeCaptureDefinition SharpnessModifierDef;

		// Target
		FGameplayEffectAttributeCaptureDefinition DefenseDef;
		FGameplayEffectAttributeCaptureDefinition FireResistDef;
		FGameplayEffectAttributeCaptureDefinition WaterResistDef;
		FGameplayEffectAttributeCaptureDefinition ThunderResistDef;
		FGameplayEffectAttributeCaptureDefinition IceResistDef;
		FGameplayEffectAttributeCaptureDefinition DragonResistDef;

		FDamageCaptureDefs()
		{
			AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
				UMHCombatAttributeSet::GetAttackPowerAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false
			);

			CriticalRateDef = FGameplayEffectAttributeCaptureDefinition(
				UMHCombatAttributeSet::GetCriticalRateAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false
			);

			SharpnessModifierDef = FGameplayEffectAttributeCaptureDefinition(
				UMHCombatAttributeSet::GetSharpnessModifierAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false
			);

			DefenseDef = FGameplayEffectAttributeCaptureDefinition(
				UMHCombatAttributeSet::GetDefenseAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);

			FireResistDef = FGameplayEffectAttributeCaptureDefinition(
				UMHResistanceAttributeSet::GetFireResistAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);

			WaterResistDef = FGameplayEffectAttributeCaptureDefinition(
				UMHResistanceAttributeSet::GetWaterResistAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);

			ThunderResistDef = FGameplayEffectAttributeCaptureDefinition(
				UMHResistanceAttributeSet::GetThunderResistAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);

			IceResistDef = FGameplayEffectAttributeCaptureDefinition(
				UMHResistanceAttributeSet::GetIceResistAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);

			DragonResistDef = FGameplayEffectAttributeCaptureDefinition(
				UMHResistanceAttributeSet::GetDragonResistAttribute(),
				EGameplayEffectAttributeCaptureSource::Target,
				false
			);
		}
	};

	static const FDamageCaptureDefs& Get()
	{
		static FDamageCaptureDefs Statics;
		return Statics;
	}
}

UMHDamageExecutionCalculation::UMHDamageExecutionCalculation()
{
	const MHDamageStatics::FDamageCaptureDefs& Statics = MHDamageStatics::Get();

	RelevantAttributesToCapture.Add(Statics.AttackPowerDef);
	RelevantAttributesToCapture.Add(Statics.CriticalRateDef);
	RelevantAttributesToCapture.Add(Statics.SharpnessModifierDef);

	RelevantAttributesToCapture.Add(Statics.DefenseDef);
	RelevantAttributesToCapture.Add(Statics.FireResistDef);
	RelevantAttributesToCapture.Add(Statics.WaterResistDef);
	RelevantAttributesToCapture.Add(Statics.ThunderResistDef);
	RelevantAttributesToCapture.Add(Statics.IceResistDef);
	RelevantAttributesToCapture.Add(Statics.DragonResistDef);
}

void UMHDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	const MHDamageStatics::FDamageCaptureDefs& Statics = MHDamageStatics::Get();

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackPower = 0.f;
	float CriticalRate = 0.f;
	float SharpnessModifier = 1.f;
	float Defense = 0.f;

	float FireResist = 0.f;
	float WaterResist = 0.f;
	float ThunderResist = 0.f;
	float IceResist = 0.f;
	float DragonResist = 0.f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.AttackPowerDef, EvalParams, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.CriticalRateDef, EvalParams, CriticalRate);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.SharpnessModifierDef, EvalParams, SharpnessModifier);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.DefenseDef, EvalParams, Defense);

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.FireResistDef, EvalParams, FireResist);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.WaterResistDef, EvalParams, WaterResist);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.ThunderResistDef, EvalParams, ThunderResist);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.IceResistDef, EvalParams, IceResist);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.DragonResistDef, EvalParams, DragonResist);

	AttackPower = FMath::Max(0.f, AttackPower);
	CriticalRate = FMath::Clamp(CriticalRate, 0.f, 1.f);
	SharpnessModifier = FMath::Max(0.f, SharpnessModifier);
	Defense = FMath::Max(0.f, Defense);

	// SetByCaller 입력값
	const float BasePhysicalDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Physical, false, 0.f);
	const float BaseFireDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Fire, false, 0.f);
	const float BaseWaterDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Water, false, 0.f);
	const float BaseThunderDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Thunder, false, 0.f);
	const float BaseIceDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Ice, false, 0.f);
	const float BaseDragonDamage = Spec.GetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Dragon, false, 0.f);

	// 물리 대미지 계산
	const float PhysicalDamage = CalculatePhysicalDamage(
		BasePhysicalDamage,
		AttackPower,
		SharpnessModifier,
		Defense
	);

	// 속성 대미지 계산
	const float FireDamage = CalculateElementDamage(BaseFireDamage, FireResist);
	const float WaterDamage = CalculateElementDamage(BaseWaterDamage, WaterResist);
	const float ThunderDamage = CalculateElementDamage(BaseThunderDamage, ThunderResist);
	const float IceDamage = CalculateElementDamage(BaseIceDamage, IceResist);
	const float DragonDamage = CalculateElementDamage(BaseDragonDamage, DragonResist);

	float FinalDamage =
		PhysicalDamage +
		FireDamage +
		WaterDamage +
		ThunderDamage +
		IceDamage +
		DragonDamage;

	// 회심은 우선 전체 합산 후 적용
	FinalDamage = ApplyCritical(FinalDamage, CriticalRate);
	FinalDamage = FMath::Max(0.f, FinalDamage);

	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UMHHealthAttributeSet::GetIncomingDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage
			)
		);
	}
	
	// 최종 대미지 로그 _이건주
	UE_LOG(LogTemp, Warning, TEXT("[DamageExec] AP=%.2f CR=%.2f Sharp=%.2f BasePhys=%.2f Final=%.2f"),
	AttackPower,
	CriticalRate,
	SharpnessModifier,
	BasePhysicalDamage,
	FinalDamage);
}

float UMHDamageExecutionCalculation::CalculatePhysicalDamage(
	float InBasePhysicalDamage,
	float InAttackPower,
	float InSharpnessModifier,
	float InDefense
) const
{
	// 기본 공식 예시:
	// (기본 공격값 + 공격력) * 예리도보정 * 방어보정
	const float RawDamage = (InBasePhysicalDamage + InAttackPower) * InSharpnessModifier;

	// 간단한 방어식
	const float DefenseMultiplier = 100.f / (100.f + InDefense);

	return FMath::Max(0.f, RawDamage * DefenseMultiplier);
}

float UMHDamageExecutionCalculation::CalculateElementDamage(
	float InBaseElementDamage,
	float InResist
) const
{
	// 내성은 0~100 기준으로 가정
	const float ClampedResist = FMath::Clamp(InResist, 0.f, 100.f);
	const float ResistMultiplier = 1.f - (ClampedResist / 100.f);

	return FMath::Max(0.f, InBaseElementDamage * ResistMultiplier);
}

float UMHDamageExecutionCalculation::ApplyCritical(
	float InDamage,
	float InCriticalRate
) const
{
	if (InDamage <= 0.f)
	{
		return 0.f;
	}

	const bool bIsCritical = FMath::FRand() <= InCriticalRate;
	const float CritMultiplier = 1.25f;

	return bIsCritical ? InDamage * CritMultiplier : InDamage;
}