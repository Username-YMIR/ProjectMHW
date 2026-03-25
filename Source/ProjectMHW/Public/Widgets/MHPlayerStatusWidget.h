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
	void UpdateSharpnessBarVisual();
	void UpdateSpiritGaugeVisual();
	void ApplySpiritGaugeLevelStyle(int32 InSpiritLevel);
	FLinearColor GetSpiritLevelFrontColor(int32 InSpiritLevel) const;
	FLinearColor GetSpiritLevelBackColor(int32 InSpiritLevel) const;

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
	void SetSpiritGaugeTimerValues(float InRemainingTime, float InDuration);

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
	void HandleHealableHealthChanged(float InHealableHealth, float InMaxHealth);

	UFUNCTION()
	void HandleSpiritGaugeChanged(float InCurrentSpiritGauge, float InMaxSpiritGauge);

	UFUNCTION()
	void HandleSpiritLevelChanged(int32 InCurrentSpiritLevel, int32 InMaxSpiritLevel);

	UFUNCTION()
	void HandleSpiritLevelTimerChanged(float InRemainingTime, float InDuration);

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel0FrontColor = FLinearColor(0.55f, 0.55f, 0.55f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel1FrontColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel2FrontColor = FLinearColor(0.95f, 0.80f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel3FrontColor = FLinearColor(0.88f, 0.18f, 0.12f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel0BackColor = FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel1BackColor = FLinearColor(0.38f, 0.38f, 0.38f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel2BackColor = FLinearColor(0.45f, 0.35f, 0.10f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	FLinearColor SpiritGaugeLevel3BackColor = FLinearColor(0.40f, 0.09f, 0.07f, 1.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|PlayerStatus|SpiritGauge")
	int32 CachedSpiritLevel = 0;

private:
	// 비소유 참조이므로 TWeakObjectPtr 사용
	TWeakObjectPtr<AMHPlayerCharacter> CachedPlayerCharacter;
};
