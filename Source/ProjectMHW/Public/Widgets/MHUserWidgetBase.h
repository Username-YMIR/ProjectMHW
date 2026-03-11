// Fill out your copyright notice in the Description page of Project Settings.

// 제작자 : 이건주
// 제작일 : 2026-03-11
// 수정자 : 이건주
// 수정일 : 2026-03-11 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHUserWidgetBase.generated.h"

class APlayerController;
class APawn;
class APlayerState;
class UAbilitySystemComponent;

/**
 * 모든 MH UI 위젯의 공통 베이스 클래스
 *
 * 책임:
 * - 공통 초기화 라이프사이클 제공
 * - Owning Player / Pawn / PlayerState / ASC 접근 헬퍼 제공
 * - 공통 캐시 갱신 기능 제공
 *
 * 비책임:
 * - 특정 HUD 로직
 * - 상태바 전용 로직
 * - 아이템 슬롯 전용 로직
 */
UCLASS(Abstract, Blueprintable)
class PROJECTMHW_API UMHUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UMHUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 위젯이 참조하는 플레이어 관련 객체 캐시를 갱신한다.
	 * PlayerController / Pawn / PlayerState / ASC를 다시 찾아 캐시한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "MH|Widget")
	virtual void RefreshWidgetReferences();

	/**
	 * 공통 초기화 완료 후 파생 클래스가 확장할 수 있는 훅
	 */
	virtual void HandleWidgetInitialized();

	/**
	 * Pawn 또는 PlayerState에서 ASC를 찾는다.
	 */
	virtual UAbilitySystemComponent* ResolveAbilitySystemComponent() const;

public:
	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	APlayerController* GetMHPlayerController() const;

	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	APawn* GetMHPawn() const;

	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	APlayerState* GetMHPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	UAbilitySystemComponent* GetMHAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	bool HasValidPlayerContext() const;

	UFUNCTION(BlueprintPure, Category = "MH|Widget")
	bool HasValidAbilitySystemComponent() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "MH|Widget")
	void OnWidgetInitialized();

protected:
	/** 위젯 공통 초기화 수행 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MH|Widget")
	bool bIsWidgetInitialized = false;

	/** Owning Player 캐시 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|Widget")
	TObjectPtr<APlayerController> CachedPlayerController = nullptr;

	/** Owning Player의 Pawn 캐시 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|Widget")
	TObjectPtr<APawn> CachedPawn = nullptr;

	/** Owning Player의 PlayerState 캐시 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|Widget")
	TObjectPtr<APlayerState> CachedPlayerState = nullptr;

	/** Pawn 또는 PlayerState에서 찾은 ASC 캐시 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "MH|Widget")
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent = nullptr;
};