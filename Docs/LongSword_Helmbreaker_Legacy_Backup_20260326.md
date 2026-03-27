# 태도 투구깨기 기존 구조 백업

## 목적

- 투구깨기 리팩토링 전 기존 흐름을 백업한다.
- 이후 공중 페이즈 기반 구조로 바꾼 뒤 비교 기준으로 사용한다.

## 기존 진입 구조

- `Move_LS_SpiritThrust`에서 `InputPattern_LS_Helmbreaker` 입력 시 `Move_LS_SpiritHelmbreaker`로 파생된다.
- 관련 위치: `Source/ProjectMHW/Private/Weapons/LongSword/MHLongSwordComboGraph.cpp`

## 기존 시작 조건

- `Move_LS_SpiritHelmbreaker`는 `bLongSwordSpiritThrustHelmbreakerReady == true` 이고 `CurrentSpiritLevel >= 1`일 때만 시작된다.
- 관련 위치: `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`

## 기존 자원 처리

- `Move_LS_SpiritThrust` 히트 보상 처리 시 `bLongSwordSpiritThrustHelmbreakerReady = true`가 된다.
- `Move_LS_SpiritHelmbreaker` 시작 시 `bLongSwordSpiritThrustHelmbreakerReady = false`로 되돌리고 기인 레벨을 1 감소시킨다.
- 관련 위치: `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`

## 기존 어빌리티 실행 구조

- 투구깨기는 다른 태도 콤보와 동일하게 `UMHGA_LongSwordCombo` 안에서 단일 콤보 노드 몽타주로 처리된다.
- `PlayResolvedNode()`가 `Move_LS_SpiritHelmbreaker` 몽타주를 그대로 재생한다.
- 몽타주 완료 시 `OnMontageCompleted()`가 즉시 아래 순서로 후처리한다.
- `HandleComboMontageStateTransition(false)`
- `ResetMeleeWeaponAttack()`
- `CachedComboComponent->ResetCombo()`
- `CachedPlayer->ClearAllLongSwordCounterSuccessFlags()`
- `EndAbility(...)`
- 관련 위치: `Source/ProjectMHW/Private/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.cpp`

## 기존 구조의 한계

- 투구깨기 전용 공중 상태가 없다.
- 상승 / 정점 전 공중 제어 / 낙하 / 착지 마감이 모두 하나의 몽타주 길이에 묶인다.
- 정점 판정을 실제 `Velocity.Z`로 보지 않는다.
- 착지 판정을 `Landed()`나 지면 접촉 이벤트로 처리하지 않는다.
- 플레이어 캐릭터에는 투구깨기 전용 `Landed()` 처리와 공중 페이즈 상태가 없다.
- `CombatState_Helmbreaker` 태그는 선언만 되어 있고 플레이어 현재 전투 상태 반환에는 아직 사용되지 않는다.

## 리팩토링 방향 메모

- 1단계는 기존 투구깨기 진입 몽타주를 상승 히트 구간으로 축소해서 사용한다.
- 2단계 이후는 실제 공중 상태 기준으로 제어한다.
- 2단계 종료는 `Velocity.Z <= 0` 시점 기준으로 처리한다.
- 3단계 종료는 `Landed()` 또는 지면 접촉으로 처리한다.
- 착지 모션은 마지막 마감 단계로 분리한다.
