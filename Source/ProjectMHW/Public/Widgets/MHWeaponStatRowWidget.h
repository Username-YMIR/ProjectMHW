#pragma once

#include "CoreMinimal.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHWeaponStatRowWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECTMHW_API UMHWeaponStatRowWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void SetTexts(const FText& InPropertyName, const FText& InPropertyValue);

protected:
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UTextBlock> PropertyNameText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UTextBlock> PropertyValueText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText PropertyValue;
};
