#include "Widgets/MHWeaponStatPanelWidget.h"

#include "Items/Data/ItemDataRegistry.h"
#include "Items/Data/MHItemDataBase.h"
#include "Items/Data/MHWeaponItemData.h"
#include "Items/Instance/MHItemInstanceBase.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "Widgets/MHWeaponStatRowWidget.h"

void UMHWeaponStatPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ApplyWeaponClass(nullptr);
}

void UMHWeaponStatPanelWidget::ApplyWeaponClass(const TSubclassOf<AMHWeaponInstance> InWeaponClass)
{
	const UMHItemDataBase* ItemData = ResolvePreviewItemData(InWeaponClass);
	const UMHWeaponItemData* WeaponItemData = ResolveWeaponItemData(InWeaponClass);
	if (!ItemData || !WeaponItemData)
	{
		ApplyFallback();
		return;
	}

	const FMHAttackStats& AttackStats = WeaponItemData->AttackStats;

	SetRowText(NameRow, FText::FromString(TEXT("이름")), ItemData->Name);
	SetRowText(AttackPowerRow, FText::FromString(TEXT("공격력")), BuildNumberText(AttackStats.AttackPower));
	ApplySharpnessRows(AttackStats);
	SetRowText(AffinityRow, FText::FromString(TEXT("회심율")), BuildAffinityText(AttackStats.Affinity));
	SetRowText(ElementRow, FText::FromString(TEXT("무기 속성")), BuildElementText(AttackStats.AttackElementTag));
}

void UMHWeaponStatPanelWidget::ApplyFallback()
{
	SetRowText(NameRow, EmptyWeaponLabel, EmptyWeaponValue);
	SetRowText(AttackPowerRow, FText::FromString(TEXT("공격력")), EmptyWeaponValue);
	SetRowText(SharpnessTotalRow, FText::FromString(TEXT("예리도")), EmptyWeaponValue);
	SetRowText(SharpnessRedRow, BuildSharpnessColorText(EMHSharpnessColor::Red), EmptyWeaponValue);
	SetRowText(SharpnessOrangeRow, BuildSharpnessColorText(EMHSharpnessColor::Orange), EmptyWeaponValue);
	SetRowText(SharpnessYellowRow, BuildSharpnessColorText(EMHSharpnessColor::Yellow), EmptyWeaponValue);
	SetRowText(SharpnessGreenRow, BuildSharpnessColorText(EMHSharpnessColor::Green), EmptyWeaponValue);
	SetRowText(SharpnessBlueRow, BuildSharpnessColorText(EMHSharpnessColor::Blue), EmptyWeaponValue);
	SetRowText(SharpnessWhiteRow, BuildSharpnessColorText(EMHSharpnessColor::White), EmptyWeaponValue);
	SetRowText(AffinityRow, FText::FromString(TEXT("회심율")), EmptyWeaponValue);
	SetRowText(ElementRow, FText::FromString(TEXT("무기 속성")), EmptyWeaponValue);
}

void UMHWeaponStatPanelWidget::ApplySharpnessRows(const FMHAttackStats& InAttackStats)
{
	SetRowText(SharpnessTotalRow, FText::FromString(TEXT("예리도")), BuildNumberText(GetTotalSharpnessLength(InAttackStats.SharpnessLength)));
	SetRowText(SharpnessRedRow, BuildSharpnessColorText(EMHSharpnessColor::Red), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::Red));
	SetRowText(SharpnessOrangeRow, BuildSharpnessColorText(EMHSharpnessColor::Orange), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::Orange));
	SetRowText(SharpnessYellowRow, BuildSharpnessColorText(EMHSharpnessColor::Yellow), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::Yellow));
	SetRowText(SharpnessGreenRow, BuildSharpnessColorText(EMHSharpnessColor::Green), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::Green));
	SetRowText(SharpnessBlueRow, BuildSharpnessColorText(EMHSharpnessColor::Blue), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::Blue));
	SetRowText(SharpnessWhiteRow, BuildSharpnessColorText(EMHSharpnessColor::White), BuildSharpnessValueText(InAttackStats, EMHSharpnessColor::White));
}

void UMHWeaponStatPanelWidget::SetRowText(
	UMHWeaponStatRowWidget* InRowWidget,
	const FText& InPropertyName,
	const FText& InPropertyValue) const
{
	if (InRowWidget)
	{
		InRowWidget->SetTexts(InPropertyName, InPropertyValue);
	}
}

const UMHItemDataBase* UMHWeaponStatPanelWidget::ResolvePreviewItemData(const TSubclassOf<AMHWeaponInstance> InWeaponClass) const
{
	UItemDataRegistry* ItemRegistry = nullptr;
	FName ItemDataKey = NAME_None;
	if (!ResolveWeaponPreviewData(InWeaponClass, ItemRegistry, ItemDataKey) || !ItemRegistry)
	{
		return nullptr;
	}

	return ItemRegistry->GetItemData(ItemDataKey);
}

const UMHWeaponItemData* UMHWeaponStatPanelWidget::ResolveWeaponItemData(const TSubclassOf<AMHWeaponInstance> InWeaponClass) const
{
	return Cast<UMHWeaponItemData>(ResolvePreviewItemData(InWeaponClass));
}

bool UMHWeaponStatPanelWidget::ResolveWeaponPreviewData(
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

FText UMHWeaponStatPanelWidget::BuildElementText(const FGameplayTag& InElementTag) const
{
	if (!InElementTag.IsValid())
	{
		return FText::FromString(TEXT("없음"));
	}

	const FString TagString = InElementTag.ToString();
	if (TagString.Contains(TEXT("Fire")))
	{
		return FText::FromString(TEXT("불"));
	}
	if (TagString.Contains(TEXT("Water")))
	{
		return FText::FromString(TEXT("물"));
	}
	if (TagString.Contains(TEXT("Thunder")))
	{
		return FText::FromString(TEXT("번개"));
	}
	if (TagString.Contains(TEXT("Ice")))
	{
		return FText::FromString(TEXT("얼음"));
	}
	if (TagString.Contains(TEXT("Dragon")))
	{
		return FText::FromString(TEXT("용"));
	}

	return FText::FromString(TagString);
}

FText UMHWeaponStatPanelWidget::BuildSharpnessColorText(const EMHSharpnessColor InColor) const
{
	switch (InColor)
	{
	case EMHSharpnessColor::Red:
		return FText::FromString(TEXT("빨강"));
	case EMHSharpnessColor::Orange:
		return FText::FromString(TEXT("주황"));
	case EMHSharpnessColor::Yellow:
		return FText::FromString(TEXT("노랑"));
	case EMHSharpnessColor::Green:
		return FText::FromString(TEXT("초록"));
	case EMHSharpnessColor::Blue:
		return FText::FromString(TEXT("파랑"));
	case EMHSharpnessColor::White:
		return FText::FromString(TEXT("하양"));
	default:
		return FText::FromString(TEXT("없음"));
	}
}

FText UMHWeaponStatPanelWidget::BuildNumberText(const float InValue) const
{
	return FText::AsNumber(FMath::RoundToInt(InValue));
}

FText UMHWeaponStatPanelWidget::BuildAffinityText(const float InValue) const
{
	return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(InValue * 100.0f)));
}

FText UMHWeaponStatPanelWidget::BuildSharpnessValueText(const FMHAttackStats& InAttackStats, const EMHSharpnessColor InColor) const
{
	if (static_cast<uint8>(InColor) > static_cast<uint8>(InAttackStats.MaxSharpnessColor))
	{
		return FText::FromString(TEXT("-"));
	}

	return BuildNumberText(GetSharpnessLength(InAttackStats.SharpnessLength, InColor));
}
