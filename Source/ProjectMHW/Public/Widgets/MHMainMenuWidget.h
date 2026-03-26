#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHMainMenuWidget.generated.h"

class AMHWeaponInstance;
class UButton;
class UPanelWidget;
class UTextBlock;
class UMHItemDataBase;
class UMHWeaponMenuSlotWidget;
class UMHWeaponStatPanelWidget;
class UItemDataRegistry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMHOnStartBattleRequested, UClass*, SelectedWeaponClass);

UCLASS()
class PROJECTMHW_API UMHMainMenuWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	TSubclassOf<AMHWeaponInstance> GetSelectedWeaponClass() const;

	UPROPERTY(BlueprintAssignable, Category="MH|Frontend")
	FMHOnStartBattleRequested OnStartBattleRequested;

protected:
	UFUNCTION()
	void HandleWeaponMenuSlotClicked(UMHWeaponMenuSlotWidget* ClickedSlot);

	UFUNCTION()
	void HandleStartButtonClicked();

	UFUNCTION()
	void HandleStartButtonHovered();

	UFUNCTION()
	void HandleStartButtonUnhovered();

	void BuildWeaponSlots();
	void SelectSlot(int32 NewIndex);
	void RefreshSelectionVisuals();
	void RefreshSelectedWeaponText();
	void RefreshWeaponStatPanel();
	void RefreshStartButtonState();
	void RefreshStartButtonTextStyle(bool bHovered);
	FSlateBrush BuildSlotBrush(TSubclassOf<AMHWeaponInstance> InWeaponClass) const;
	const UMHItemDataBase* ResolvePreviewItemData(TSubclassOf<AMHWeaponInstance> InWeaponClass) const;
	bool ResolveWeaponPreviewData(
		TSubclassOf<AMHWeaponInstance> InWeaponClass,
		UItemDataRegistry*& OutRegistry,
		FName& OutItemDataKey) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	TArray<TSubclassOf<AMHWeaponInstance>> WeaponSlotClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	TSubclassOf<UMHWeaponMenuSlotWidget> WeaponMenuSlotWidgetClass;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UPanelWidget> WeaponSlotContainer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UButton> StartButton = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UTextBlock> StartButtonText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UTextBlock> SelectedWeaponText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatPanelWidget> WeaponStatPanelWidget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText EmptySelectionText = FText::FromString(TEXT("No weapon selected"));

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMHWeaponMenuSlotWidget>> CreatedSlotWidgets;

	UPROPERTY(Transient)
	int32 SelectedSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend|Style", meta=(ClampMin="0"))
	int32 StartButtonTextOutlineSize = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend|Style")
	FLinearColor StartButtonTextOutlineColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend|Style")
	FVector2D StartButtonTextHoveredScale = FVector2D(1.06f, 1.06f);

private:
	FSlateFontInfo CachedStartButtonBaseFont;
	bool bHasCachedStartButtonBaseFont = false;
};
