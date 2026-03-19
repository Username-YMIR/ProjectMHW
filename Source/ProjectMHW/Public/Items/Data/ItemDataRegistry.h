// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDataRegistry.generated.h"

/**
 * 
 */
class UMHItemDataBase;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTMHW_API UItemDataRegistry : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const UMHItemDataBase* GetItemData(FName KeyName) const;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly)
	TMap<FName, UMHItemDataBase*> ItemDataMap;
};