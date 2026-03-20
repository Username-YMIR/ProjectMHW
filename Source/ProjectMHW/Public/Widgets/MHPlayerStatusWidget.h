#pragma once

#include "CoreMinimal.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHPlayerStatusWidget.generated.h"

class AMHPlayerCharacter;
class UMHLayeredProgressBarWidget;
class UMHProgressBarWidget;

UCLASS(Blueprintable)
class PROJECTMHW_API UMHPlayerStatusWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	UMHPlayerStatusWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void HandleWidgetInitialized() override;

protected:
	void BindToPlayerCharacter();
	void UnbindFromPlayerCharacter();
	void SyncInitialValues();

public:
	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetCurrentHealth(float InCurrentHealth);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetMaxHealth(float InMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetHealthValues(float InCurrentHealth, float InMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetCurrentSpiritGauge(float InCurrentSpiritGauge);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetMaxSpiritGauge(float InMaxSpiritGauge);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetSpiritGaugeValues(float InCurrentSpiritGauge, float InMaxSpiritGauge);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetCurrentStamina(float InCurrentStamina);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetMaxStamina(float InMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetStaminaValues(float InCurrentStamina, float InMaxStamina);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetCurrentSharpness(float InCurrentSharpness);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetMaxSharpness(float InMaxSharpness);

	UFUNCTION(BlueprintCallable, Category = "MH|PlayerStatus")
	void SetSharpnessValues(float InCurrentSharpness, float InMaxSharpness);

protected:
	// PlayerCharacter UI Delegate 예시 핸들러
	UFUNCTION()
	void HandleHealthChanged(float InCurrentHealth, float InMaxHealth);

	UFUNCTION()
	void HandleSpiritGaugeChanged(float InCurrentSpiritGauge, float InMaxSpiritGauge);

	UFUNCTION()
	void HandleStaminaChanged(float InCurrentStamina, float InMaxStamina);

	UFUNCTION()
	void HandleSharpnessChanged(float InCurrentSharpness, float InMaxSharpness);

protected:
	// HP Bar
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|PlayerStatus")
	TObjectPtr<UMHLayeredProgressBarWidget> HPBar = nullptr;

	// Spirit Gauge Bar
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|PlayerStatus")
	TObjectPtr<UMHLayeredProgressBarWidget> SpiritGaugeBar = nullptr;

	// Stamina Bar
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|PlayerStatus")
	TObjectPtr<UMHProgressBarWidget> StaminaBar = nullptr;

	// Sharpness Bar
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "MH|PlayerStatus")
	TObjectPtr<UMHProgressBarWidget> SharpnessBar = nullptr;

private:
	// 비소유 참조이므로 TWeakObjectPtr 사용
	TWeakObjectPtr<AMHPlayerCharacter> CachedPlayerCharacter;
};
