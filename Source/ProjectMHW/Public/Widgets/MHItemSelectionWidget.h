// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHItemSelectionWidget.generated.h"

class UMHItemSlotWidget;

/**
 * 하단 consumable 선택 상태를 표시하는 HUD 위젯
 */
UCLASS()
class PROJECTMHW_API UMHItemSelectionWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	UMHItemSelectionWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void HandleWidgetInitialized() override;

	void BindToPlayerCharacter();
	void UnbindFromPlayerCharacter();
	void SyncInitialSelection();

	UFUNCTION()
	void HandleConsumableSelectionChanged(EMHConsumableSelection NewSelection);

public:
	UFUNCTION(BlueprintCallable, Category = "MH|ItemSelection")
	void RefreshSelection(EMHConsumableSelection InSelection);

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|ItemSelection")
	TObjectPtr<UMHItemSlotWidget> SharpenSlot = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|ItemSelection")
	TObjectPtr<UMHItemSlotWidget> PotionSlot = nullptr;

private:
	TWeakObjectPtr<AMHPlayerCharacter> CachedPlayerCharacter;
};
