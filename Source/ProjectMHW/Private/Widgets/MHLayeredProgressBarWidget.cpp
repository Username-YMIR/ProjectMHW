#include "Widgets/MHLayeredProgressBarWidget.h"

#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

UMHLayeredProgressBarWidget::UMHLayeredProgressBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHLayeredProgressBarWidget::SetFrontMaxValue(float InMaxValue)
{
	FrontMaxValue = FMath::Max(0.f, InMaxValue);
	UpdateFrontProgressBar();
}

void UMHLayeredProgressBarWidget::SetFrontCurrentValue(float InCurrentValue)
{
	FrontCurrentValue = InCurrentValue;
	UpdateFrontProgressBar();
}

void UMHLayeredProgressBarWidget::SetFrontValues(float InCurrentValue, float InMaxValue)
{
	FrontCurrentValue = InCurrentValue;
	FrontMaxValue = FMath::Max(0.f, InMaxValue);
	UpdateFrontProgressBar();
}

void UMHLayeredProgressBarWidget::SetBackMaxValue(float InMaxValue)
{
	BackMaxValue = FMath::Max(0.f, InMaxValue);
	UpdateBackProgressBar();
}

void UMHLayeredProgressBarWidget::SetBackCurrentValue(float InCurrentValue)
{
	BackCurrentValue = InCurrentValue;
	UpdateBackProgressBar();
}

void UMHLayeredProgressBarWidget::SetBackValues(float InCurrentValue, float InMaxValue)
{
	BackCurrentValue = InCurrentValue;
	BackMaxValue = FMath::Max(0.f, InMaxValue);
	UpdateBackProgressBar();
}

void UMHLayeredProgressBarWidget::UpdateFrontProgressBar()
{
	const float FrontPercent = FMath::Clamp(
		UKismetMathLibrary::SafeDivide(FrontCurrentValue, FrontMaxValue),
		0.f,
		1.f
	);

	if (FrontProgressBar)
	{
		FrontProgressBar->SetPercent(FrontPercent);
	}
}

void UMHLayeredProgressBarWidget::UpdateBackProgressBar()
{
	const float BackPercent = FMath::Clamp(
		UKismetMathLibrary::SafeDivide(BackCurrentValue, BackMaxValue),
		0.f,
		1.f
	);

	if (BackProgressBar)
	{
		BackProgressBar->SetPercent(BackPercent);
	}
}