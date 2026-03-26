#include "Widgets/MHLoadingWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UMHLoadingWidget::SetProgress(const float InProgress)
{
	Progress = FMath::Clamp(InProgress, 0.0f, 1.0f);

	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(Progress);
	}
}

void UMHLoadingWidget::SetStatusText(const FText& InText)
{
	StatusText = InText;

	if (LoadingStatusText)
	{
		LoadingStatusText->SetText(StatusText);
	}
}

float UMHLoadingWidget::PlayFadeIn()
{
	if (!FadeInAnimation)
	{
		return 0.0f;
	}

	PlayAnimation(FadeInAnimation);
	return FadeInAnimation->GetEndTime();
}

float UMHLoadingWidget::PlayFadeOut()
{
	if (!FadeOutAnimation)
	{
		return 0.0f;
	}

	PlayAnimation(FadeOutAnimation);
	return FadeOutAnimation->GetEndTime();
}
