#include "Widgets/MHProgressBarWidget.h"

#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"

UMHProgressBarWidget::UMHProgressBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentFillColor = DefaultFillColor;
}

void UMHProgressBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	CurrentFillColor = DefaultFillColor;

	UpdateProgressBar();
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

void UMHProgressBarWidget::SetFillColor(const FLinearColor& InFillColor)
{
	CurrentFillColor = InFillColor;
	UpdateProgressBar();
}

void UMHProgressBarWidget::ResetFillColor()
{
	CurrentFillColor = DefaultFillColor;
	UpdateProgressBar();
}

void UMHProgressBarWidget::SetSharpnessFillColor(EMHSharpnessColor InSharpnessColor)
{
	switch (InSharpnessColor)
	{
	case EMHSharpnessColor::Red:
		SetFillColor(SharpnessRedColor);
		break;
	case EMHSharpnessColor::Orange:
		SetFillColor(SharpnessOrangeColor);
		break;
	case EMHSharpnessColor::Yellow:
		SetFillColor(SharpnessYellowColor);
		break;
	case EMHSharpnessColor::Green:
		SetFillColor(SharpnessGreenColor);
		break;
	case EMHSharpnessColor::Blue:
		SetFillColor(SharpnessBlueColor);
		break;
	case EMHSharpnessColor::White:
		SetFillColor(SharpnessWhiteColor);
		break;
	default:
		ResetFillColor();
		break;
	}
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
	ProgressBar->SetFillColorAndOpacity(CurrentFillColor);
}
