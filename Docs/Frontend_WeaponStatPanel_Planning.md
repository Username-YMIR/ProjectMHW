# Frontend Weapon Stat Panel Planning

## Scope
This document records the planned weapon stat panel for the frontend main menu.

The goal is to replace the previous selected-weapon summary text with a dedicated stat panel widget that shows the currently selected weapon details.

## Confirmed Rules
- The selected weapon summary should be displayed through a separate stat widget instead of `SelectedWeaponText`.
- The stat widget is built from rows that use a `property name || value` layout.
- The stat rows are pre-placed in the panel and bound statically.
- The panel updates text only and does not create rows dynamically at runtime.
- The panel shows:
  - weapon name
  - attack power
  - sharpness length per color
  - affinity
  - weapon element
- The sharpness total row should display total sharpness length.
- All sharpness color rows should always exist in the layout.
- Colors after the configured max sharpness color should display `-`.
- Colors up to the configured max sharpness color should display the actual value, including `0`.
- If no element exists, show `없음`.

## Proposed Widget Model
### `UMHWeaponStatRowWidget`
- Owns one horizontal row.
- Displays a property label and a property value.
- Should be reused for every stat line.

### `UMHWeaponStatPanelWidget`
- Owns the stat panel.
- Owns fixed bound row widgets.
- Refreshes row texts whenever the selected weapon changes.
- Resolves weapon item data from the selected weapon class.
- Applies sharpness values to pre-existing sharpness rows.

## Main Menu Integration
- `UMHMainMenuWidget` keeps selection ownership.
- On selection change, it calls the stat panel widget with the currently selected weapon class.
- `SelectedWeaponText` can remain in code as optional, but the stat panel becomes the primary UI for selected-weapon information.

## Data Sources
### Base Item Data
- `UMHItemDataBase::Name`

### Weapon Item Data
- `UMHWeaponItemData::AttackStats`

### Attack Stats Fields
- `AttackPower`
- `SharpnessLength`
- `MaxSharpnessColor`
- `Affinity`
- `AttackElementTag`

## Display Plan
### Base Rows
- `이름`
- `공격력`
- `회심율`
- `무기 속성`

### Sharpness Section
- Add a total row: `예리도 | {합산 예리도}`
- Always keep all color rows in the layout
- Use Korean color labels:
  - `빨강`
  - `주황`
  - `노랑`
  - `초록`
  - `파랑`
  - `하양`

Recommended output example:

```text
이름       용의 태도
공격력     100
예리도     180
빨강       50
주황       40
노랑       60
초록       30
파랑       -
하양       -
회심율     15%
무기 속성  불
```

## Error and Empty Handling
- If no selected weapon exists, show a single fallback row like `선택 무기  없음`.
- If selected item data is not weapon data, show safe fallback rows.
- If sharpness data is empty, still show `예리도 | 0` and apply `0 / -` rules to each color row.

## Implementation Order
1. Add planning and UML docs.
2. Add `UMHWeaponStatRowWidget`.
3. Add `UMHWeaponStatPanelWidget`.
4. Bind the fixed row widgets into `UMHWeaponStatPanelWidget`.
5. Bind the stat panel widget into `UMHMainMenuWidget`.
6. Refresh the stat panel on initial build and selection change.

## Summary
The stat panel should become the authoritative selected-weapon summary UI.
It uses a reusable row widget and a panel widget with fixed bound rows that updates text from `UMHWeaponItemData`.
