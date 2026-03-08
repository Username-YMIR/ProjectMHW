#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MHAnimNotify_WeaponSocketSwap.generated.h"

// 무기 소켓 이동 타입
UENUM(BlueprintType)
enum class EMHWeaponSocketSwapTarget : uint8
{
    ToHand  UMETA(DisplayName = "ToHand"), // 손 소켓으로 이동
    ToBack  UMETA(DisplayName = "ToBack"), // 등 소켓으로 이동
};

/**
 * 무기 메쉬를 손/등 소켓으로 이동시키는 노티파이
 */
UCLASS()
class PROJECTMHW_API UMHAnimNotify_WeaponSocketSwap : public UAnimNotify
{
    GENERATED_BODY()

public:
    // 노티파이 실행
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override; // 손승우 수정

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
    EMHWeaponSocketSwapTarget Target = EMHWeaponSocketSwapTarget::ToHand; // 이동 대상
};
