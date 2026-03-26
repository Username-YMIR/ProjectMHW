#include "Frontend/MHFrontendPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "System/MHFrontendGameInstance.h"
#include "Widgets/MHLoadingWidget.h"
#include "Widgets/MHMainMenuWidget.h"

AMHFrontendPlayerController::AMHFrontendPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AMHFrontendPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShowMainMenu();

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMHFrontendPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RefreshLoadingScreen();
}

void AMHFrontendPlayerController::HandleStartBattleRequested(UClass* SelectedWeaponClass)
{
	UMHFrontendGameInstance* FrontendGameInstance = GetGameInstance<UMHFrontendGameInstance>();
	if (!FrontendGameInstance)
	{
		return;
	}

	FrontendGameInstance->SetPendingWeaponClass(TSubclassOf<AMHWeaponInstance>(SelectedWeaponClass));

	ShowLoadingScreen();

	if (!FrontendGameInstance->StartBattleTransition(BattleLevelName))
	{
		if (LoadingWidget)
		{
			LoadingWidget->SetStatusText(FText::FromString(TEXT("Battle level is not configured.")));
			LoadingWidget->SetProgress(0.0f);
		}
	}
	else
	{
		bFadeOutRequested = false;
	}
}

void AMHFrontendPlayerController::ShowMainMenu()
{
	if (!MainMenuWidgetClass || MainMenuWidget)
	{
		return;
	}

	MainMenuWidget = CreateWidget<UMHMainMenuWidget>(this, MainMenuWidgetClass);
	if (!MainMenuWidget)
	{
		return;
	}

	MainMenuWidget->OnStartBattleRequested.AddDynamic(this, &ThisClass::HandleStartBattleRequested);
	MainMenuWidget->AddToViewport(0);
}

void AMHFrontendPlayerController::ShowLoadingScreen()
{
	if (!LoadingWidget && LoadingWidgetClass)
	{
		LoadingWidget = CreateWidget<UMHLoadingWidget>(this, LoadingWidgetClass);
	}

	if (MainMenuWidget)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (LoadingWidget && !LoadingWidget->IsInViewport())
	{
		LoadingWidget->AddToViewport(100);
		LoadingWidget->PlayFadeIn();
	}

	RefreshLoadingScreen();
}

void AMHFrontendPlayerController::RefreshLoadingScreen()
{
	if (!LoadingWidget)
	{
		return;
	}

	const UMHFrontendGameInstance* FrontendGameInstance = GetGameInstance<UMHFrontendGameInstance>();
	if (!FrontendGameInstance || !FrontendGameInstance->IsBattleTransitionInProgress())
	{
		return;
	}

	LoadingWidget->SetProgress(FrontendGameInstance->GetLoadingProgress());
	LoadingWidget->SetStatusText(FrontendGameInstance->GetLoadingStatusText());

	if (!bFadeOutRequested && FrontendGameInstance->IsPreloadCompleted())
	{
		bFadeOutRequested = true;
		const float FadeOutDuration = LoadingWidget->PlayFadeOut();
		if (FadeOutDuration > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				OpenBattleLevelTimerHandle,
				this,
				&ThisClass::HandleOpenBattleLevelAfterFadeOut,
				FadeOutDuration,
				false
			);
		}
		else
		{
			HandleOpenBattleLevelAfterFadeOut();
		}
	}
}

void AMHFrontendPlayerController::HandleOpenBattleLevelAfterFadeOut()
{
	UMHFrontendGameInstance* FrontendGameInstance = GetGameInstance<UMHFrontendGameInstance>();
	if (!FrontendGameInstance)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	FrontendGameInstance->OpenPendingBattleLevel();
}
