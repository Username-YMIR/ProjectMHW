// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHLayeredProgressBarWidget.generated.h"

class UProgressBar;

/**
 * Front / Back 이중 ProgressBar 위젯
 *
 * 책임:
 * - Front / Back ProgressBar 값 보관
 * - 값 변경 시 두 ProgressBar Percent 갱신
 *
 * 비책임:
 * - 지연 보간
 * - 애니메이션
 * - 게임 데이터 직접 참조
 */
UCLASS()
class PROJECTMHW_API UMHLayeredProgressBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
	public:
	UMHLayeredProgressBarWidget(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetFrontMaxValue(float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetFrontCurrentValue(float InCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetFrontValues(float InCurrentValue, float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetBackMaxValue(float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetBackCurrentValue(float InCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void SetBackValues(float InCurrentValue, float InMaxValue);

	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void UpdateFrontProgressBar();
	
	UFUNCTION(BlueprintCallable, Category = "MH|LayeredProgressBar")
	void UpdateBackProgressBar();

	UFUNCTION(BlueprintPure, Category = "MH|LayeredProgressBar")
	float GetFrontMaxValue() const { return FrontMaxValue; }

	UFUNCTION(BlueprintPure, Category = "MH|LayeredProgressBar")
	float GetFrontCurrentValue() const { return FrontCurrentValue; }

	UFUNCTION(BlueprintPure, Category = "MH|LayeredProgressBar")
	float GetBackMaxValue() const { return BackMaxValue; }

	UFUNCTION(BlueprintPure, Category = "MH|LayeredProgressBar")
	float GetBackCurrentValue() const { return BackCurrentValue; }

protected:
	/** 전면 ProgressBar */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> FrontProgressBar = nullptr;

	/** 후면 ProgressBar */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> BackProgressBar = nullptr;

	/** 전면 최대값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|LayeredProgressBar", meta = (ClampMin = "0.0"))
	float FrontMaxValue = 100.f;

	/** 전면 현재값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|LayeredProgressBar")
	float FrontCurrentValue = 100.f;

	/** 후면 최대값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|LayeredProgressBar", meta = (ClampMin = "0.0"))
	float BackMaxValue = 100.f;

	/** 후면 현재값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|LayeredProgressBar")
	float BackCurrentValue = 100.f;
};
