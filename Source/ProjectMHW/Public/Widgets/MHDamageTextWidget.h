// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHDamageTextWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMHDamageTextWidget();
	
protected:
	// 대미지 텍스트
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> DamageText = nullptr;
};
