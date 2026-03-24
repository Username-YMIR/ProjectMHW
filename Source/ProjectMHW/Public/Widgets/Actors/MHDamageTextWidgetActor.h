// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "GameFramework/Actor.h"
#include "MHDamageTextWidgetActor.generated.h"

class UWidgetAnimation;
class UWidgetComponent;
class UMHDamageTextWidget;

UCLASS()
class PROJECTMHW_API AMHDamageTextWidgetActor : public AActor
{
	GENERATED_BODY()

public:
	AMHDamageTextWidgetActor();

    void InitDamage(const FMHDamageTextPayload& InPayload);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
    void EnsureWidgetInitialized();

    void ApplyPayloadToWidget();

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UWidgetComponent> WidgetComponent = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "DamageText")
    TSubclassOf<UMHDamageTextWidget> DamageTextWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UMHDamageTextWidget> DamageWidget = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UWidgetAnimation> DamageTextAnimation = nullptr;

    UPROPERTY(Transient)
    FMHDamageTextPayload CachedPayload;

    UPROPERTY(EditDefaultsOnly, Category = "DamageText")
    float FallbackLifetime = 2.0f;

    UPROPERTY(Transient)
    float ElapsedTime = 0.0f;

    UPROPERTY(Transient)
    float ResolvedLifetime = 2.0f;

    UPROPERTY(Transient)
    bool bHasPayload = false;
};
