#include "System/MHFrontendGameInstance.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Items/Instance/MHWeaponInstance.h"
#include "Kismet/GameplayStatics.h"

void UMHFrontendGameInstance::SetPendingWeaponClass(const TSubclassOf<AMHWeaponInstance> InWeaponClass)
{
	PendingWeaponClass = TSoftClassPtr<AMHWeaponInstance>(InWeaponClass);
}

void UMHFrontendGameInstance::ClearPendingWeaponClass()
{
	PendingWeaponClass.Reset();
}

TSubclassOf<AMHWeaponInstance> UMHFrontendGameInstance::GetPendingWeaponClass() const
{
	UClass* LoadedClass = PendingWeaponClass.Get();
	if (!LoadedClass && !PendingWeaponClass.IsNull())
	{
		LoadedClass = PendingWeaponClass.LoadSynchronous();
	}

	return LoadedClass;
}

bool UMHFrontendGameInstance::StartBattleTransition(const FName InBattleLevelName)
{
	if (bBattleTransitionInProgress || InBattleLevelName.IsNone())
	{
		return false;
	}

	PendingBattleLevelName = InBattleLevelName;
	bBattleTransitionInProgress = true;
	bPreloadCompleted = false;
	CachedLoadingProgress = 0.0f;
	CachedLoadingStatusText = FText::FromString(TEXT("Preparing battle..."));

	BeginPreload();
	return true;
}

float UMHFrontendGameInstance::GetLoadingProgress() const
{
	if (PreloadHandle.IsValid())
	{
		return FMath::Clamp(0.1f + PreloadHandle->GetProgress() * 0.9f, 0.0f, 1.0f);
	}

	return CachedLoadingProgress;
}

FText UMHFrontendGameInstance::GetLoadingStatusText() const
{
	return CachedLoadingStatusText;
}

void UMHFrontendGameInstance::BeginPreload()
{
	TArray<FSoftObjectPath> AssetsToLoad;

	CachedLoadingProgress = 0.1f;
	CachedLoadingStatusText = FText::FromString(TEXT("Loading selected weapon..."));

	if (!PendingWeaponClass.IsNull())
	{
		AssetsToLoad.AddUnique(PendingWeaponClass.ToSoftObjectPath());
	}

	if (AssetsToLoad.IsEmpty())
	{
		CachedLoadingProgress = 1.0f;
		CachedLoadingStatusText = FText::FromString(TEXT("Ready"));
		bPreloadCompleted = true;
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	PreloadHandle = StreamableManager.RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateUObject(this, &ThisClass::HandlePreloadCompleted)
	);

	if (!PreloadHandle.IsValid())
	{
		CachedLoadingProgress = 1.0f;
		CachedLoadingStatusText = FText::FromString(TEXT("Ready"));
		bPreloadCompleted = true;
	}
}

void UMHFrontendGameInstance::HandlePreloadCompleted()
{
	CachedLoadingProgress = 1.0f;
	CachedLoadingStatusText = FText::FromString(TEXT("Ready"));
	bPreloadCompleted = true;
}

bool UMHFrontendGameInstance::OpenPendingBattleLevel()
{
	if (PendingBattleLevelName.IsNone())
	{
		ResetTransitionState();
		return false;
	}

	UGameplayStatics::OpenLevel(this, PendingBattleLevelName);
	ResetTransitionState();
	return true;
}

void UMHFrontendGameInstance::ResetTransitionState()
{
	PreloadHandle.Reset();
	PendingBattleLevelName = NAME_None;
	bBattleTransitionInProgress = false;
	bPreloadCompleted = false;
	CachedLoadingProgress = 0.0f;
	CachedLoadingStatusText = FText::GetEmpty();
}
