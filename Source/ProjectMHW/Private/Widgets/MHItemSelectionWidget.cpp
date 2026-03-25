// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MHItemSelectionWidget.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Widgets/MHItemSlotWidget.h"

UMHItemSelectionWidget::UMHItemSelectionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHItemSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayerCharacter();
	SyncInitialSelection();
}

void UMHItemSelectionWidget::NativeDestruct()
{
	UnbindFromPlayerCharacter();

	Super::NativeDestruct();
}

void UMHItemSelectionWidget::HandleWidgetInitialized()
{
	Super::HandleWidgetInitialized();

	BindToPlayerCharacter();
}

void UMHItemSelectionWidget::BindToPlayerCharacter()
{
	if (CachedPlayerCharacter.IsValid())
	{
		return;
	}

	AMHPlayerCharacter* PlayerCharacter = Cast<AMHPlayerCharacter>(GetMHPawn());
	if (!PlayerCharacter)
	{
		return;
	}

	CachedPlayerCharacter = PlayerCharacter;
	PlayerCharacter->OnConsumableSelectionChanged.AddDynamic(this, &ThisClass::HandleConsumableSelectionChanged);
}

void UMHItemSelectionWidget::UnbindFromPlayerCharacter()
{
	if (!CachedPlayerCharacter.IsValid())
	{
		return;
	}

	CachedPlayerCharacter->OnConsumableSelectionChanged.RemoveDynamic(this, &ThisClass::HandleConsumableSelectionChanged);
	CachedPlayerCharacter.Reset();
}

void UMHItemSelectionWidget::SyncInitialSelection()
{
	AMHPlayerCharacter* PlayerCharacter = CachedPlayerCharacter.Get();
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AMHPlayerCharacter>(GetMHPawn());
	}

	if (!PlayerCharacter)
	{
		return;
	}

	RefreshSelection(PlayerCharacter->GetSelectedConsumable());
}

void UMHItemSelectionWidget::HandleConsumableSelectionChanged(const EMHConsumableSelection NewSelection)
{
	RefreshSelection(NewSelection);
}

void UMHItemSelectionWidget::RefreshSelection(const EMHConsumableSelection InSelection)
{
	if (SharpenSlot)
	{
		SharpenSlot->SetSelected(InSelection == EMHConsumableSelection::Sharpen);
	}

	if (PotionSlot)
	{
		PotionSlot->SetSelected(InSelection == EMHConsumableSelection::Potion);
	}
}
