// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponInstance.h"
#include "Items/Data/MHGreatSwordItemData.h"
#include "MHGreatSwordInstance.generated.h"

class UMHGreatSwordActionComponent;

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

#pragma region Components
	// 대검 입력 상태와 차징 상태를 관리하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|GreatSword", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHGreatSwordActionComponent> ActionComponent;
#pragma endregion

public:
	UMHGreatSwordActionComponent* GetActionComponent() const { return ActionComponent; }

private:
	FORCEINLINE const UMHGreatSwordItemData* GetGreatSwordData() const
	{
		const UMHGreatSwordItemData* Data = Cast<UMHGreatSwordItemData>(CachedItemData);
		ensure(Data);
		return Data;
	}
};
