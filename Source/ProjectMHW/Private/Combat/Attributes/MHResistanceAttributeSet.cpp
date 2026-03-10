// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Attributes/MHResistanceAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMHResistanceAttributeSet::UMHResistanceAttributeSet()
{
	InitFireResist(0.f);
	InitWaterResist(0.f);
	InitThunderResist(0.f);
	InitIceResist(0.f);
	InitDragonResist(0.f);
}

void UMHResistanceAttributeSet::OnRep_FireResist(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHResistanceAttributeSet, FireResist, OldValue);
}

void UMHResistanceAttributeSet::OnRep_WaterResist(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHResistanceAttributeSet, WaterResist, OldValue);
}

void UMHResistanceAttributeSet::OnRep_ThunderResist(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHResistanceAttributeSet, ThunderResist, OldValue);
}

void UMHResistanceAttributeSet::OnRep_IceResist(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHResistanceAttributeSet, IceResist, OldValue);
}

void UMHResistanceAttributeSet::OnRep_DragonResist(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHResistanceAttributeSet, DragonResist, OldValue);
}

void UMHResistanceAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHResistanceAttributeSet, FireResist, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHResistanceAttributeSet, WaterResist, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHResistanceAttributeSet, ThunderResist, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHResistanceAttributeSet, IceResist, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHResistanceAttributeSet, DragonResist, COND_None, REPNOTIFY_Always);
}