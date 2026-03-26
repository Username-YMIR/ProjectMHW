# Frontend Weapon Menu Slot Planning

## Scope
This document records the confirmed refactor of the main menu weapon entry layout.

The goal is to replace the previous split layout:
- icon slot widgets in `WeaponSlotContainer`
- separate weapon name text widgets in `WeaponNameSlotContainer`

with one combined weapon menu slot widget per weapon entry.

## Confirmed Rules
- One widget owns the full weapon entry.
- That widget owns icon display, weapon name display, selected state visuals, and click handling.
- The internal button of the combined widget handles click input.
- The combined widget uses:
  - one `UMHItemSlotWidget`
  - one `UTextBlock`
  - one `UButton`
- The main menu uses one `WeaponSlotContainer` only.
- `WeaponSlotContainer` becomes a vertical list container for full entry widgets.
- Entry layout is one horizontal row:
  - left icon
  - right weapon name
  - vertically centered
  - fully clickable
- Selected state means:
  - icon outline enabled
  - weapon name uses highlight color
- Unselected state means:
  - icon outline disabled
  - weapon name uses default color
- `SelectedWeaponText` can remain as the separate currently-selected summary text.
- `WeaponNameSlotContainer` is removed from the active design.

## Current Problem
The previous implementation built icon slots and weapon name texts independently.
That makes alignment and spacing depend on two separate container trees, which caused overlap and drift in the menu layout.

## New Runtime Model
### `UMHWeaponMenuSlotWidget`
- Represents exactly one weapon entry in the main menu.
- Owns the icon slot widget, name text, and button.
- Receives icon/name data from `UMHMainMenuWidget`.
- Broadcasts a click delegate back to the menu.
- Applies selected/unselected style locally.

### `UMHMainMenuWidget`
- Builds only combined entry widgets.
- Stores created combined widgets.
- Tracks selected index.
- Updates only the combined widgets and `SelectedWeaponText`.

## Widget Composition
### Internal Structure
Recommended hierarchy for `WBP_WeaponMenuSlot`:

```text
Button(SlotButton)
- HorizontalBox
  - UMHItemSlotWidget(ItemSlotWidget)
  - TextBlock(WeaponNameText)
```

### Layout Rules
- `SlotButton` covers the full entry width.
- `HorizontalBox` aligns children vertically to center.
- The icon stays fixed-width on the left.
- The weapon name text expands on the right.
- The entry is a single clickable strip.

## Data Flow
1. `UMHMainMenuWidget` iterates over `WeaponSlotClasses`.
2. It resolves preview item data from the weapon class default object.
3. It creates a `UMHWeaponMenuSlotWidget`.
4. It sets:
   - icon brush
   - weapon name
   - selected state
5. The entry widget handles its own visuals.
6. When clicked, the entry widget broadcasts itself back to the menu.
7. `UMHMainMenuWidget` updates selected index and refreshes all entry widgets.

## Main Menu Changes
### Remove
- dynamic creation of separate `UTextBlock` name entries
- `WeaponNameSlotContainer`
- direct use of `UMHItemSlotWidget` as the top-level menu entry widget

### Keep
- `WeaponSlotContainer` binding name
- `SelectedWeaponText`
- Start button hover text styling
- weapon class preview data resolution

### Replace
- `SlotWidgetClass : TSubclassOf<UMHItemSlotWidget>`
with
- `WeaponMenuSlotWidgetClass : TSubclassOf<UMHWeaponMenuSlotWidget>`

## Visual Rules
### Selected
- call `UMHItemSlotWidget::SetSelected(true)`
- set weapon name text to highlight color

### Unselected
- call `UMHItemSlotWidget::SetSelected(false)`
- set weapon name text to default color

## Implementation Order
1. Add this planning doc and the UML doc.
2. Add `UMHWeaponMenuSlotWidget`.
3. Refactor `UMHMainMenuWidget` to generate only combined slot widgets.
4. Remove `WeaponNameSlotContainer` usage from code.
5. Keep `SelectedWeaponText` refresh path intact.
6. Update menu blueprints to use the new entry widget.

## Summary
The refactor moves the weapon menu from a two-container composition into a single-entry composition.
That removes the overlap problem at the layout source and keeps selection visuals in one place.
