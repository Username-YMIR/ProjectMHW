# Frontend Menu and Loading Planning

## Scope
This document records the planned frontend flow for weapon selection and pre-battle loading in the current codebase.

The goal is to:
- show a main menu widget in an empty frontend level
- let the player select a weapon from an array of `AMHWeaponInstance` classes
- resolve each slot icon from `ItemRegistry + ItemDataKey`
- carry the selected weapon into the battle level
- finish preload work before opening the battle level
- show loading progress on a progress bar

## Confirmed Rules
- The main menu is shown in an empty level.
- Weapon slots are configured from an array of weapon instance classes.
- The first valid slot is selected by default.
- Clicking a slot changes the selected weapon.
- The selected weapon becomes the player's runtime equipped weapon when the battle level starts.
- Loading is completed before the battle level is opened.
- The loading screen must show progress on a progress bar.

## Current Code Anchors
- `Source/ProjectMHW/Public/Widgets/MHItemSlotWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHItemSlotWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHItemSelectionWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHItemSelectionWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHUserWidgetBase.h`
- `Source/ProjectMHW/Private/Widgets/MHUserWidgetBase.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHItemInstanceBase.h`
- `Source/ProjectMHW/Private/Items/Instance/MHItemInstanceBase.cpp`
- `Source/ProjectMHW/Public/Items/Data/MHItemDataBase.h`
- `Source/ProjectMHW/Public/Items/Data/ItemDataRegistry.h`
- `Source/ProjectMHW/Public/Items/Instance/MHWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHWeaponInstance.cpp`
- `Source/ProjectMHW/Public/Character/Player/MHPlayerCharacter.h`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Variant_Combat/CombatPlayerController.h`
- `Source/ProjectMHW/Variant_Combat/CombatPlayerController.cpp`

## Current Runtime Model
### UI
- `UMHItemSlotWidget` already owns slot selection state and icon brush display.
- `UMHItemSelectionWidget` already demonstrates the local pattern of binding widget state to gameplay state and refreshing selected visuals.
- `UMHUserWidgetBase` already provides a common widget lifecycle and cached player references.

### Item Data
- `AMHItemInstanceBase` stores `ItemDataKey`, `ItemRegistry`, and cached item data.
- `UMHItemDataBase` already owns a soft `Icon` reference that can drive menu slot visuals.
- `AMHWeaponInstance` already inherits the item data path and is the correct slot source type.

### Weapon Equip
- `AMHPlayerCharacter::BeginPlay()` currently calls `SpawnAndEquipDefaultWeapon()`.
- `SpawnAndEquipDefaultWeapon()` currently spawns `DefaultWeaponClass`.
- `EquipWeaponInstance()` already performs the actual runtime equip path.

## Design Decision
The new frontend flow should be centered on:
- a dedicated frontend `PlayerController`
- a frontend/session `GameInstance`
- a main menu widget
- a loading widget
- the existing `AMHPlayerCharacter` equip path

This is lower risk than pushing the logic into `GameMode`, because the flow is primarily local-player UI, data handoff, and level transition.

## Proposed Runtime Model
### `AFrontendPlayerController`
- Creates the main menu widget in the empty frontend level.
- Switches from menu UI to loading UI when the player presses Start.
- Sets UI-only input mode and mouse cursor behavior.

### `UMHMainMenuWidget`
- Owns the configured array of `TSubclassOf<AMHWeaponInstance>`.
- Builds slot widgets dynamically.
- Resolves each slot icon from the weapon class default object.
- Tracks `SelectedSlotIndex`.
- Emits the start request with the currently selected weapon class.

### `UMHLoadingWidget`
- Displays a progress bar and status text.
- Reflects staged preload progress before level open.

### `UMHFrontendGameInstance`
- Stores `PendingWeaponClass`.
- Stores target battle level name.
- Runs preload work before `OpenLevel`.
- Exposes loading progress to the loading widget.

### `AMHPlayerCharacter`
- Keeps the existing spawn/equip path.
- Resolves the weapon class with the following priority:
1. `PendingWeaponClass` from the frontend session state
2. existing `DefaultWeaponClass`

## Required API Adjustments
### `AMHItemInstanceBase`
The menu needs a safe way to resolve slot visuals from a weapon class before runtime spawn.

Required additions:
- `GetItemDataKey() const`
- `GetItemRegistry() const`

Recommended alternative:
- a small helper that resolves preview item data directly from the class default object

Reason:
- `ItemDataKey` and `ItemRegistry` are protected today, so external menu code cannot read them directly.

### `AMHPlayerCharacter`
`SpawnAndEquipDefaultWeapon()` needs a small change so it can consume the pending selection from `GameInstance` before falling back to `DefaultWeaponClass`.

## Main Menu Widget Plan
### Data
- `WeaponSlotClasses`
- `SelectedSlotIndex`
- `SlotWidgetClass`
- `WeaponSlotContainer`
- `StartButton`

### Build Flow
1. On construct, validate the configured slot class array.
2. For each weapon class, get the class default object.
3. Resolve `ItemRegistry + ItemDataKey`.
4. Find `UMHItemDataBase`.
5. Resolve `Icon`.
6. Build a brush and push it into `UMHItemSlotWidget::SetItemIconBrush()`.
7. Bind click behavior per slot.
8. Select the first valid slot by default.

### Selection Rules
- The menu should not allow a deselected state.
- The first valid slot is selected automatically.
- Clicking another slot moves the selection.
- If the slot array is empty, Start is disabled.

### Recommended Empty Selection Policy
- Do not support manual deselect.
- If future UX requires deselect, Start should auto-select the first valid slot before transition.

## Loading Screen Plan
### Core Rule
Loading must finish before the battle level is opened.

### Implementation Rule
Use staged preload progress instead of trying to display the internal map open percent directly.

Reason:
- `OpenLevel` does not provide a stable gameplay-facing percent that maps cleanly to a UMG progress bar in this flow.
- The requirement is satisfied if required assets are preloaded first and the level is only opened after preload completion.

### Progress Model
Recommended staged model:
- `0.0` menu accepted, loading screen shown
- `0.1` selected weapon validated
- `0.2 - 0.9` soft asset preload in progress
- `1.0` preload finished, ready to open battle level

### Preload Targets
- selected weapon class
- selected weapon icon if still needed by the loading UI
- battle HUD classes if configured as soft references
- any other required soft references needed for the first frame of battle entry

## Battle Entry Plan
1. Menu confirms the selected slot.
2. `GameInstance` stores `PendingWeaponClass`.
3. Loading widget is shown.
4. Preload begins.
5. Loading progress is updated.
6. Preload completes.
7. `OpenLevel(BattleLevel)` is called.
8. The battle level spawns the player as usual.
9. `AMHPlayerCharacter::BeginPlay()` calls `SpawnAndEquipDefaultWeapon()`.
10. `SpawnAndEquipDefaultWeapon()` prefers `PendingWeaponClass`.
11. `EquipWeaponInstance()` performs the existing runtime equip flow.

## Error Handling
### No weapon slots configured
- Disable Start.
- Show a simple status message.

### Slot icon data missing
- Keep the slot usable.
- Show a fallback brush or empty icon.

### Pending weapon class invalid
- Fall back to `DefaultWeaponClass`.
- Log a warning.

### Both pending and default weapon classes invalid
- Start without a weapon and log a warning.

## UI Responsibilities
### Main Menu
- title and background
- weapon slot list
- current selection text
- start button

### Loading Screen
- full-screen overlay
- progress bar
- loading status text

## Implementation Order
1. Add frontend planning docs and UML docs.
2. Add the missing preview accessors to `AMHItemInstanceBase`.
3. Add the frontend/session `GameInstance`.
4. Add the frontend `PlayerController`.
5. Add `UMHMainMenuWidget`.
6. Add `UMHLoadingWidget`.
7. Update `AMHPlayerCharacter::SpawnAndEquipDefaultWeapon()` to consume the pending selection.
8. Create the frontend level and widget blueprints.

## Out of Scope for the First Pass
- save/load persistence for the last selected weapon
- multiplayer synchronization
- a true streamed transition map flow
- animation-rich carousel presentation

## Summary
The minimum-risk implementation is:
- empty frontend level
- dedicated frontend `PlayerController`
- `UMHMainMenuWidget` built from weapon class array
- slot icons resolved through existing item data
- `GameInstance` stores selected weapon class
- staged preload shown on `UMHLoadingWidget`
- battle level opens only after preload completes
- `AMHPlayerCharacter` equips the selected weapon through the existing equip path
