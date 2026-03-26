# Frontend Menu and Loading UML

## Class View
```mermaid
classDiagram
    class AFrontendPlayerController {
        +BeginPlay()
        +ShowMainMenu()
        +ShowLoadingScreen()
        +HandleStartRequested()
    }

    class UMHMainMenuWidget {
        +WeaponSlotClasses
        +SelectedSlotIndex
        +BuildWeaponSlots()
        +HandleSlotClicked()
        +GetSelectedWeaponClass()
        +RequestStartBattle()
    }

    class UMHItemSlotWidget {
        +SetSelected(bool)
        +SetItemIconBrush(FSlateBrush)
    }

    class UMHLoadingWidget {
        +SetProgress(float)
        +SetStatusText(FText)
    }

    class UMHFrontendGameInstance {
        +PendingWeaponClass
        +StartBattleTransition()
        +BeginPreload()
        +GetLoadingProgress()
        +OpenBattleLevel()
    }

    class AMHItemInstanceBase {
        #ItemDataKey
        #ItemRegistry
        +GetItemData()
        +GetItemDataKey()
        +GetItemRegistry()
    }

    class UMHItemDataBase {
        +Name
        +Icon
        +Description
    }

    class UItemDataRegistry {
        +ItemDataMap
        +GetItemData(FName)
    }

    class AMHWeaponInstance {
        +ApplyItemData()
        +GetWeaponType()
    }

    class AMHPlayerCharacter {
        +BeginPlay()
        +SpawnAndEquipDefaultWeapon()
        +EquipWeaponInstance()
    }

    AFrontendPlayerController --> UMHMainMenuWidget
    AFrontendPlayerController --> UMHLoadingWidget
    AFrontendPlayerController --> UMHFrontendGameInstance
    UMHMainMenuWidget --> UMHItemSlotWidget
    UMHMainMenuWidget --> AMHWeaponInstance
    UMHMainMenuWidget --> AMHItemInstanceBase
    AMHItemInstanceBase --> UItemDataRegistry
    UItemDataRegistry --> UMHItemDataBase
    UMHFrontendGameInstance --> AMHWeaponInstance
    AMHPlayerCharacter --> AMHWeaponInstance
    AMHPlayerCharacter --> UMHFrontendGameInstance
```

## Menu Build Sequence
```mermaid
sequenceDiagram
    participant PC as AFrontendPlayerController
    participant Menu as UMHMainMenuWidget
    participant WeaponCDO as AMHWeaponInstance CDO
    participant Registry as UItemDataRegistry
    participant Data as UMHItemDataBase
    participant Slot as UMHItemSlotWidget

    PC->>Menu: ShowMainMenu()
    Menu->>Menu: BuildWeaponSlots()
    loop for each WeaponSlotClasses entry
        Menu->>WeaponCDO: GetDefaultObject()
        Menu->>WeaponCDO: GetItemRegistry() / GetItemDataKey()
        WeaponCDO->>Registry: Resolve registry
        Registry->>Data: Find item data by key
        Data-->>Menu: Icon
        Menu->>Slot: Create slot widget
        Menu->>Slot: SetItemIconBrush()
    end
    Menu->>Menu: Select first valid slot
```

## Start Transition Sequence
```mermaid
sequenceDiagram
    participant User
    participant Menu as UMHMainMenuWidget
    participant PC as AFrontendPlayerController
    participant GI as UMHFrontendGameInstance
    participant Loading as UMHLoadingWidget

    User->>Menu: Click Start
    Menu->>Menu: Resolve selected weapon class
    Menu->>PC: Request start battle
    PC->>GI: Set PendingWeaponClass
    PC->>Loading: Show loading widget
    PC->>GI: StartBattleTransition()
    GI->>GI: Begin preload
    loop until preload complete
        GI-->>Loading: Update progress
    end
    GI->>GI: OpenBattleLevel()
```

## Battle Entry Sequence
```mermaid
sequenceDiagram
    participant GI as UMHFrontendGameInstance
    participant World as Battle Level
    participant Player as AMHPlayerCharacter
    participant Weapon as AMHWeaponInstance

    GI->>World: OpenBattleLevel()
    World->>Player: Spawn player
    Player->>Player: BeginPlay()
    Player->>Player: SpawnAndEquipDefaultWeapon()
    alt PendingWeaponClass is valid
        Player->>Weapon: Spawn pending weapon class
    else Fallback
        Player->>Weapon: Spawn DefaultWeaponClass
    end
    Player->>Player: EquipWeaponInstance(Weapon)
```

## Loading Progress State
```mermaid
flowchart TD
    A["Start button accepted"] --> B["Show loading widget"]
    B --> C["Validate selected weapon"]
    C --> D["Preload required soft assets"]
    D --> E["Progress reaches 1.0"]
    E --> F["Open battle level"]
    F --> G["Player BeginPlay"]
    G --> H["SpawnAndEquipDefaultWeapon"]
```

## Notes
- The main menu reuses `UMHItemSlotWidget` instead of inventing a separate slot control.
- Icon resolution depends on adding safe preview accessors to `AMHItemInstanceBase`.
- The loading bar should represent staged preload progress, not the opaque internal map load percent.
- `AMHPlayerCharacter` should keep the existing equip pipeline and only change the class resolution priority.
