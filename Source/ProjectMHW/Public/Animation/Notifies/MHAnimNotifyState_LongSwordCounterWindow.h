#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "MHAnimNotifyState_LongSwordCounterWindow.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHANSLongSwordCounterWindow, Log, All)

class USkeletalMeshComponent;

UCLASS(meta = (DisplayName = "MH LongSword Counter Window"))
class PROJECTMHW_API UMHAnimNotifyState_LongSwordCounterWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Counter")
    EMHLongSwordCounterWindowType CounterWindowType = EMHLongSwordCounterWindowType::Foresight;

private:
    AMHPlayerCharacter* ResolvePlayerCharacter(USkeletalMeshComponent* MeshComp) const;
};
