// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/Data/ItemDataRegistry.h"
#include "MHItemInstanceBase.generated.h"

class UMHItemDataBase;
class UItemDataRegistry;

UCLASS()
class PROJECTMHW_API AMHItemInstanceBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHItemInstanceBase();

	
	FORCEINLINE const UMHItemDataBase* GetItemData() const
	{
		return CachedItemData;
	}
protected:
	virtual void BeginPlay() override;
	virtual void ApplyItemData();
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, Category="Item")
	FName ItemDataKey;

	/** Registry 참조 */
	UPROPERTY(EditAnywhere, Category="Item")
	TObjectPtr<UItemDataRegistry> ItemRegistry;

	/** 캐싱된 데이터 */
	UPROPERTY(Transient)
	TObjectPtr<UMHItemDataBase> CachedItemData;


};
