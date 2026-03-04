#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MHCharacterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCharacterBase, Log, All)


// ============= 전방 선언 =============
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UDataAsset;
// ====================================

UCLASS()
class PROJECTMHW_API AMHCharacterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AMHCharacterBase();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS" , meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset> GASAsset;
    
    UPROPERTY()
    TObjectPtr<UAttributeSet> EnemyAttributeSet;
    
    UPROPERTY(BlueprintReadOnly)
    bool bGASInitialized = false;
    
};
