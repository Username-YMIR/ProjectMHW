// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHItemSlotWidget.generated.h"

class UImage;

/**
 * 아이템 선택 HUD에서 사용하는 공용 슬롯 위젯
 */
UCLASS()
class PROJECTMHW_API UMHItemSlotWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	UMHItemSlotWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

	void RefreshVisuals();
	void UpdateItemIconVisibility();

public:
	UFUNCTION(BlueprintCallable, Category = "MH|ItemSlot")
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintCallable, Category = "MH|ItemSlot")
	void SetItemIconBrush(const FSlateBrush& InBrush);

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|ItemSlot")
	TObjectPtr<UImage> SlotImage = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|ItemSlot")
	TObjectPtr<UImage> ItemIconImage = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|ItemSlot")
	TObjectPtr<UImage> SelectionOutlineImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MH|ItemSlot|Style", meta = (ExposeOnSpawn = "true"))
	FSlateBrush ItemIconBrush;
};
