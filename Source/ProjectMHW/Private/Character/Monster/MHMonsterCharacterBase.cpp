// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-05

#include "Character/Monster/MHMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "DataAsset/MHMonsterDataAsset.h"



DEFINE_LOG_CATEGORY(MonsterCharacter)

AMHMonsterCharacterBase::AMHMonsterCharacterBase()
{
    MonsterAttributes  = CreateDefaultSubobject<UMHMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
    AttributeSet = MonsterAttributes;
}

void AMHMonsterCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    InitMonsterGAS();
}


void AMHMonsterCharacterBase::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;
}

void AMHMonsterCharacterBase::EnterCombat()
{
    bInCombat = true;
}

void AMHMonsterCharacterBase::ExitCombat()
{
    bInCombat = false;
    CombatTarget = nullptr;
}

void AMHMonsterCharacterBase::InitMonsterGAS()
{
    // INIT GAS
    
    if (bGASInitialized)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(": MonsterCharacter Base InitMonsterGAS bGASInitialized"));
        return;
    }
    
    if (!AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base InitMonsterGAS AbilitySystemComponent"));
        return; 
    }
    
    AbilitySystemComponent->InitAbilityActorInfo(this , this);
    
    ApplyStartupLooseTags();
    GrantStartupAbilities();
    ApplyStartupEffects();
    
    bGASInitialized = true;
}

void AMHMonsterCharacterBase::ApplyStartupLooseTags()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(": MonsterCharacter ApplyStartup "))
        return;
    }
    
    for (const FGameplayTag& Tag : MonsterDataAsset->MonsterTags)
    {
        AbilitySystemComponent->AddLooseGameplayTag(Tag);
    }
    
    
}

void AMHMonsterCharacterBase::GrantStartupAbilities()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter GrantStartupAbilities "))
        return;
    }
    
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : MonsterDataAsset->StartupAbilities)
    {
        if (!AbilityClass )
        {
            UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter GrantStartupAbilities2 "))
        }
        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        
    }
    
}

void AMHMonsterCharacterBase::ApplyStartupEffects()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter ApplyStartupEffects "))
        return;
    }
    
    for (const TSubclassOf<UGameplayEffect>& EffectClass : MonsterDataAsset->StartupEffects)
    {
        if (!EffectClass ) continue;
        
        FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);
        
        FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass , 1.f , ContextHandle );
        
        if (SpecHandle.IsValid())
        {
            UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter SpecHandle "))
            
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            
        }
        
        
    }
    
    
}
