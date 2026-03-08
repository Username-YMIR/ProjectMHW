#include "Animation/Notifies/MHAnimNotify_WeaponSocketSwap.h"

#include "Components/SkeletalMeshComponent.h"
#include "Character/Player/MHPlayerCharacter.h"

void UMHAnimNotify_WeaponSocketSwap::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference); // 손승우 수정

    if (!MeshComp)
    {
        return;
    }

    AMHPlayerCharacter* PlayerCharacter = Cast<AMHPlayerCharacter>(MeshComp->GetOwner());
    if (!PlayerCharacter)
    {
        return;
    }

    // 지정된 대상에 따라 소켓 이동
    switch (Target)
    {
    case EMHWeaponSocketSwapTarget::ToHand:
        PlayerCharacter->Notify_AttachWeaponToHand();
        break;
    case EMHWeaponSocketSwapTarget::ToBack:
        PlayerCharacter->Notify_AttachWeaponToBack();
        break;
    default:
        break;
    }
}
