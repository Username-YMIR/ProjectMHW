#include "Character/MHCharacterBase.h"

#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY(LogCharacterBase)



AMHCharacterBase::AMHCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AMHCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AMHCharacterBase::BeginPlay()
{
    Super::BeginPlay();
}
