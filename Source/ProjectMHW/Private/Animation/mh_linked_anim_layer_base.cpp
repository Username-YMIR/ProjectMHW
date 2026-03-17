#include "Animation/mh_linked_anim_layer_base.h"

#include "Animation/mh_player_anim_instance.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY(LogMHLinkedAnimLayerBase);

UMHPlayerAnimInstance* UMHLinkedAnimLayerBase::GetMHPlayerAnimInstance() const
{
    if (const USkeletalMeshComponent* MeshComp = GetOwningComponent())
    {
        return Cast<UMHPlayerAnimInstance>(MeshComp->GetAnimInstance());
    }

    return nullptr;
}
