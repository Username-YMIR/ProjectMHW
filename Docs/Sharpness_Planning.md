# Sharpness Planning

## Scope
This document records the confirmed Monster Hunter World-style sharpness plan for the current codebase. It is based on the existing runtime flow and the user's confirmed rules.

## Confirmed Rules
- World-style single-color sharpness bar only.
- `UMHProgressBarWidget` owns 6 sharpness colors plus `DefaultFillColor`.
- Sharpness color is resolved from the player via getter, not by expanding the UI delegate payload.
- `CurrentSharpnessLength` is removed.
- `HandleSharpnessBounce()` lives on `AMHPlayerCharacter`.
- Bounce stops only the current weapon attack ability, ends the attack window, stops the current montage, resets combo/action state, invalidates buffered input, and plays one shared full-body bounce montage.
- Bounce means no damage, no hit-confirmed reward, and no camera shake.
- Sharpness refills to `MaxSharpness` immediately at `SharpenStart`.
- Sharpen montage continues unless cancelled.
- Sharpen is cancelled by move, attack, dodge input, or `ReceiveDamageSpec`.
- Cancelled inputs still execute normally after cancelling sharpen.
- `MaxSharpness == 0` or no weapon means progress stays at `0` and the bar uses `DefaultFillColor`.

## Current Code Anchors
- `Source/ProjectMHW/Public/Character/Player/MHPlayerCharacter.h`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Public/Widgets/MHPlayerStatusWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHPlayerStatusWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHProgressBarWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHProgressBarWidget.cpp`
- `Source/ProjectMHW/Public/Type/MHCombatStatStructType.h`
- `Source/ProjectMHW/Public/Combat/Attributes/MHPlayerAttributeSet.h`
- `Source/ProjectMHW/Private/Combat/Attributes/MHPlayerAttributeSet.cpp`
- `Source/ProjectMHW/Public/Combat/Attributes/MHCombatAttributeSet.h`
- `Source/ProjectMHW/Private/Combat/Attributes/MHCombatAttributeSet.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHMeleeWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHMeleeWeaponInstance.cpp`
- `Source/ProjectMHW/Public/AbilitySystem/Abilities/Status/MHGA_MHSharpen.h`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Status/MHGA_MHSharpen.cpp`
- `Source/ProjectMHW/Public/AbilitySystem/Abilities/Status/MHGA_Potion.h`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Status/MHGA_Potion.cpp`
- `Source/ProjectMHW/Public/Interfaces/MHDamageSpecReceiverInterface.h`
- `Source/ProjectMHW/Private/Combat/Execution/MHDamageExecutionCalculation.cpp`
- `Source/ProjectMHW/Private/Character/Monster/MHMonsterCharacterBase.cpp`

## Runtime Model
### Data
- `FMHSharpnessData` defines per-color lengths.
- `FMHAttackStats` remains the weapon data container for `SharpnessLength` and `MaxSharpnessColor`.
- `EMHSharpnessColor` remains the sharpness phase enum.

### Player State
- `AMHPlayerCharacter` remains the runtime source of truth for sharpness color, current sharpness value, and bounce/sharpen behavior.
- `UMHPlayerAttributeSet` holds `Sharpness` and `MaxSharpness`.
- `UMHCombatAttributeSet` holds `SharpnessModifier`.

### UI
- `UMHPlayerStatusWidget` continues to receive sharpness updates through `OnSharpnessChanged`.
- `UMHProgressBarWidget` stays the bar control; it now needs fill tint logic for sharpness and a fallback tint for non-sharpness uses.

## Required Changes
### `UMHProgressBarWidget`
- Add `DefaultFillColor`.
- Add 6 editable sharpness colors in C++/BP.
- Add a setter that receives `EMHSharpnessColor` and applies the matching tint.
- Keep the percent logic unchanged.

### `AMHPlayerCharacter`
- Remove `CurrentSharpnessLength`.
- Add `GetCurrentSharpnessColor()` for UI.
- Keep the sharpness modifier update path centered on `GetSharpnessMultiplier()`.
- Add `HandleSharpnessBounce()` and route weapon-type-specific cleanup through it.
- Add a shared bounce montage reference as `EditDefaultsOnly`.

### `AMHMeleeWeaponInstance`
- On bounce, call the player bounce handler only.
- Do not apply damage, hit-confirmed reward, or camera shake on bounce.

### `UGA_MHSharpen`
- On `SharpenStart`, restore sharpness to max immediately.
- Keep the montage running until cancelled or finished.
- End the ability when the hold condition is released or cancellation conditions occur.

## UI Plan
### Minimum Risk Path
- Keep `UMHPlayerStatusWidget` as the owner of the sharpness bar slot.
- Keep `UMHProgressBarWidget` as the actual bar control.
- Add a color setter and a default tint fallback.
- Update percent from `CurrentSharpness / MaxSharpness`.
- Set tint from `AMHPlayerCharacter::GetCurrentSharpnessColor()`.

### Behavior Rules
- If the player has no weapon or `MaxSharpness == 0`, set percent to `0` and tint to `DefaultFillColor`.
- Otherwise, set percent to the current ratio and tint to the resolved sharpness color.

## Bounce Flow
1. `AMHMeleeWeaponInstance::OnWeaponBeginOverlap()` confirms the overlap.
2. If `AMHPlayerCharacter::HandleWeaponAttackHit()` returns bounce, stop the attack window and call `HandleSharpnessBounce()`.
3. `HandleSharpnessBounce()` stops the current attack ability, closes the current attack window, stops the current montage, resets combo/action state, invalidates buffered input, and plays the shared bounce montage.
4. No damage, reward, or camera shake is produced.

## Sharpen Flow
1. Item use starts from input and activates `UGA_MHSharpen`.
2. `SharpenStart` immediately fills sharpness to max.
3. The montage keeps running.
4. Move, attack, dodge, or damage receipt cancels sharpen.
5. On cancel, current input still executes normally after the sharpen cancel path.

## Implementation Order
1. Add color/tint support to `UMHProgressBarWidget`.
2. Remove `CurrentSharpnessLength` and expose the color getter on `AMHPlayerCharacter`.
3. Add `HandleSharpnessBounce()` and the shared bounce montage reference.
4. Wire bounce behavior through `AMHMeleeWeaponInstance`.
5. Make sharpen restore to max at `SharpenStart`.
6. Apply the UI fallback rules for zero sharpness and no-weapon states.
