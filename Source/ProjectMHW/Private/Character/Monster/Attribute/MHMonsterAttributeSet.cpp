// 제작자 : 허혁
// 제작일 : 2026-03-05
// 수정자 : 이건주
// 수정일 : 2026-03-10


#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMHMonsterAttributeSet::UMHMonsterAttributeSet()
{
	InitRage(0.f);
	InitRageThreshold(100.f);
}

void UMHMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetRageThresholdAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetRageAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetRageThreshold());
	}
}

void UMHMonsterAttributeSet::OnRep_Rage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHMonsterAttributeSet, Rage, OldValue);
}

void UMHMonsterAttributeSet::OnRep_RageThreshold(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHMonsterAttributeSet, RageThreshold, OldValue);
}

void UMHMonsterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHMonsterAttributeSet, Rage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHMonsterAttributeSet, RageThreshold, COND_None, REPNOTIFY_Always);
}

//=================
//Legacy
//=================

// #include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
//
//
// UMHMonsterAttributeSet::UMHMonsterAttributeSet()
// {
// 	
// }
//
// //====================================
// //Attribute 세팅
// //====================================
// void UMHMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
// {
// 	Super::PreAttributeChange(Attribute, NewValue);
// 	
// 	
// 	if (Attribute == GetHealthAttribute())
// 	{
// 		const float MaxHP = FMath::Max(1.f, GetMaxHealth());
// 		NewValue = FMath::Clamp(NewValue, 0.f, MaxHP);
// 	}
// 	else if (Attribute == GetPoiseAttribute())
// 	{
// 		const float MaxP = FMath::Max(1.f, GetMaxPoise());
// 		NewValue = FMath::Clamp(NewValue, 0.f, MaxP);
// 	}
// 	else if (Attribute == GetWeightAttribute())
// 	{
// 		const float MaxW = FMath::Max(1.f, GetMaxWeight());
// 		NewValue = FMath::Clamp(NewValue, 0.f, MaxW);
// 	}
// 	
// }
//
// void UMHMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
// {
// 	Super::PostGameplayEffectExecute(Data);
//
// 	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
// 	{
// 		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
// 	{
// 		if (GetHealth() <= 0.f)
// 		{
// 			SetHealth(GetMaxHealth());
// 		}
// 		else
// 		{
// 			SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
// 		}
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
// 	{
// 		SetPoise(FMath::Clamp(GetPoise(), 0.f, GetMaxPoise()));
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetMaxPoiseAttribute())
// 	{
// 		if (GetPoise() <= 0.f)
// 		{
// 			SetPoise(GetMaxPoise());
// 		}
// 		else
// 		{
// 			SetPoise(FMath::Clamp(GetPoise(), 0.f, GetMaxPoise()));
// 		}
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetWeightAttribute())
// 	{
// 		SetWeight(FMath::Clamp(GetWeight(), 0.f, GetMaxWeight()));
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetMaxWeightAttribute())
// 	{
// 		if (GetWeight() <= 0.f)
// 		{
// 			SetWeight(GetMaxWeight());
// 		}
// 		else
// 		{
// 			SetWeight(FMath::Clamp(GetWeight(), 0.f, GetMaxWeight()));
// 		}
// 	}
// }
