#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHProgressBarWidget.generated.h"

class UProgressBar;

/**
 * 수치 기반 ProgressBar 공통 위젯
 *
 * 책임:
 * - Max / Current 값 보관
 * - 값 변경 시 ProgressBar Percent 갱신
 *
 * 비책임:
 * - ASC 직접 참조
 * - 플레이어 컨텍스트 관리
 * - 텍스트 / 애니메이션 / 지연 게이지 처리
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

	UFUNCTION(BlueprintPure, Category = "MH|ProgressBar")
	float GetMaxValue() const { return MaxValue; }

	UFUNCTION(BlueprintPure, Category = "MH|ProgressBar")
	float GetCurrentValue() const { return CurrentValue; }

protected:
	/** 실제 UMG ProgressBar */
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> ProgressBar = nullptr;

	/** 게이지 최대값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar", meta = (ClampMin = "0.0"))
	float MaxValue = 100.f;

	/** 게이지 현재값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MH|ProgressBar")
	float CurrentValue = 100.f;
};