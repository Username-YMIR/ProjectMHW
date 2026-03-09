// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-06


#include "Character/Monster/MHMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "DataAsset/MHMonsterDataAsset.h"
#include "Kismet/GameplayStatics.h"

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

    
    GetWorldTimerManager().SetTimer(
        RoarCheckTimer,
        this,
        &AMHMonsterCharacterBase::CheckRoar,
        0.2f,
        true
    );
    
}

void AMHMonsterCharacterBase::CheckRoar()
{
    //bCanRoar
    if (bHasRoared)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base CheckRoar !bCanRoar"));
        return;
    }
    //RoarMontage
    if (!RoarMontage)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base CheckRoar !RoarMontage"));
        return;
    }
    
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this , 0);
    //PlayerCharacter
    if (!PlayerCharacter)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base CheckRoar !PlayerCharacter"));
        return;
    }
    
    const float Dist = FVector::Dist(PlayerCharacter->GetActorLocation(),GetActorLocation());
    if (Dist > RoarTriggerDistance)
    {
        /*UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base CheckRoar Dist>RoarTriggerDistance"));*/
        return;
    }
    
    // 몽타주 재생 
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
        {
            // 중복체크
            if (!Anim->Montage_IsPlaying(RoarMontage))
            {
                                // 몽타주 재생 몽타주종류 , 재생 속도
                Anim->Montage_Play(RoarMontage , 1.0f);
                bHasRoared = true;
                GetWorldTimerManager().ClearTimer(RoarCheckTimer);
            }
            
        }
    }
    
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
    

    if (bMonsterGASInitialized)
    {
        UE_LOG(MonsterCharacter , Warning , TEXT(": MonsterCharacter Base InitMonsterGAS bMonsterGASInitialized"));

   
    
        if (!AbilitySystemComponent)
        {
            UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter Base InitMonsterGAS AbilitySystemComponent"));
            return; 
        }
    
        AbilitySystemComponent->InitAbilityActorInfo(this , this);
    
        ApplyStartupLooseTags();
        GrantStartupAbilities();
        ApplyStartupEffects();
    
        bMonsterGASInitialized = true;
        if (MonsterAttributes)
        {
            UE_LOG(LogTemp, Warning, TEXT("[MonsterGAS] %s HP=%f/%f  Poise=%f/%f  Atk=%f Def=%f"),
                *GetName(),
                MonsterAttributes->GetHealth(), MonsterAttributes->GetMaxHealth(),
                MonsterAttributes->GetPoise(), MonsterAttributes->GetMaxPoise(),
                MonsterAttributes->GetAttackPower(), MonsterAttributes->GetDefense()
            );
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[MonsterGAS] %s MonsterAttributes is NULL"), *GetName());
        }
        
        ApplyStartupEffects();
    
        bGASInitialized = true;
    }
}

    void AMHMonsterCharacterBase::ApplyStartupLooseTags()
    {
        UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
        if (!MonsterDataAsset || !AbilitySystemComponent)
        {
            UE_LOG(MonsterCharacter , Warning , TEXT(": MonsterCharacter ApplyStartup "));
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
            UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter GrantStartupAbilities "));

            return;
        }
    
        for (const TSubclassOf<UGameplayAbility>& AbilityClass : MonsterDataAsset->StartupAbilities)
        {
            if (!AbilityClass )
            {
                UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter GrantStartupAbilities2 "));
            }
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        
        }
    
    }

    void AMHMonsterCharacterBase::ApplyStartupEffects()
    {
        UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
        if (!MonsterDataAsset || !AbilitySystemComponent)
        {
            UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter ApplyStartupEffects "));
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
                UE_LOG(MonsterCharacter , Warning , TEXT(" : MonsterCharacter SpecHandle "));
            
                AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            
            }
        
        
        }
    }

