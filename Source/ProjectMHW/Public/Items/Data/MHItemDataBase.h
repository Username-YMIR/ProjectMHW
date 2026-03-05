// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "MHItemDataBase.generated.h"

/**
 * 
 */
UCLASS(DefaultToInstanced, EditInlineNew, BlueprintType, Blueprintable)
class PROJECTMHW_API UMHItemDataBase : public UObject
{
	GENERATED_BODY()
public:
	UMHItemDataBase();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	// 아이콘은 Soft로 두는 게 DT에서 안전함(참조로 인한 패키징/로딩 이슈 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine="true"))
	FText Description;

	// 희귀도 등급 1~12
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="12"))
	int Rarity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 SellPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 BuyPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTag ItemTag;
};