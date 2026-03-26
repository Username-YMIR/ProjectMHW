// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MHItemSlotWidget.h"

#include "Components/Button.h"
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

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &ThisClass::HandleSlotButtonClicked);
	}

	RefreshVisuals();
	SetSelected(false);
}

void UMHItemSlotWidget::NativeDestruct()
{
	if (SlotButton)
	{
		SlotButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleSlotButtonClicked);
	}

	Super::NativeDestruct();
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

void UMHItemSlotWidget::HandleSlotButtonClicked()
{
	OnItemSlotClicked.Broadcast(this);
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
