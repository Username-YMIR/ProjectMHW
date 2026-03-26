#pragma once

#include "CoreMinimal.h"
#include "Widgets/MHUserWidgetBase.h"
#include "MHLoadingWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class PROJECTMHW_API UMHLoadingWidget : public UMHUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="MH|Loading")
	void SetProgress(float InProgress);

	UFUNCTION(BlueprintCallable, Category="MH|Loading")
	void SetStatusText(const FText& InText);	

	UFUNCTION(BlueprintCallable, Category="MH|Loading")
	float PlayFadeIn();

	UFUNCTION(BlueprintCallable, Category="MH|Loading")
	float PlayFadeOut();
	
protected:
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Loading")
	TObjectPtr<UProgressBar> LoadingProgressBar = nullptr;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="MH|Loading")
	TObjectPtr<UTextBlock> LoadingStatusText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Loading")
	float Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Loading")
	FText StatusText;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeInAnimation = nullptr;

	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> FadeOutAnimation = nullptr;
};
