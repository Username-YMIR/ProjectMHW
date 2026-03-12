#include "Character/MHCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Combat/Attributes/MHPlayerAttributeSet.h"

DEFINE_LOG_CATEGORY(LogMHCharacterBase)


AMHCharacterBase::AMHCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    
}

UAbilitySystemComponent* AMHCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AMHCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMHCharacterBase, Verbose, TEXT("%s : BeginPlay"), *GetName());
}

void AMHCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitializeAbilitySystem();
}

void AMHCharacterBase::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    InitializeAbilitySystem();
}

void AMHCharacterBase::InitializeAbilitySystem()
{
    if (bGASInitialized)
    {
        return;
    }

    if (!AbilitySystemComponent)
    {
        UE_LOG(LogMHCharacterBase, Error, TEXT("%s : AbilitySystemComponent is null"), *GetName());
        return;
    }

    // GAS ActorInfo 초기화
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    bGASInitialized = true;

    UE_LOG(LogMHCharacterBase, Log, TEXT("%s : GAS Initialized"), *GetName());
    
}
