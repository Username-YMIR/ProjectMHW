#include "Audio/MHBGMTriggerBox.h"

#include "AudioDevice.h"
#include "Character/Monster/MHMonsterCharacterBase.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "World/MHWorldSettings.h"

DEFINE_LOG_CATEGORY(LogMHBGMTriggerBox);

AMHBGMTriggerBox::AMHBGMTriggerBox()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBoxBeginOverlap);

    BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
    BGMComponent->SetupAttachment(RootComponent);
    BGMComponent->bAutoActivate = false;
    BGMComponent->bIsUISound = true;
    BGMComponent->bAllowSpatialization = false;
}

void AMHBGMTriggerBox::BeginPlay()
{
    Super::BeginPlay();

    BindTargetMonsterDelegates();
    RequestBGMState(EMHBGMTriggerState::Default);
}

void AMHBGMTriggerBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(BGMTransitionTimer);
    UnbindTargetMonsterDelegates();

    Super::EndPlay(EndPlayReason);
}

void AMHBGMTriggerBox::HandleTriggerBoxBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!IsValid(PlayerCharacter) || !PlayerCharacter->IsPlayerControlled())
    {
        return;
    }

    if (bCombatTriggered)
    {
        return;
    }

    bCombatTriggered = true;

    UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : Combat BGM trigger activated by %s"), *GetName(), *GetNameSafe(PlayerCharacter));

    RequestBGMState(EMHBGMTriggerState::Combat);
    EvaluateClearState();
}

void AMHBGMTriggerBox::HandleTargetMonsterDied(AMHMonsterCharacterBase* DeadMonster)
{
    UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : Target monster died -> %s"), *GetName(), *GetNameSafe(DeadMonster));

    EvaluateClearState();
}

void AMHBGMTriggerBox::BindTargetMonsterDelegates()
{
    UnbindTargetMonsterDelegates();

    const UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    const AMHWorldSettings* MHWorldSettings = Cast<AMHWorldSettings>(World->GetWorldSettings());
    if (!IsValid(MHWorldSettings))
    {
        UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : MHWorldSettings is not applied to this level"), *GetName());
        return;
    }

    const TArray<TObjectPtr<AMHMonsterCharacterBase>>& TargetMonsters = MHWorldSettings->GetBGMClearTargetMonsters();
    for (AMHMonsterCharacterBase* TargetMonster : TargetMonsters)
    {
        if (!IsValid(TargetMonster))
        {
            continue;
        }

        bool bAlreadyBound = false;
        for (const TWeakObjectPtr<AMHMonsterCharacterBase>& BoundMonster : BoundTargetMonsters)
        {
            if (BoundMonster.Get() == TargetMonster)
            {
                bAlreadyBound = true;
                break;
            }
        }

        if (bAlreadyBound)
        {
            continue;
        }

        TargetMonster->OnMonsterDied.RemoveDynamic(this, &ThisClass::HandleTargetMonsterDied);
        TargetMonster->OnMonsterDied.AddDynamic(this, &ThisClass::HandleTargetMonsterDied);
        BoundTargetMonsters.Add(TargetMonster);
    }

    UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : Bound %d target monsters from world settings"), *GetName(), BoundTargetMonsters.Num());
}

void AMHBGMTriggerBox::UnbindTargetMonsterDelegates()
{
    for (const TWeakObjectPtr<AMHMonsterCharacterBase>& BoundMonster : BoundTargetMonsters)
    {
        if (!BoundMonster.IsValid())
        {
            continue;
        }

        BoundMonster->OnMonsterDied.RemoveDynamic(this, &ThisClass::HandleTargetMonsterDied);
    }

    BoundTargetMonsters.Reset();
}

void AMHBGMTriggerBox::EvaluateClearState()
{
    if (!bCombatTriggered)
    {
        return;
    }

    if (CurrentBGMState == EMHBGMTriggerState::Clear)
    {
        return;
    }

    if (!AreAllTargetMonstersDead())
    {
        return;
    }

    UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : All target monsters are dead -> Clear BGM"), *GetName());

    RequestBGMState(EMHBGMTriggerState::Clear);
}

void AMHBGMTriggerBox::RequestBGMState(EMHBGMTriggerState NewState)
{
    USoundBase* NextTrack = ResolveSoundForState(NewState);
    if (!IsValid(NextTrack))
    {
        UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : No track assigned for state %d"), *GetName(), static_cast<int32>(NewState));
        return;
    }

    if (CurrentBGMState == NewState && BGMComponent->IsPlaying() && BGMComponent->Sound == NextTrack)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(BGMTransitionTimer);

    if (BGMComponent->IsPlaying() && BGMComponent->Sound != nullptr && BGMComponent->Sound != NextTrack && TransitionFadeDuration > 0.0f)
    {
        PendingTrack = NextTrack;
        PendingBGMState = NewState;

        BGMComponent->FadeOut(TransitionFadeDuration, 0.0f);
        GetWorldTimerManager().SetTimer(BGMTransitionTimer, this, &ThisClass::PlayPendingTrack, TransitionFadeDuration, false);

        UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : Queue BGM transition %d"), *GetName(), static_cast<int32>(NewState));
        return;
    }

    PlayTrackImmediately(NextTrack, NewState);
}

void AMHBGMTriggerBox::PlayPendingTrack()
{
    if (!IsValid(PendingTrack))
    {
        return;
    }

    PlayTrackImmediately(PendingTrack, PendingBGMState);
    PendingTrack = nullptr;
}

void AMHBGMTriggerBox::PlayTrackImmediately(USoundBase* InSound, EMHBGMTriggerState InState)
{
    if (!IsValid(InSound))
    {
        return;
    }

    const bool bInitialPlayback = !BGMComponent->IsPlaying() && CurrentBGMState == EMHBGMTriggerState::Default && InState == EMHBGMTriggerState::Default;
    const float FadeDuration = bInitialPlayback ? InitialFadeInDuration : TransitionFadeDuration;

    BGMComponent->Stop();
    BGMComponent->SetSound(InSound);
    CurrentBGMState = InState;

    if (FadeDuration > 0.0f)
    {
        BGMComponent->FadeIn(FadeDuration, 1.0f);
    }
    else
    {
        BGMComponent->Play();
    }

    UE_LOG(LogMHBGMTriggerBox, Warning, TEXT("%s : Play BGM state=%d sound=%s"), *GetName(), static_cast<int32>(InState), *GetNameSafe(InSound));
}

USoundBase* AMHBGMTriggerBox::ResolveSoundForState(EMHBGMTriggerState InState) const
{
    switch (InState)
    {
    case EMHBGMTriggerState::Combat:
        return CombatBGM;

    case EMHBGMTriggerState::Clear:
        return ClearBGM;

    case EMHBGMTriggerState::Default:
    default:
        return DefaultBGM;
    }
}

bool AMHBGMTriggerBox::AreAllTargetMonstersDead() const
{
    if (BoundTargetMonsters.Num() == 0)
    {
        return false;
    }

    for (const TWeakObjectPtr<AMHMonsterCharacterBase>& BoundMonster : BoundTargetMonsters)
    {
        if (!BoundMonster.IsValid())
        {
            continue;
        }

        if (!BoundMonster->HasMonsterDied())
        {
            return false;
        }
    }

    return true;
}

