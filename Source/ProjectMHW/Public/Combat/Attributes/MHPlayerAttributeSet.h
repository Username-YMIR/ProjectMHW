// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "MHAttributeSetMacro.h"
#include "MHPlayerAttributeSet.generated.h"

UCLASS()
class PROJECTMHW_API UMHPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMHPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	UPROPERTY(BlueprintReadOnly, Category="Attributes|Player", ReplicatedUsing=OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UMHPlayerAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category="Attributes|Player", ReplicatedUsing=OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UMHPlayerAttributeSet, MaxStamina)

protected:
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
};