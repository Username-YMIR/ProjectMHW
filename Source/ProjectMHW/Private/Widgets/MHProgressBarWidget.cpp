#include "Widgets/MHProgressBarWidget.h"

#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

UMHProgressBarWidget::UMHProgressBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHProgressBarWidget::SetMaxValue(float InMaxValue)
{
	MaxValue = FMath::Max(0.f, InMaxValue);
	UpdateProgressBar();
}

void UMHProgressBarWidget::SetCurrentValue(float InCurrentValue)
{
	CurrentValue = InCurrentValue;
	UpdateProgressBar();
}

void UMHProgressBarWidget::SetValues(float InCurrentValue, float InMaxValue)
{
	MaxValue = FMath::Max(0.f, InMaxValue);
	CurrentValue = InCurrentValue;
	UpdateProgressBar();
}

void UMHProgressBarWidget::UpdateProgressBar()
{
	if (!ProgressBar)
	{
		return;
	}

	const float Percent = FMath::Clamp(
		UKismetMathLibrary::SafeDivide(CurrentValue, MaxValue),
		0.f,
		1.f
	);

	ProgressBar->SetPercent(Percent);
}