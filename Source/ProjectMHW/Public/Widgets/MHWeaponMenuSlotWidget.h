#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHWeaponMenuSlotWidget.generated.h"

class UButton;
class UTextBlock;
class UMHItemSlotWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMHOnWeaponMenuSlotClicked, UMHWeaponMenuSlotWidget*, ClickedSlot);

UCLASS()
class PROJECTMHW_API UMHWeaponMenuSlotWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void SetWeaponName(const FText& InWeaponName);

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void SetItemIconBrush(const FSlateBrush& InBrush);

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void SetSelected(bool bSelected);

	UPROPERTY(BlueprintAssignable, Category="MH|Frontend")
	FMHOnWeaponMenuSlotClicked OnWeaponMenuSlotClicked;

protected:
	UFUNCTION()
	void HandleSlotButtonClicked();

	void RefreshWeaponNameVisuals() const;

protected:
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UButton> SlotButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHItemSlotWidget> ItemSlotWidget = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UTextBlock> WeaponNameText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FSlateColor SelectedTextColor = FSlateColor(FLinearColor(1.0f, 0.85f, 0.35f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FSlateColor UnselectedTextColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));

	UPROPERTY(Transient, BlueprintReadOnly, Category="MH|Frontend")
	bool bIsSelected = false;
};
