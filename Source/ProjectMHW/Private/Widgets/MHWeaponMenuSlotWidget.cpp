#include "Widgets/MHWeaponMenuSlotWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Widgets/MHItemSlotWidget.h"

void UMHWeaponMenuSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &ThisClass::HandleSlotButtonClicked);
	}

	SetWeaponName(WeaponName);
	SetSelected(bIsSelected);
}

void UMHWeaponMenuSlotWidget::NativeDestruct()
{
	if (SlotButton)
	{
		SlotButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleSlotButtonClicked);
	}

	Super::NativeDestruct();
}

void UMHWeaponMenuSlotWidget::SetWeaponName(const FText& InWeaponName)
{
	WeaponName = InWeaponName;

	if (WeaponNameText)
	{
		WeaponNameText->SetText(WeaponName);
	}
}

void UMHWeaponMenuSlotWidget::SetItemIconBrush(const FSlateBrush& InBrush)
{
	if (ItemSlotWidget)
	{
		ItemSlotWidget->SetItemIconBrush(InBrush);
	}
}

void UMHWeaponMenuSlotWidget::SetSelected(const bool bSelected)
{
	bIsSelected = bSelected;

	if (ItemSlotWidget)
	{
		ItemSlotWidget->SetSelected(bIsSelected);
	}

	RefreshWeaponNameVisuals();
}

void UMHWeaponMenuSlotWidget::HandleSlotButtonClicked()
{
	OnWeaponMenuSlotClicked.Broadcast(this);
}

void UMHWeaponMenuSlotWidget::RefreshWeaponNameVisuals() const
{
	if (!WeaponNameText)
	{
		return;
	}

	WeaponNameText->SetColorAndOpacity(bIsSelected ? SelectedTextColor : UnselectedTextColor);
}
