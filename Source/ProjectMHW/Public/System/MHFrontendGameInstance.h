#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UObject/SoftObjectPtr.h"
#include "MHFrontendGameInstance.generated.h"

class AMHWeaponInstance;
class UWorld;
struct FStreamableHandle;

UCLASS()
class PROJECTMHW_API UMHFrontendGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void SetPendingWeaponClass(TSubclassOf<AMHWeaponInstance> InWeaponClass);

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	void ClearPendingWeaponClass();

	UFUNCTION(BlueprintPure, Category="MH|Frontend")
	TSubclassOf<AMHWeaponInstance> GetPendingWeaponClass() const;

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	bool StartBattleTransition(TSoftObjectPtr<UWorld> InBattleLevel);

	UFUNCTION(BlueprintPure, Category="MH|Frontend")
	float GetLoadingProgress() const;

	UFUNCTION(BlueprintPure, Category="MH|Frontend")
	FText GetLoadingStatusText() const;

	UFUNCTION(BlueprintPure, Category="MH|Frontend")
	bool IsBattleTransitionInProgress() const { return bBattleTransitionInProgress; }

	UFUNCTION(BlueprintPure, Category="MH|Frontend")
	bool IsPreloadCompleted() const { return bPreloadCompleted; }

	UFUNCTION(BlueprintCallable, Category="MH|Frontend")
	bool OpenPendingBattleLevel();

private:
	void BeginPreload();
	void HandlePreloadCompleted();
	void ResetTransitionState();

private:
	UPROPERTY(Transient)
	TSoftClassPtr<AMHWeaponInstance> PendingWeaponClass;

	UPROPERTY(Transient)
	TSoftObjectPtr<UWorld> PendingBattleLevel;

	TSharedPtr<FStreamableHandle> PreloadHandle;

	bool bBattleTransitionInProgress = false;
	bool bPreloadCompleted = false;

	UPROPERTY(Transient)
	float CachedLoadingProgress = 0.0f;

	UPROPERTY(Transient)
	FText CachedLoadingStatusText;
};
