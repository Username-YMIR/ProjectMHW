#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponInstance.h"
#include "Items/Data/MHGreatSwordItemData.h"
#include "MHGreatSwordInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGreatSwordInstance, Log, All);

class UMHGreatSwordActionComponent;
class UMHGreatSwordChargeStateComponent;
class UMHGreatSwordComboGraph;

UCLASS()
class PROJECTMHW_API AMHGreatSwordInstance : public AMHMeleeWeaponInstance
{
    GENERATED_BODY()

public:
    AMHGreatSwordInstance();

// ===== Lifecycle =====
protected:
    virtual void ApplyItemData() override;
    virtual void BeginPlay() override;

// ===== GreatSwordObjects =====
#pragma region GreatSwordObjects
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|GreatSword", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMHGreatSwordActionComponent> ActionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|GreatSword", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMHGreatSwordChargeStateComponent> ChargeStateComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GreatSword", meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UMHGreatSwordComboGraph> ComboGraphAsset;

    UPROPERTY(Transient)
    TObjectPtr<UMHGreatSwordComboGraph> RuntimeComboGraph;
#pragma endregion

// ===== Accessors =====
public:
    UMHGreatSwordActionComponent* GetActionComponent() const { return ActionComponent; }
    UMHGreatSwordChargeStateComponent* GetChargeStateComponent() const { return ChargeStateComponent; }
    UMHGreatSwordComboGraph* GetComboGraph() const;

// ===== Internal =====
private:
    FORCEINLINE const UMHGreatSwordItemData* GetGreatSwordData() const
    {
        const UMHGreatSwordItemData* Data = Cast<UMHGreatSwordItemData>(CachedItemData);
        ensure(Data);
        return Data;
    }
};
