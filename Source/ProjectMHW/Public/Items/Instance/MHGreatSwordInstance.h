// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponInstance.h"
#include "Items/Data/MHGreatSwordItemData.h"
#include "MHGreatSwordInstance.generated.h"

UCLASS()
class PROJECTMHW_API AMHGreatSwordInstance : public AMHMeleeWeaponInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHGreatSwordInstance();

protected:
	virtual void ApplyItemData() override;
	virtual void BeginPlay() override; 
	
private:
	FORCEINLINE const UMHGreatSwordItemData* GetGreatSwordData() const
	{
		const UMHGreatSwordItemData* Data = Cast<UMHGreatSwordItemData>(CachedItemData);
		ensure(Data);
		return Data;
	}
};
