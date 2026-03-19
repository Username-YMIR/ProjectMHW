#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MHAnimNotifyState_DirectionalTurnWindow.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHAnimNotifyStateDirectionalTurnWindow, Log, All);

UCLASS()
class PROJECTMHW_API UMHAnimNotifyState_DirectionalTurnWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    // 노티파이 시작 시점의 기준 방향에서 좌우로 허용할 최대 회전 각도
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
    float MaxYawDeltaDegrees = 45.0f;

    // 회전 보간 속도(도/초). 0 이하면 입력 방향으로 즉시 맞춘다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", UIMin = "0.0"))
    float RotationInterpSpeed = 720.0f;
};
