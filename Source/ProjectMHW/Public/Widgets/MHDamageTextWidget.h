// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHDamageTextWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
struct FMHDamageTextPayload;

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UMHDamageTextWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "DamageText")
    void ApplyPayload(const FMHDamageTextPayload& InPayload);

    UFUNCTION(BlueprintCallable, Category = "DamageText")
    float GetLifetime() const;

    UFUNCTION(BlueprintCallable, Category = "DamageText")
    UWidgetAnimation* GetDamageTextAnimation() const;
	
protected:
	// 대미지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> DamageText = nullptr;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional), BlueprintReadOnly)
    TObjectPtr<UWidgetAnimation> DamageWidgetAnimation = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageText")
    float FallbackLifetime = 2.0f;
};
