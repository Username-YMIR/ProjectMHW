// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-05


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MHCharacterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHCharacterBase, Log, All)


// ============= 전방 선언 =============
class UAbilitySystemComponent;
class UMHPlayerAttributeSet;
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
    
    // property 변경 visibleanywhere 은 bp에서 사용시 위험
    // EditAnywhere << 사용 권장 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS" , meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataAsset> GASAsset; // 스타트업 데이터
    
    // 레거시
    // UPROPERTY()
    // TObjectPtr<UAttributeSet> AttributeSet; // 스탯
    
    
    UPROPERTY(BlueprintReadOnly)
    bool bGASInitialized = false; // 초기화 여부
    
};
