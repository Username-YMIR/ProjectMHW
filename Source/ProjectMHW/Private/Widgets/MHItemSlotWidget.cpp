// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MHItemSlotWidget.h"

#include "Components/Image.h"

UMHItemSlotWidget::UMHItemSlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHItemSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshVisuals();
}

void UMHItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshVisuals();
	SetSelected(false);
}

void UMHItemSlotWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	RefreshVisuals();
}

void UMHItemSlotWidget::SetSelected(const bool bSelected)
{
	if (!SelectionOutlineImage)
	{
		return;
	}

	SelectionOutlineImage->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMHItemSlotWidget::SetItemIconBrush(const FSlateBrush& InBrush)
{
	ItemIconBrush = InBrush;
	RefreshVisuals();
}

void UMHItemSlotWidget::RefreshVisuals()
{
	if (ItemIconImage)
	{
		ItemIconImage->SetBrush(ItemIconBrush);
		UpdateItemIconVisibility();
	}
}

void UMHItemSlotWidget::UpdateItemIconVisibility()
{
	if (!ItemIconImage)
	{
		return;
	}

	ItemIconImage->SetVisibility(
		ItemIconBrush.GetResourceObject() ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
