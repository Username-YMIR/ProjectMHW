// 제작자 : 허혁
// 제작일 : 2026-03-05
// 수정자 : 허혁
// 수정일 : 2026-03-05



#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "MHMonsterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)



/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UMHMonsterAttributeSet();
	
	//=====
	// 체력 
	//=====
	
	UPROPERTY(BlueprintReadOnly, Category="Vitals")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , MaxHealth)
	
	//=====
	// 전투 스텟
	//=====
	
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , AttackPower)
	
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , Defense)
	
	//=====
	// 그로기 게이지 / 가중치
	//=====
	
	//그로기 게이지
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , Poise)
	
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , MaxPoise)
	
	// 가중치 
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData Weight;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , Weight)
	
	UPROPERTY(BlueprintReadOnly , Category="Status")
	FGameplayAttributeData MaxWeight;
	ATTRIBUTE_ACCESSORS(UMHMonsterAttributeSet , MaxWeight)
	
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
