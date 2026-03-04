#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MHCharacterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHCharacterBase, Log, All)


// ============= 전방 선언 =============
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UDataAsset;
class AController;
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

    virtual void PossessedBy(AController* NewController) override;

    virtual void OnRep_PlayerState() override;

    // GAS 초기화
    virtual void InitializeAbilitySystem();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent; // ASC
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS" , meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset> GASAsset; // 스타트업 데이터
    
    UPROPERTY()
    TObjectPtr<UAttributeSet> AttributeSet; // 스탯
    
    UPROPERTY(BlueprintReadOnly)
    bool bGASInitialized = false; // 초기화 여부
    
};
