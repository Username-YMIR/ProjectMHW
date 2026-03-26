#pragma once

#include "CoreMinimal.h"
#include "Type/MHCombatStatStructType.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHWeaponStatPanelWidget.generated.h"

class AMHWeaponInstance;
class UPanelWidget;
class UMHItemDataBase;
class UMHWeaponItemData;
class UMHWeaponStatRowWidget;
class UItemDataRegistry;

UCLASS()
class PROJECTMHW_API UMHWeaponStatPanelWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void ApplyWeaponClass(TSubclassOf<AMHWeaponInstance> InWeaponClass);

protected:
	void ApplyFallback();
	void ApplySharpnessRows(const FMHAttackStats& InAttackStats);
	void SetRowText(UMHWeaponStatRowWidget* InRowWidget, const FText& InPropertyName, const FText& InPropertyValue) const;
	const UMHItemDataBase* ResolvePreviewItemData(TSubclassOf<AMHWeaponInstance> InWeaponClass) const;
	const UMHWeaponItemData* ResolveWeaponItemData(TSubclassOf<AMHWeaponInstance> InWeaponClass) const;
	bool ResolveWeaponPreviewData(
		TSubclassOf<AMHWeaponInstance> InWeaponClass,
		UItemDataRegistry*& OutRegistry,
		FName& OutItemDataKey) const;

	FText BuildElementText(const FGameplayTag& InElementTag) const;
	FText BuildSharpnessColorText(EMHSharpnessColor InColor) const;
	FText BuildNumberText(float InValue) const;
	FText BuildAffinityText(float InValue) const;
	FText BuildSharpnessValueText(const FMHAttackStats& InAttackStats, EMHSharpnessColor InColor) const;

protected:
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> NameRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> AttackPowerRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessTotalRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessRedRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessOrangeRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessYellowRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessGreenRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessBlueRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> SharpnessWhiteRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> AffinityRow = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Frontend")
	TObjectPtr<UMHWeaponStatRowWidget> ElementRow = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText EmptyWeaponLabel = FText::FromString(TEXT("선택 무기"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FText EmptyWeaponValue = FText::FromString(TEXT("없음"));
};
