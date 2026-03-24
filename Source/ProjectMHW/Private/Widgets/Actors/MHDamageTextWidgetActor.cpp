// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Actors/MHDamageTextWidgetActor.h"

#include "Animation/WidgetAnimation.h"
#include "Components/WidgetComponent.h"
#include "Widgets/MHDamageTextWidget.h"

AMHDamageTextWidgetActor::AMHDamageTextWidgetActor()
{
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    bReplicates = false;

    WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
    SetRootComponent(WidgetComponent);

    WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    WidgetComponent->SetDrawAtDesiredSize(true);
    WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WidgetComponent->SetGenerateOverlapEvents(false);

    ResolvedLifetime = FallbackLifetime;
}

void AMHDamageTextWidgetActor::BeginPlay()
{
	Super::BeginPlay();

    EnsureWidgetInitialized();
    ApplyPayloadToWidget();
}

void AMHDamageTextWidgetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!IsValid(DamageWidget) || !IsValid(DamageTextAnimation))
    {
        return;
    }

    ElapsedTime = FMath::Min(ElapsedTime + DeltaTime, ResolvedLifetime);
    DamageWidget->SetAnimationCurrentTime(DamageTextAnimation, ElapsedTime);
}

void AMHDamageTextWidgetActor::InitDamage(const FMHDamageTextPayload& InPayload)
{
    CachedPayload = InPayload;
    bHasPayload = true;

    if (!HasActorBegunPlay())
    {
        return;
    }

    ApplyPayloadToWidget();
}

void AMHDamageTextWidgetActor::EnsureWidgetInitialized()
{
    if (!IsValid(WidgetComponent))
    {
        return;
    }

    if (DamageTextWidgetClass)
    {
        WidgetComponent->SetWidgetClass(DamageTextWidgetClass);
    }

    if (!IsValid(WidgetComponent->GetUserWidgetObject()))
    {
        WidgetComponent->InitWidget();
    }

    if (!IsValid(DamageWidget))
    {
        DamageWidget = Cast<UMHDamageTextWidget>(WidgetComponent->GetUserWidgetObject());
    }
}

void AMHDamageTextWidgetActor::ApplyPayloadToWidget()
{
    if (!bHasPayload)
    {
        return;
    }

    EnsureWidgetInitialized();

    ResolvedLifetime = FallbackLifetime;
    DamageTextAnimation = nullptr;
    ElapsedTime = 0.0f;

    if (IsValid(DamageWidget))
    {
        DamageWidget->ApplyPayload(CachedPayload);
        ResolvedLifetime = DamageWidget->GetLifetime();
        DamageTextAnimation = DamageWidget->GetDamageTextAnimation();
    }

    if (ResolvedLifetime <= 0.0f)
    {
        ResolvedLifetime = FallbackLifetime;
    }

    if (IsValid(DamageWidget) && IsValid(DamageTextAnimation))
    {
        DamageWidget->PlayAnimation(DamageTextAnimation, 0.0f, 1);
        // DamageWidget->PauseAnimation(DamageTextAnimation);
        DamageWidget->SetAnimationCurrentTime(DamageTextAnimation, 0.0f);
    }

    SetLifeSpan(ResolvedLifetime);
}

