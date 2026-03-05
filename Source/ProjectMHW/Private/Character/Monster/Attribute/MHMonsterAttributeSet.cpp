// 제작자 : 허혁
// 제작일 : 2026-03-05
// 수정자 : 허혁
// 수정일 : 2026-03-05


#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"


UMHMonsterAttributeSet::UMHMonsterAttributeSet()
{
	
}

//====================================
//Attribute 세팅
//====================================
void UMHMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxPoise());
	}
	else if (Attribute == GetWeightAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxWeight());
	}
	
}

void UMHMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetPoiseAttribute())
	{
		SetPoise(FMath::Clamp(GetPoise(), 0.f, GetMaxPoise()));
	}
	else if (Data.EvaluatedData.Attribute == GetWeightAttribute())
	{
		SetWeight(FMath::Clamp(GetWeight(), 0.f, GetMaxWeight()));
	}
	
}
