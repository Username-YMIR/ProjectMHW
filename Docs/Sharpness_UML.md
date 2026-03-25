# Sharpness UML

## Class View
```mermaid
classDiagram
    class AMHPlayerCharacter {
        +CurrentSharpnessColor
        +GetCurrentSharpnessValue()
        +GetMaxSharpnessValue()
        +GetCurrentSharpnessColor()
        +RefreshSharpnessState()
        +HandleSharpnessBounce()
        +Input_ItemUseStarted()
        +Input_ItemUseCompleted()
        +Input_AttackPrimary()
        +Input_AttackSecondary()
        +Input_Dodge()
        +ReceiveDamageSpec_Implementation()
    }

    class UMHPlayerAttributeSet {
        +Sharpness
        +MaxSharpness
    }

    class UMHCombatAttributeSet {
        +SharpnessModifier
    }

    class UMHPlayerStatusWidget {
        +BindToPlayerCharacter()
        +SyncInitialValues()
        +HandleSharpnessChanged()
    }

    class UMHProgressBarWidget {
        +SetValues()
        +SetSharpnessColor()
        +SetDefaultFillColor()
    }

    class AMHMeleeWeaponInstance {
        +BeginAttackWindow()
        +EndAttackWindow()
        +OnWeaponBeginOverlap()
    }

    class UGA_MHSharpen {
        +ActivateAbility()
        +OnSharpenStart()
        +TickSharpen()
        +HandleMontageCompleted()
        +HandleMontageInterrupted()
    }

    class UMHGA_Potion {
        +ActivateAbility()
        +OnDrink()
        +TickHeal()
    }

    class AMHMonsterCharacterBase {
        +ReceiveDamageSpec_Implementation()
        +HandleDamageAccepted()
    }

    class UMHDamageExecutionCalculation {
        +Execute_Implementation()
    }

    AMHPlayerCharacter --> UMHPlayerAttributeSet
    AMHPlayerCharacter --> UMHCombatAttributeSet
    AMHPlayerCharacter --> UMHPlayerStatusWidget
    AMHPlayerCharacter --> AMHMeleeWeaponInstance
    AMHPlayerCharacter --> UGA_MHSharpen
    AMHPlayerCharacter --> UMHGA_Potion
    UMHPlayerStatusWidget --> UMHProgressBarWidget
    AMHMeleeWeaponInstance --> AMHMonsterCharacterBase
    AMHMeleeWeaponInstance --> AMHPlayerCharacter
    AMHMonsterCharacterBase --> UMHDamageExecutionCalculation
```

## Bounce Sequence
```mermaid
sequenceDiagram
    participant AN as AnimNotifyState_MeleeAttackHitWindow
    participant W as AMHMeleeWeaponInstance
    participant P as AMHPlayerCharacter
    participant M as AMHMonsterCharacterBase

    AN->>W: BeginAttackWindow()
    W->>P: HandleWeaponAttackHit()
    alt Sharpness <= 0
        P-->>W: EMHHitResultType::Bounced
        W->>W: EndAttackWindow()
        W->>P: HandleSharpnessBounce()
        P->>P: Stop current weapon attack ability
        P->>P: Stop current montage
        P->>P: Reset combo/action state
        P->>P: Invalidate buffered input
        P->>P: Play shared bounce montage
    else Normal hit
        W->>M: ReceiveDamageSpec()
        M->>M: Apply damage
        M->>P: Return accepted hit
    end
```

## Sharpen Sequence
```mermaid
sequenceDiagram
    participant I as Input
    participant P as AMHPlayerCharacter
    participant G as UGA_MHSharpen
    participant A as UMHPlayerAttributeSet
    participant U as UMHPlayerStatusWidget

    I->>P: Select whetstone / use item
    P->>G: TryActivateAbilityByClass()
    G->>G: ActivateAbility()
    G->>G: Wait for Event.Item.SharpenStart
    G->>P: SharpenStart event
    G->>A: Set Sharpness = MaxSharpness
    G->>P: RefreshSharpnessState()
    P->>U: OnSharpnessChanged(Current, Max)
    U->>U: Update bar percent and tint
    alt move/attack/dodge input or damage
        P->>G: Cancel sharpen flow
        G->>G: EndAbility()
    end
```

## UI Flow
```mermaid
flowchart TD
    A["AMHPlayerCharacter"] --> B["OnSharpnessChanged"]
    B --> C["UMHPlayerStatusWidget"]
    C --> D["UMHProgressBarWidget"]
    D --> E["SetPercent(Current / Max)"]
    D --> F["SetSharpnessColor(EMHSharpnessColor)"]
    D --> G["DefaultFillColor when no weapon or MaxSharpness == 0"]
```

## Notes
- `CurrentSharpnessLength` is intentionally excluded from the active model.
- `GetSharpnessMultiplier()` in `MHCombatStatStructType.h` is the single physical sharpness multiplier source.
- Sharpness color lookup is done on the player, then pushed into the UI.
- `UMHProgressBarWidget` is reused for stamina and sharpness, so the default tint must remain available for non-sharpness bars.
