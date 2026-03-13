// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-05


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/MHDamageSpecReceiverInterface.h"
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
class PROJECTMHW_API AMHCharacterBase 
    : public ACharacter
    , public IAbilitySystemInterface
    , public IMHDamageSpecReceiverInterface
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
    
    
#pragma region DamageSystem
public:
    virtual FMHHitAcknowledge ReceiveDamageSpec_Implementation(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FGameplayEffectSpecHandle& DamageSpecHandle,
        const FHitResult& HitResult
    ) override;

protected:
    /** 전달받은 DamageSpec이 유효한지 검사 */
    virtual bool ValidateDamageSpec(
        const FGameplayEffectSpecHandle& DamageSpecHandle
    ) const;

    /** 현재 이 캐릭터가 피격 가능한 상태인지 검사 */
    virtual bool CanReceiveDamage(
        AActor* SourceActor,
        FGameplayTag AttackTag,
        const FGameplayEffectSpecHandle& DamageSpecHandle,
        const FHitResult& HitResult
    ) const;

    /** 이 캐릭터의 ASC 반환 */
    virtual UAbilitySystemComponent* GetCharacterASC() const;

    /** 전달받은 DamageSpec을 자신의 ASC에 적용 */
    virtual bool ApplyIncomingDamageSpec(
        const FGameplayEffectSpecHandle& DamageSpecHandle
    );

    /** 대미지 적용 성공 후 후처리 */
    virtual void HandleDamageAccepted(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    );

    /** 대미지 적용 거절 후 후처리 */
    virtual void HandleDamageRejected(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    );

    /** 사망 여부 검사 */
    virtual bool IsDead() const;

    /** 사망 처리 */
    virtual void HandleDeath();

    /** 성공 응답 생성 */
    virtual FMHHitAcknowledge BuildAcceptedHitAcknowledge() const;

    /** 실패 응답 생성 */
    virtual FMHHitAcknowledge BuildRejectedHitAcknowledge() const;
#pragma endregion 
    

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
