#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MHFrontendPlayerController.generated.h"

class UMHLoadingWidget;
class UMHMainMenuWidget;

UCLASS()
class PROJECTMHW_API AMHFrontendPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMHFrontendPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleStartBattleRequested(UClass* SelectedWeaponClass);

	UFUNCTION()
	void HandleOpenBattleLevelAfterFadeOut();

	void ShowMainMenu();
	void ShowLoadingScreen();
	void RefreshLoadingScreen();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	TSubclassOf<UMHMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	TSubclassOf<UMHLoadingWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MH|Frontend")
	FName BattleLevelName = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<UMHMainMenuWidget> MainMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMHLoadingWidget> LoadingWidget = nullptr;

	FTimerHandle OpenBattleLevelTimerHandle;

	bool bFadeOutRequested = false;
};
