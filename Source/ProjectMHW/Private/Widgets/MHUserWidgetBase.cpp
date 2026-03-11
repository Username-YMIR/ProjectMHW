// Fill out your copyright notice in the Description page of Project Settings.

// 제작자 : 이건주
// 제작일 : 2026-03-11
// 수정자 : 이건주
// 수정일 : 2026-03-11 
#include "Widgets/MHUserWidgetBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UMHUserWidgetBase::UMHUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHUserWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 생성 직후 1회성 초기화 시점
	RefreshWidgetReferences();
}

void UMHUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Viewport에 붙는 시점에서 최신 참조를 다시 캐시
	RefreshWidgetReferences();

	HandleWidgetInitialized();
	OnWidgetInitialized();

	bIsWidgetInitialized = true;
}

void UMHUserWidgetBase::NativeDestruct()
{
	CachedAbilitySystemComponent = nullptr;
	CachedPlayerState = nullptr;
	CachedPawn = nullptr;
	CachedPlayerController = nullptr;
	bIsWidgetInitialized = false;

	Super::NativeDestruct();
}

void UMHUserWidgetBase::RefreshWidgetReferences()
{
	CachedPlayerController = GetOwningPlayer();
	CachedPawn = nullptr;
	CachedPlayerState = nullptr;
	CachedAbilitySystemComponent = nullptr;

	if (!CachedPlayerController)
	{
		return;
	}

	CachedPawn = CachedPlayerController->GetPawn();
	CachedPlayerState = CachedPlayerController->PlayerState;
	CachedAbilitySystemComponent = ResolveAbilitySystemComponent();
}

void UMHUserWidgetBase::HandleWidgetInitialized()
{
	// 베이스 클래스에서는 비워둔다.
}

UAbilitySystemComponent* UMHUserWidgetBase::ResolveAbilitySystemComponent() const
{
	// 1) Pawn에서 ASC 탐색
	if (CachedPawn && CachedPawn->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(CachedPawn))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				return ASC;
			}
		}
	}

	// 2) PlayerState에서 ASC 탐색
	if (CachedPlayerState && CachedPlayerState->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (const IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(CachedPlayerState))
		{
			if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
			{
				return ASC;
			}
		}
	}

	return nullptr;
}

APlayerController* UMHUserWidgetBase::GetMHPlayerController() const
{
	return CachedPlayerController.Get();
}

APawn* UMHUserWidgetBase::GetMHPawn() const
{
	return CachedPawn.Get();
}

APlayerState* UMHUserWidgetBase::GetMHPlayerState() const
{
	return CachedPlayerState.Get();
}

UAbilitySystemComponent* UMHUserWidgetBase::GetMHAbilitySystemComponent() const
{
	return CachedAbilitySystemComponent.Get();
}

bool UMHUserWidgetBase::HasValidPlayerContext() const
{
	return CachedPlayerController != nullptr;
}

bool UMHUserWidgetBase::HasValidAbilitySystemComponent() const
{
	return CachedAbilitySystemComponent != nullptr;
}