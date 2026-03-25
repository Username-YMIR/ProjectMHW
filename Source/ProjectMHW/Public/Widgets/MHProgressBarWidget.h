#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/MHCombatStatStructType.h"
#include "MHProgressBarWidget.generated.h"

class UProgressBar;

/**
 * ?섏튂 湲곕컲 ProgressBar 怨듯넻 ?꾩젽
 *
 * 梨낆엫:
 * - Max / Current 媛?蹂닿?
 * - 媛?蹂寃???ProgressBar Percent 媛깆떊
 *
 * 鍮꾩콉??
 * - ASC 吏곸젒 李몄“
 * - ?뚮젅?댁뼱 而⑦뀓?ㅽ듃 愿由?
 * - ?띿뒪??/ ?좊땲硫붿씠??/ 吏??寃뚯씠吏 泥섎━
 */
UCLASS(Abstract, Blueprintable)
class PROJECTMHW_API UMHProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMHProgressBarWidget(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void SetMaxValue(float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void SetCurrentValue(float InCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void SetValues(float InCurrentValue, float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void UpdateProgressBar();

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void SetFillColor(const FLinearColor& InFillColor);

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void ResetFillColor();

	UFUNCTION(BlueprintCallable, Category = "MH|ProgressBar")
	void SetSharpnessFillColor(EMHSharpnessColor InSharpnessColor);

	UFUNCTION(BlueprintPure, Category = "MH|ProgressBar")
	float GetMaxValue() const { return MaxValue; }

	UFUNCTION(BlueprintPure, Category = "MH|ProgressBar")
	float GetCurrentValue() const { return CurrentValue; }

protected:
	virtual void NativePreConstruct() override;

protected:
	/** ?ㅼ젣 UMG ProgressBar */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> ProgressBar = nullptr;

	/** 寃뚯씠吏 理쒕?媛?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar", meta = (ClampMin = "0.0"))
	float MaxValue = 100.f;

	/** 寃뚯씠吏 ?꾩옱媛?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar")
	float CurrentValue = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor DefaultFillColor = FLinearColor(0.92f, 0.92f, 0.92f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessRedColor = FLinearColor(0.80f, 0.18f, 0.13f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessOrangeColor = FLinearColor(0.88f, 0.49f, 0.12f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessYellowColor = FLinearColor(0.92f, 0.79f, 0.20f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessGreenColor = FLinearColor(0.29f, 0.73f, 0.24f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessBlueColor = FLinearColor(0.18f, 0.48f, 0.86f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor SharpnessWhiteColor = FLinearColor(0.97f, 0.97f, 0.99f, 1.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|ProgressBar|Color")
	FLinearColor CurrentFillColor = FLinearColor::White;
};
