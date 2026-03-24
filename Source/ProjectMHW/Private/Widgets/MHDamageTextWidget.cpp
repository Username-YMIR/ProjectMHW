// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MHDamageTextWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Character/MHCharacterBase.h"
#include "Components/TextBlock.h"

UMHDamageTextWidget::UMHDamageTextWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UMHDamageTextWidget::ApplyPayload(const FMHDamageTextPayload& InPayload)
{
    if (!DamageText)
    {
        return;
    }

    DamageText->SetText(FText::AsNumber(FMath::RoundToInt(InPayload.AppliedDamage)));
}

float UMHDamageTextWidget::GetLifetime() const
{
    if (IsValid(DamageWidgetAnimation))
    {
        return DamageWidgetAnimation->GetEndTime();
    }

    return FallbackLifetime;
}

UWidgetAnimation* UMHDamageTextWidget::GetDamageTextAnimation() const
{
    return DamageWidgetAnimation;
}
