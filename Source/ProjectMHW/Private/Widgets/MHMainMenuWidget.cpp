#include "Widgets/MHMainMenuWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Items/Data/ItemDataRegistry.h"
#include "Items/Data/MHItemDataBase.h"
#include "Items/Instance/MHItemInstanceBase.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "PaperSprite.h"
#include "Widgets/MHWeaponMenuSlotWidget.h"
#include "Widgets/MHWeaponStatPanelWidget.h"

void UMHMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::HandleStartButtonClicked);
		StartButton->OnHovered.AddDynamic(this, &ThisClass::HandleStartButtonHovered);
		StartButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleStartButtonUnhovered);
	}

	BuildWeaponSlots();
	RefreshStartButtonTextStyle(false);
	RefreshStartButtonState();
}

void UMHMainMenuWidget::NativeDestruct()
{
	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleStartButtonClicked);
		StartButton->OnHovered.RemoveDynamic(this, &ThisClass::HandleStartButtonHovered);
		StartButton->OnUnhovered.RemoveDynamic(this, &ThisClass::HandleStartButtonUnhovered);
	}

	for (UMHWeaponMenuSlotWidget* SlotWidget : CreatedSlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->OnWeaponMenuSlotClicked.RemoveDynamic(this, &ThisClass::HandleWeaponMenuSlotClicked);
		}
	}

	Super::NativeDestruct();
}

TSubclassOf<AMHWeaponInstance> UMHMainMenuWidget::GetSelectedWeaponClass() const
{
	return WeaponSlotClasses.IsValidIndex(SelectedSlotIndex)
		? WeaponSlotClasses[SelectedSlotIndex]
		: nullptr;
}

void UMHMainMenuWidget::HandleWeaponMenuSlotClicked(UMHWeaponMenuSlotWidget* ClickedSlot)
{
	const int32 ClickedIndex = CreatedSlotWidgets.IndexOfByKey(ClickedSlot);
	if (ClickedIndex != INDEX_NONE)
	{
		SelectSlot(ClickedIndex);
	}
}

void UMHMainMenuWidget::HandleStartButtonClicked()
{
	if (TSubclassOf<AMHWeaponInstance> SelectedWeaponClass = GetSelectedWeaponClass())
	{
		OnStartBattleRequested.Broadcast(*SelectedWeaponClass);
	}
}

void UMHMainMenuWidget::HandleStartButtonHovered()
{
	RefreshStartButtonTextStyle(true);
}

void UMHMainMenuWidget::HandleStartButtonUnhovered()
{
	RefreshStartButtonTextStyle(false);
}

void UMHMainMenuWidget::BuildWeaponSlots()
{
	for (UMHWeaponMenuSlotWidget* SlotWidget : CreatedSlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->OnWeaponMenuSlotClicked.RemoveDynamic(this, &ThisClass::HandleWeaponMenuSlotClicked);
		}
	}

	CreatedSlotWidgets.Reset();
	SelectedSlotIndex = INDEX_NONE;

	if (!WeaponSlotContainer || !WeaponMenuSlotWidgetClass)
	{
		RefreshSelectedWeaponText();
		return;
	}

	WeaponSlotContainer->ClearChildren();

	for (TSubclassOf<AMHWeaponInstance> WeaponClass : WeaponSlotClasses)
	{
		UMHWeaponMenuSlotWidget* SlotWidget = CreateWidget<UMHWeaponMenuSlotWidget>(this, WeaponMenuSlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		SlotWidget->SetItemIconBrush(BuildSlotBrush(WeaponClass));
		if (const UMHItemDataBase* ItemData = ResolvePreviewItemData(WeaponClass))
		{
			SlotWidget->SetWeaponName(ItemData->Name);
		}
		else
		{
			SlotWidget->SetWeaponName(EmptySelectionText);
		}

		SlotWidget->OnWeaponMenuSlotClicked.AddDynamic(this, &ThisClass::HandleWeaponMenuSlotClicked);
		WeaponSlotContainer->AddChild(SlotWidget);
		CreatedSlotWidgets.Add(SlotWidget);
	}

	for (int32 Index = 0; Index < WeaponSlotClasses.Num(); ++Index)
	{
		if (WeaponSlotClasses[Index])
		{
			SelectedSlotIndex = Index;
			break;
		}
	}

	RefreshSelectionVisuals();
	RefreshSelectedWeaponText();
	RefreshWeaponStatPanel();
}

void UMHMainMenuWidget::SelectSlot(const int32 NewIndex)
{
	if (!WeaponSlotClasses.IsValidIndex(NewIndex) || !WeaponSlotClasses[NewIndex])
	{
		return;
	}

	SelectedSlotIndex = NewIndex;
	RefreshSelectionVisuals();
	RefreshSelectedWeaponText();
	RefreshWeaponStatPanel();
	RefreshStartButtonState();
}

void UMHMainMenuWidget::RefreshSelectionVisuals()
{
	for (int32 Index = 0; Index < CreatedSlotWidgets.Num(); ++Index)
	{
		if (UMHWeaponMenuSlotWidget* SlotWidget = CreatedSlotWidgets[Index])
		{
			SlotWidget->SetSelected(Index == SelectedSlotIndex);
		}
	}
}

void UMHMainMenuWidget::RefreshSelectedWeaponText()
{
	if (!SelectedWeaponText)
	{
		return;
	}

	const UMHItemDataBase* ItemData = ResolvePreviewItemData(GetSelectedWeaponClass());
	if (ItemData)
	{
		SelectedWeaponText->SetText(ItemData->Name);
		return;
	}

	SelectedWeaponText->SetText(EmptySelectionText);
}

void UMHMainMenuWidget::RefreshWeaponStatPanel()
{
	if (!WeaponStatPanelWidget)
	{
		return;
	}

	WeaponStatPanelWidget->ApplyWeaponClass(GetSelectedWeaponClass());
}

void UMHMainMenuWidget::RefreshStartButtonState()
{
	if (StartButton)
	{
		StartButton->SetIsEnabled(GetSelectedWeaponClass() != nullptr);
	}
}

void UMHMainMenuWidget::RefreshStartButtonTextStyle(const bool bHovered)
{
	if (!StartButtonText)
	{
		return;
	}

	if (!bHasCachedStartButtonBaseFont)
	{
		CachedStartButtonBaseFont = StartButtonText->GetFont();
		bHasCachedStartButtonBaseFont = true;
	}

	FSlateFontInfo UpdatedFont = CachedStartButtonBaseFont;
	UpdatedFont.OutlineSettings.OutlineSize = bHovered ? StartButtonTextOutlineSize : 0;
	UpdatedFont.OutlineSettings.OutlineColor = StartButtonTextOutlineColor;
	StartButtonText->SetFont(UpdatedFont);
	StartButtonText->SetRenderScale(bHovered ? StartButtonTextHoveredScale : FVector2D(1.0f, 1.0f));
}

FSlateBrush UMHMainMenuWidget::BuildSlotBrush(const TSubclassOf<AMHWeaponInstance> InWeaponClass) const
{
	FSlateBrush SlotBrush;

	const UMHItemDataBase* ItemData = ResolvePreviewItemData(InWeaponClass);
	if (!ItemData)
	{
		return SlotBrush;
	}

	if (!ItemData->IconSprite.IsNull())
	{
		if (UPaperSprite* IconSprite = ItemData->IconSprite.LoadSynchronous())
		{
			const FSlateAtlasData AtlasData = IconSprite->GetSlateAtlasData();
			SlotBrush.SetResourceObject(IconSprite);
			SlotBrush.ImageSize = AtlasData.GetSourceDimensions();
			return SlotBrush;
		}
	}

	if (!ItemData->Icon.IsNull())
	{
		if (UTexture2D* IconTexture = ItemData->Icon.LoadSynchronous())
		{
			SlotBrush.SetResourceObject(IconTexture);
			SlotBrush.ImageSize = FVector2D(
				static_cast<float>(IconTexture->GetSizeX()),
				static_cast<float>(IconTexture->GetSizeY()));
		}
	}

	return SlotBrush;
}

const UMHItemDataBase* UMHMainMenuWidget::ResolvePreviewItemData(const TSubclassOf<AMHWeaponInstance> InWeaponClass) const
{
	UItemDataRegistry* ItemRegistry = nullptr;
	FName ItemDataKey = NAME_None;
	if (!ResolveWeaponPreviewData(InWeaponClass, ItemRegistry, ItemDataKey) || !ItemRegistry)
	{
		return nullptr;
	}

	return ItemRegistry->GetItemData(ItemDataKey);
}

bool UMHMainMenuWidget::ResolveWeaponPreviewData(
	const TSubclassOf<AMHWeaponInstance> InWeaponClass,
	UItemDataRegistry*& OutRegistry,
	FName& OutItemDataKey) const
{
	OutRegistry = nullptr;
	OutItemDataKey = NAME_None;

	if (!InWeaponClass)
	{
		return false;
	}

	const AMHWeaponInstance* WeaponCDO = InWeaponClass->GetDefaultObject<AMHWeaponInstance>();
	const AMHItemInstanceBase* ItemCDO = Cast<AMHItemInstanceBase>(WeaponCDO);
	if (!ItemCDO)
	{
		return false;
	}

	OutRegistry = ItemCDO->GetItemRegistry();
	OutItemDataKey = ItemCDO->GetItemDataKey();
	return OutRegistry != nullptr && !OutItemDataKey.IsNone();
}
