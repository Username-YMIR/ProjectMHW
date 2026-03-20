// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Attributes/MHHealthAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMHHealthAttributeSet::UMHHealthAttributeSet()
{
	InitMaxHealth(100.f);
	InitHealth(100.f);

	InitIncomingDamage(0.f);
	InitIncomingHeal(0.f);
}

void UMHHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

}

void UMHHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	/* 대미지 처리 */

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// IncomingDamage 값을 복제
		float Damage = GetIncomingDamage();
		// 그 후 IncomingDamage을 0으로 초기화 
		SetIncomingDamage(0.f);

		// 대미지를 Health에 적용
		if (Damage > 0.f)
		{
			const float NewHealth = GetHealth() - Damage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}

	/* 회복 처리 */

	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		float Heal = GetIncomingHeal();

		SetIncomingHeal(0.f);

		if (Heal > 0.f)
		{
			const float NewHealth = GetHealth() + Heal;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}
	}

	/* 사망 체크 */

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (GetHealth() <= 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Actor Died"));
		}
	}
}

void UMHHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHHealthAttributeSet, Health, OldValue);
}

void UMHHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMHHealthAttributeSet, MaxHealth, OldValue);
}

void UMHHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMHHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMHHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}