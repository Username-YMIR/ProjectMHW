#include "Animation/mh_player_anim_instance.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Items/Instance/MHLongSwordInstance.h"
#include "Weapons/LongSword/MHLongSwordComboComponent.h"

DEFINE_LOG_CATEGORY(LogMHPlayerAnimInstance);

void UMHPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    AMHPlayerCharacter* PlayerCharacter = GetMHPlayerCharacter();
    if (!PlayerCharacter)
    {
        CurrentMoveTag = FGameplayTag();
        return;
    }

    CurrentMoveTag = FGameplayTag();

    // 현재는 태도 전투의 MoveTag만 읽어오며,
    // 차지액스는 추후 동일한 방식으로 확장할 수 있게 자리를 비워둔다.
    if (AMHLongSwordInstance* LongSword = Cast<AMHLongSwordInstance>(PlayerCharacter->GetEquippedWeapon()))
    {
        if (UMHLongSwordComboComponent* ComboComp = LongSword->GetComboComponent())
        {
            CurrentMoveTag = ComboComp->GetCurrentMoveTag();
        }
    }
}
