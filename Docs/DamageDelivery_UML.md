# Damage Delivery System UML

## 1. 클래스 구조

```mermaid
classDiagram
    class AMHCharacterBase {
        +ReceiveDamageSpec()
        +ValidateDamageSpec()
        +CanReceiveDamage()
        +ApplyIncomingDamageSpec()
        +HandleDamageAccepted()
    }

    class AMHPlayerCharacter {
        +ApplyIncomingPlayerDamageSpec()
        +HandleWeaponAttackHit()
        +RefreshEquippedWeaponStatEffect()
    }

    class AMHMonsterCharacterBase {
        +ConsumeMonsterAttackHitOnce()
        +BuildMonsterDamageSpec()
        +HandleDamageAccepted()
    }

    class AMHWeaponInstance {
        +GrantWeaponAbilities()
        +ClearWeaponAbilities()
        +GetAttackStats()
    }

    class AMHMeleeWeaponInstance {
        +BeginAttackWindow()
        +EndAttackWindow()
        +SetCurrentDamageSpec()
        +SetCurrentAttackTag()
        +OnWeaponBeginOverlap()
        +TryDeliverDamageSpecToTarget()
    }

    class UMHGA_LongSwordCombo {
        +BuildDamageSpecForNode()
        +PushDamageSpecToWeapon()
    }

    class UMHGA_GreatSwordAttack {
        +BuildDamageSpecForMove()
        +PushCurrentAttackDataToWeapon()
    }

    class UGA_MHMonsterAttackBasic {
        +ActivateAbility()
        +EndAbility()
    }

    class UAbilitySystemComponent {
        +GiveAbility()
        +MakeOutgoingSpec()
        +ApplyGameplayEffectSpecToSelf()
        +RemoveActiveGameplayEffect()
        +SetNumericAttributeBase()
    }

    class UMHGameplayEffect_Damage {
        +Instant GE
    }

    class UMHGameplayEffect_PlayerDamage {
        +Player 전용 Damage GE 래퍼
    }

    class UMHGameplayEffect_WeaponStat {
        +Infinite GE
        +AttackPower += SetByCaller
        +CriticalRate += SetByCaller
    }

    class UMHDamageExecutionCalculation {
        +Execute()
        +CalculatePhysicalDamage()
        +CalculateElementDamage()
        +ApplyCritical()
    }

    class UMHHealthAttributeSet {
        +Health
        +MaxHealth
        +IncomingDamage
        +PostGameplayEffectExecute()
    }

    class UMHCombatAttributeSet {
        +AttackPower
        +Defense
        +CriticalRate
        +SharpnessModifier
    }

    class UMHResistanceAttributeSet {
        +FireResist
        +WaterResist
        +ThunderResist
        +IceResist
        +DragonResist
    }

    class IMHDamageSpecReceiverInterface {
        +ReceiveDamageSpec()
    }

    AMHPlayerCharacter --|> AMHCharacterBase
    AMHMonsterCharacterBase --|> AMHCharacterBase
    AMHCharacterBase ..|> IMHDamageSpecReceiverInterface

    AMHPlayerCharacter --> UAbilitySystemComponent : owns
    AMHMonsterCharacterBase --> UAbilitySystemComponent : owns
    AMHPlayerCharacter --> UMHHealthAttributeSet : owns
    AMHPlayerCharacter --> UMHCombatAttributeSet : owns
    AMHPlayerCharacter --> UMHResistanceAttributeSet : owns
    AMHMonsterCharacterBase --> UMHHealthAttributeSet : owns
    AMHMonsterCharacterBase --> UMHCombatAttributeSet : owns

    AMHMeleeWeaponInstance --|> AMHWeaponInstance
    AMHPlayerCharacter --> AMHWeaponInstance : equips
    AMHMeleeWeaponInstance --> IMHDamageSpecReceiverInterface : deliver spec

    AMHWeaponInstance --> UAbilitySystemComponent : GiveAbility/ClearAbility
    AMHPlayerCharacter --> UMHGameplayEffect_WeaponStat : create/apply
    UMHGameplayEffect_WeaponStat --> UMHCombatAttributeSet : modify

    UMHGA_LongSwordCombo --> UAbilitySystemComponent : read attrs / make spec
    UMHGA_GreatSwordAttack --> UAbilitySystemComponent : read attrs / make spec
    UGA_MHMonsterAttackBasic --> AMHMonsterCharacterBase : open attack window

    UMHGA_LongSwordCombo --> UMHGameplayEffect_Damage : create spec
    UMHGA_GreatSwordAttack --> UMHGameplayEffect_Damage : create spec
    AMHMonsterCharacterBase --> UMHGameplayEffect_Damage : create spec
    AMHPlayerCharacter --> UMHGameplayEffect_PlayerDamage : rewrap incoming spec

    UMHGameplayEffect_PlayerDamage --|> UMHGameplayEffect_Damage
    UMHGameplayEffect_Damage --> UMHDamageExecutionCalculation : execute
    UMHDamageExecutionCalculation --> UMHCombatAttributeSet : capture
    UMHDamageExecutionCalculation --> UMHResistanceAttributeSet : capture
    UMHDamageExecutionCalculation --> UMHHealthAttributeSet : output IncomingDamage
```

## 2. 플레이어가 몬스터를 타격하는 시퀀스

```mermaid
sequenceDiagram
    participant Input as Player Input
    participant GA as Weapon GameplayAbility
    participant ASC as Source ASC
    participant W as AMHMeleeWeaponInstance
    participant Notify as AnimNotifyState HitWindow
    participant T as AMHMonsterCharacterBase
    participant TASC as Target ASC
    participant GE as UMHGameplayEffect_Damage
    participant Exec as UMHDamageExecutionCalculation
    participant Health as UMHHealthAttributeSet

    Input->>GA: 공격 입력으로 Ability 활성화
    GA->>ASC: AttackPower 등 현재 Attribute 조회
    GA->>GA: 무브 배율/차지 배율/기인 배율 계산
    GA->>ASC: MakeOutgoingSpec(DamageGE)
    GA->>GA: SetByCaller(Data.Damage.*) 주입
    GA->>W: SetCurrentDamageSpec()
    GA->>W: SetCurrentAttackTag()

    Notify->>W: BeginAttackWindow()
    W->>T: ReceiveDamageSpec(SourceActor, Weapon, AttackTag, Spec)
    T->>T: ValidateDamageSpec / CanReceiveDamage
    T->>TASC: ApplyGameplayEffectSpecToSelf(Spec)
    TASC->>GE: Execute
    GE->>Exec: 최종 대미지 계산 요청
    Exec->>Health: IncomingDamage += FinalDamage
    Health->>Health: Health -= IncomingDamage
    T->>T: HandleDamageAccepted()
    T-->>W: FMHHitAcknowledge 반환
    W->>W: 히트 소비 / 카메라 셰이크 / 후속 처리
```

## 3. 몬스터가 플레이어를 타격하는 시퀀스

```mermaid
sequenceDiagram
    participant AI as Monster AI
    participant MGA as UGA_MHMonsterAttackBasic
    participant M as AMHMonsterCharacterBase
    participant MASC as Monster ASC
    participant P as AMHPlayerCharacter
    participant PASC as Player ASC
    participant PGE as UMHGameplayEffect_PlayerDamage
    participant Exec as UMHDamageExecutionCalculation
    participant Health as UMHHealthAttributeSet

    AI->>MGA: 공격 태그 기반 Ability 활성화
    MGA->>M: BeginMonsterAttackWindow()
    MGA->>M: ConsumeMonsterAttackHitOnce(AttackTag)
    M->>MASC: MakeOutgoingSpec(DamageGE)
    M->>M: SetByCaller(Data.Damage.Physical)
    M->>P: ReceiveDamageSpec(SourceActor, SourceWeapon, AttackTag, Spec)

    P->>P: 카운터/무적/히트리액트 상태 확인
    alt 카운터 또는 무적
        P-->>M: Invulnerable HitAcknowledge
    else 일반 피격
        P->>PASC: MakeOutgoingSpec(PlayerIncomingDamageGE)
        P->>PGE: 기존 Spec의 Data.Damage.* 재주입
        P->>PASC: ApplyGameplayEffectSpecToSelf(PlayerSpec)
        PGE->>Exec: 최종 대미지 계산
        Exec->>Health: IncomingDamage += FinalDamage
        Health->>Health: Health -= IncomingDamage
        P->>P: HandleDamageAccepted()
        P-->>M: Accepted HitAcknowledge
    end
```

## 4. 장착 스탯이 ASC로 반영되는 시퀀스

```mermaid
sequenceDiagram
    participant Player as AMHPlayerCharacter
    participant Weapon as AMHWeaponInstance
    participant ASC as UAbilitySystemComponent
    participant WGE as UMHGameplayEffect_WeaponStat
    participant Combat as UMHCombatAttributeSet

    Player->>Weapon: EquipWeaponInstance()
    Player->>Weapon: GrantWeaponAbilities(ASC)
    Weapon->>ASC: GiveAbility(PrimaryAttackAbilityClass)

    Player->>ASC: MakeOutgoingSpec(WeaponStatGE)
    Player->>Player: 무기 ItemData에서 AttackStats 읽기
    Player->>WGE: SetByCaller(Data.Weapon.AttackPower)
    Player->>WGE: SetByCaller(Data.Weapon.Affinity)
    Player->>ASC: ApplyGameplayEffectSpecToSelf()
    WGE->>Combat: AttackPower += AP
    WGE->>Combat: CriticalRate += Affinity

    Note over Player,Combat: 장착 지속 스탯은 Infinite GE로 유지되고,\n공격 1회 대미지는 별도 DamageSpec으로 생성된다.
```

## 5. UML 해석 포인트

- 공격 Ability와 실제 피격 적용은 같은 객체가 아니다.
- `GameplayEffectSpec`은 공격자에서 만들어지지만, 실제 적용 주체는 타깃 ASC다.
- 플레이어는 incoming damage를 한 번 더 재포장하는 별도 레이어를 갖는다.
- 무기 장착 스탯 GE와 공격용 Damage GE는 목적이 다르므로 분리되어 있다.
