# UI Viewer Role UML

## 1. 클래스 구조

```mermaid
classDiagram
    class AMHPlayerCharacter {
        +OnHealthChanged
        +OnStaminaChanged
        +OnSharpnessChanged
        +BindAttributeDelegates()
        +GetCurrentSharpnessColor()
    }

    class UAbilitySystemComponent {
        +GetGameplayAttributeValueChangeDelegate()
    }

    class UMHUserWidgetBase {
        +CacheOwningPlayerContext()
        +GetAbilitySystemComponent()
    }

    class UMHPlayerStatusWidget {
        +BindToPlayerCharacter()
        +HandleHealthChanged()
        +HandleStaminaChanged()
        +HandleSharpnessChanged()
    }

    class UMHProgressBarWidget {
        +SetValues(Max, Current)
        +SetSharpnessColor()
    }

    class UMHMainMenuWidget {
        +BuildWeaponSlots()
        +RefreshWeaponStatPanel()
    }

    class UMHWeaponStatPanelWidget {
        +ApplyWeaponClass()
        +RefreshStatRows()
    }

    class AMHWeaponInstance {
        +GetItemRegistry()
        +GetItemDataKey()
    }

    class UItemDataRegistry {
        +GetItemData(FName)
    }

    class UMHItemDataBase {
        +Name
        +Icon
    }

    class UMHWeaponItemData {
        +AttackStats
    }

    AMHPlayerCharacter --> UAbilitySystemComponent : bind attribute change
    UMHUserWidgetBase <|-- UMHPlayerStatusWidget
    UMHPlayerStatusWidget --> AMHPlayerCharacter : subscribe delegates
    UMHPlayerStatusWidget --> UMHProgressBarWidget : update visuals
    UMHMainMenuWidget --> AMHWeaponInstance : class CDO lookup
    UMHMainMenuWidget --> UMHWeaponStatPanelWidget : pass selected class
    AMHWeaponInstance --> UItemDataRegistry : key lookup
    UItemDataRegistry --> UMHItemDataBase : base item data
    UItemDataRegistry --> UMHWeaponItemData : weapon stat data
    UMHWeaponStatPanelWidget --> UItemDataRegistry : resolve preview data
```

- 인게임 UI는 `PlayerCharacter`가 브로드캐스트한 상태를 소비한다.
- 프론트 UI는 무기 클래스 CDO와 `ItemDataRegistry`를 통해 정적 데이터를 조회한다.
- 두 UI 모두 상태 계산의 주체가 아니라, 외부 상태를 표시하는 소비자다.

## 2. 인게임 HUD 갱신 시퀀스

```mermaid
sequenceDiagram
    participant ASC as AbilitySystemComponent
    participant Player as PlayerCharacter
    participant Status as PlayerStatusWidget
    participant Bar as ProgressBarWidget

    Status->>Player: BindToPlayerCharacter()
    Player->>ASC: Bind GetGameplayAttributeValueChangeDelegate()
    ASC-->>Player: Health / Stamina / Sharpness changed
    Player-->>Status: Broadcast UI delegates
    Status->>Bar: SetValues(Max, Current)
    Status->>Bar: SetSharpnessColor()
```

- UI는 Attribute를 직접 계산하지 않고, 플레이어가 정리한 값을 받는다.
- `UMHProgressBarWidget`은 퍼센트와 색상만 갱신한다.

## 3. 메인 메뉴 프리뷰 시퀀스

```mermaid
sequenceDiagram
    participant Menu as MainMenuWidget
    participant WeaponClass as WeaponClass
    participant CDO as Weapon CDO
    participant Registry as ItemDataRegistry
    participant Data as WeaponItemData
    participant Panel as WeaponStatPanelWidget

    Menu->>WeaponClass: GetDefaultObject()
    WeaponClass-->>CDO: Class CDO
    Menu->>CDO: GetItemRegistry()
    Menu->>CDO: GetItemDataKey()
    CDO->>Registry: GetItemData(Key)
    Registry-->>Data: Weapon preview data
    Menu->>Panel: ApplyWeaponClass(WeaponClass)
    Panel->>Registry: Resolve weapon stats
    Panel->>Panel: Refresh text rows only
```

- 프론트 메뉴는 런타임 무기 액터를 만들지 않는다.
- UI는 레지스트리에서 얻은 결과를 텍스트와 이미지로 표시만 한다.

## 4. UI 뷰어 원칙 플로우

```mermaid
flowchart TD
    A["Gameplay State / Item Data"] --> B["Player / Controller / Registry"]
    B --> C["Widget Update Function"]
    C --> D["Text / Progress / Highlight Refresh"]
    D --> E["UI is Viewer Only"]
```

- UI는 `상태 저장소`가 아니라 `최종 표시 레이어`라는 점을 강조하기 위한 발표용 도식이다.
