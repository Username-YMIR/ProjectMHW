#include "Widgets/MHWeaponStatRowWidget.h"

#include "Components/TextBlock.h"

void UMHWeaponStatRowWidget::SetTexts(const FText& InPropertyName, const FText& InPropertyValue)
{
	PropertyName = InPropertyName;
	PropertyValue = InPropertyValue;

	if (PropertyNameText)
	{
		PropertyNameText->SetText(PropertyName);
	}

	if (PropertyValueText)
	{
		PropertyValueText->SetText(PropertyValue);
	}
}
