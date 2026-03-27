# Damage Delivery System 분석 기획서

## 1. 문서 목적

이 문서는 `ProjectMHW`의 현행 대미지 전달 시스템을 코드 기준으로 분석한 결과를 정리한 문서다.

- 플레이어와 몬스터가 서로에게 대미지를 전달하는 전체 흐름을 설명한다.
- GAS가 이 프로젝트에서 어떤 역할로 사용되고 있는지 의도를 해석한다.
- 현재 구조의 장점, 설계 의도, 한계와 개선 포인트를 함께 정리한다.

## 2. 핵심 결론

현 구조의 핵심은 대미지를 한 번에 처리하지 않고, 아래 5단계로 분리했다는 점이다.

1. 장착 단계에서 무기 스탯과 공격 Ability를 ASC에 등록한다.
2. 공격 Ability가 현재 무브의 의도에 맞는 `GameplayEffectSpec`를 런타임에 생성한다.
3. 무기 액터가 충돌 시점에 그 Spec을 타깃에게 전달한다.
4. 타깃이 피격 가능 여부를 판단하고 자기 ASC 기준으로 Spec을 적용한다.
5. ExecutionCalculation과 AttributeSet이 최종 수치 계산과 체력 차감을 수행한다.

즉, 이 프로젝트는 GAS를 "공격 판정 전체를 자동으로 처리하는 프레임워크"로 쓰기보다, 아래처럼 역할을 잘라서 쓰고 있다.

- `GameplayAbility`: 공격 연출과 런타임 Spec 생성
- `GameplayEffectSpec + SetByCaller`: 공격 페이로드 전달
- `ExecutionCalculation`: 방어력/내성/크리티컬 같은 계산 집중화
- `AttributeSet`: 실제 체력 변화 반영
- `ASC`: 능력, 스탯, 이펙트의 실행 주체

## 3. 분석 범위

이번 분석은 아래 코드 경로를 기준으로 했다.

- `Source/ProjectMHW/Private/Character/MHCharacterBase.cpp`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Private/Character/Monster/MHMonsterCharacterBase.cpp`
- `Source/ProjectMHW/Private/Items/Instance/MHWeaponInstance.cpp`
- `Source/ProjectMHW/Private/Items/Instance/MHMeleeWeaponInstance.cpp`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.cpp`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Weapon/GreatSword/MHGA_GreatSwordAttack.cpp`
- `Source/ProjectMHW/Private/Character/Monster/Ability/GA_MHMonsterAttackBasic.cpp`
- `Source/ProjectMHW/Private/Combat/Execution/MHDamageExecutionCalculation.cpp`
- `Source/ProjectMHW/Private/Combat/Attributes/MHHealthAttributeSet.cpp`
- `Source/ProjectMHW/Private/Combat/Attributes/MHCombatAttributeSet.cpp`
- `Source/ProjectMHW/Private/Combat/Effects/MHGameplayEffect_Damage.cpp`
- `Source/ProjectMHW/Private/Combat/Effects/MHGameplayEffect_PlayerDamage.cpp`
- `Source/ProjectMHW/Private/Items/Effects/MHGameplayEffect_WeaponStat.cpp`

## 4. 시스템 구조 요약

### 4.1 장착 시점

플레이어가 무기를 장착하면 두 가지가 동시에 들어간다.

1. 무기 액터가 ASC에 공격 Ability를 부여한다.
2. 플레이어가 무기 스탯 GE를 만들어 자기 ASC에 적용한다.

여기서 무기 스탯은 `UMHGameplayEffect_WeaponStat`가 담당한다.

- Duration은 `Infinite`
- `Data.Weapon.AttackPower`
- `Data.Weapon.Affinity`

를 `SetByCaller`로 받아서 `UMHCombatAttributeSet`의

- `AttackPower`
- `CriticalRate`

에 더한다.

즉, 무기의 "장착 중 지속되는 능력치"는 ASC 위의 지속 GE로 유지된다.

### 4.2 공격 시점

공격은 무기 Ability가 시작점이다.

- 롱소드: `UMHGA_LongSwordCombo`
- 대검: `UMHGA_GreatSwordAttack`
- 몬스터: `UGA_MHMonsterAttackBasic`

플레이어 무기 Ability는 현재 무브 태그, 차지 배율, 콤보 배율, 기인 배율 등을 반영해 `UMHGameplayEffect_Damage` 기반 Spec을 런타임 생성한다.

이때 실제 공격 페이로드는 `SetByCaller`로 채운다.

- `Data.Damage.Physical`
- `Data.Damage.Fire`
- `Data.Damage.Water`
- `Data.Damage.Thunder`
- `Data.Damage.Ice`
- `Data.Damage.Dragon`

생성한 Spec은 바로 타깃에 적용하지 않고, 현재 공격 중인 무기 인스턴스에 저장한다.

- `CurrentDamageSpecHandle`
- `CurrentAttackTag`

즉, Ability는 "이 공격이 어떤 대미지를 가져야 하는가"만 결정하고, "언제 누구에게 적용되는가"는 무기 액터가 맡는다.

### 4.3 충돌 전달 시점

`AMHMeleeWeaponInstance`는 애니메이션의 공격 유효 구간 동안만 히트박스를 켠다.

- `BeginAttackWindow()`
- `EndAttackWindow()`

히트박스가 타깃과 겹치면 `IMHDamageSpecReceiverInterface::ReceiveDamageSpec()`를 호출해 Spec을 전달한다.

여기서 중요한 점은 공격자가 타깃 구체 타입을 몰라도 된다는 점이다.

- 무기는 "DamageSpec을 받을 수 있는 대상"이라는 인터페이스만 본다.
- 피격자는 자신의 규칙에 따라 수락, 무적, 카운터, 바운스 결과를 결정한다.

또한 반환값인 `FMHHitAcknowledge`로 공격자에게 결과를 다시 돌려준다.

- `bAcceptedHit`
- `bConsumeHitOnce`
- `bShouldStopAttackWindow`
- `ResultType`

이 구조 때문에 무기 쪽은 결과에 따라

- 1회 히트 소비
- 공격 윈도우 종료
- 카메라 셰이크
- 롱소드 히트 보상
- 예리도 바운스

를 처리할 수 있다.

### 4.4 피격 수신 시점

공통 수신 템플릿은 `AMHCharacterBase`에 있다.

1. `ValidateDamageSpec()`
2. `CanReceiveDamage()`
3. `PrepareDamageTextContext()`
4. `ApplyIncomingDamageSpec()`
5. `FinalizeDamageTextContext()`
6. `HandleDamageAccepted()`
7. 필요 시 `HandleDeath()`

이 베이스 템플릿의 의미는, "공격 Spec 수신"과 "피격 후처리"를 분리하는 것이다.

## 5. 실제 대미지 전달 흐름

### 5.1 플레이어 -> 몬스터

현 코드 기준 흐름은 아래와 같다.

1. 플레이어 입력이 무기 Ability를 활성화한다.
2. 롱소드/대검 Ability가 현재 무브 기준 `DamageSpec`을 만든다.
3. Ability가 무기 인스턴스에 `CurrentDamageSpecHandle`, `CurrentAttackTag`를 저장한다.
4. 애니메이션 Notify가 공격 윈도우를 연다.
5. 무기 히트박스가 몬스터와 겹치면 `ReceiveDamageSpec()` 호출
6. 몬스터는 자기 ASC에 전달받은 Spec을 그대로 적용한다.
7. `UMHDamageExecutionCalculation`이 최종 피해를 계산한다.
8. `UMHHealthAttributeSet::PostGameplayEffectExecute()`가 `IncomingDamage`를 실제 `Health` 감소로 변환한다.
9. 몬스터는 피격 VFX/SFX/DamageText를 후처리로 실행한다.
10. 무기는 `FMHHitAcknowledge`를 받아 히트 소비 여부와 후속 처리를 결정한다.

### 5.2 몬스터 -> 플레이어

몬스터는 플레이어보다 단순한 공격 생성 방식을 쓴다.

1. 몬스터 Ability가 공격 태그를 정하고 공격 윈도우를 연다.
2. `ConsumeMonsterAttackHitOnce()`가 실제 히트 판정과 사거리 체크를 수행한다.
3. `BuildMonsterDamageSpec()`가 몬스터 전용 DamageSpec을 생성한다.
4. 플레이어의 `ReceiveDamageSpec()`를 호출한다.
5. 플레이어는 카운터, 무적, 피격 반응 여부를 먼저 판단한다.
6. 일반 피격이면 `PlayerIncomingDamageEffectClass` 기반 Spec으로 재포장한다.
7. 재포장된 Spec이 플레이어 ASC에 적용된다.
8. 동일한 `UMHDamageExecutionCalculation`과 `UMHHealthAttributeSet`이 최종 체력 감소를 처리한다.
9. 플레이어는 피격 몽타주, 포션 취소, 화상 점화 같은 후처리를 실행한다.

### 5.3 플레이어 수신이 별도 재포장을 하는 이유

플레이어는 `ApplyIncomingDamageSpec()`를 override 해서, 들어온 Spec을 자기 전용 GE로 다시 만든다.

직접 확인되는 사실은 아래와 같다.

- 현재 `UMHGameplayEffect_PlayerDamage`는 `UMHGameplayEffect_Damage`를 상속하고 추가 로직이 없다.
- 하지만 플레이어는 굳이 한 번 더 `MakeOutgoingSpec()`를 한다.

이 구조의 의도는 다음처럼 해석할 수 있다.

- 플레이어 피격 규칙을 몬스터와 분리할 확장 지점을 미리 만들어 둔 것
- 추후 가드, 슈퍼아머, 전용 Execution, 플레이어 전용 GameplayCue를 붙이기 쉬운 구조
- "들어온 공격"과 "플레이어가 실제로 적용하는 피격 효과"를 분리하려는 의도

즉, 현재는 동작이 거의 같아 보여도, 구조상으로는 플레이어 피격 경로를 별도 정책 레이어로 떼어 놓은 셈이다.

## 6. GAS를 어떻게 사용했는가

## 6.1 ASC는 전투 상태의 실행 컨테이너

이 프로젝트에서 ASC는 단순 Ability 보관소가 아니라 전투 상태의 기준 저장소다.

- 부여된 공격 Ability를 관리한다.
- 무기 장착 GE를 유지한다.
- Health, Defense, AttackPower, CriticalRate, SharpnessModifier 같은 전투 값을 보관한다.
- DamageSpec 적용 시 Execution과 AttributeSet 처리를 실행한다.

즉, "전투 수치의 권위"를 ASC 쪽으로 몰아둔 구조다.

## 6.2 GameplayAbility는 공격 연출과 런타임 조합 담당

플레이어 공격 Ability의 책임은 아래에 가깝다.

- 어떤 무브가 실행되는지 결정
- 어떤 몽타주를 재생할지 결정
- 현재 공격력이 얼마인지 ASC에서 조회
- 현재 무브 배율과 특수 배율을 합산
- 이번 공격용 `GameplayEffectSpec`을 생성
- 그 Spec을 무기에 전달

즉, Ability는 "행동의 컨트롤러"이고, 실제 체력 감소는 Ability 안에서 직접 하지 않는다.

이 방식의 장점은 공격 모션과 공격 수치가 동기화되기 쉽다는 점이다.

- 콤보 노드별 배율
- 차지 단계별 배율
- 기인 레벨 배율
- 무브별 히트 보상

같은 런타임 정보를 Ability에서 묶어서 다루기 좋다.

## 6.3 SetByCaller는 공격 페이로드 봉투 역할

이 프로젝트는 `SetByCaller`를 매우 의도적으로 쓰고 있다.

- 무기 장착 시: `Data.Weapon.*`
- 공격 시: `Data.Damage.*`

선택 이유는 명확하다.

- 공격 수치가 정적 데이터 하나로 끝나지 않는다.
- 무브 태그, 콤보 배율, 차지 배율, 기인 배율처럼 런타임 계산이 필요하다.
- 따라서 "공격 시점에 계산한 값"을 Spec에 실어서 보내야 한다.

즉, `GameplayEffect` 클래스 자체는 거의 고정 틀이고, 실제 값은 매 공격마다 `SetByCaller`로 주입하는 구조다.

이 설계는 무브 수가 많고 배율 조합이 잦은 액션 게임에 잘 맞는다.

## 6.4 ExecutionCalculation은 피격 계산의 중심

`UMHDamageExecutionCalculation`은 소스와 타깃 정보를 모두 캡처하지만, 역할을 보면 "최종 피격 계산기"에 가깝다.

직접 확인되는 계산은 아래와 같다.

- 물리: `BasePhysicalDamage * SharpnessModifier * DefenseMultiplier`
- 속성: `BaseElementDamage * ResistMultiplier`
- 최종합: 물리 + 속성 합산 후 크리티컬 적용
- 출력: `IncomingDamage`에 Additive

이 구조의 의도는 다음과 같다.

- 공격 Ability는 공격자 관점의 "의도된 대미지"를 만든다.
- Execution은 타깃 관점의 "실제 적용 대미지"를 만든다.

즉, 공격자와 타깃의 책임이 분리된다.

## 6.5 AttributeSet은 최종 반영 계층

`UMHHealthAttributeSet`은 `IncomingDamage`를 바로 HP로 쓰지 않고 한 번 버퍼처럼 받는다.

1. Execution이 `IncomingDamage += FinalDamage`
2. AttributeSet이 `IncomingDamage`를 읽는다.
3. `IncomingDamage`를 0으로 되돌린다.
4. `Health -= Damage`

이 방식의 의도는 아래와 같다.

- 대미지 계산 로직과 HP 반영 로직을 분리
- 여러 종류의 GE가 동일한 체력 처리 루틴을 재사용 가능
- Damage Text처럼 적용 전후 체력 차이를 잡기 쉬움

즉, `IncomingDamage`는 "최종 대미지 전달 버퍼" 역할이다.

## 6.6 현재 구조는 완전한 GAS 전환이 아니라 하이브리드 구조

이 프로젝트는 GAS를 적극 사용하지만, 모든 전투 로직을 GAS로 밀어 넣지는 않았다.

GAS 쪽에 둔 것:

- 공격 Ability 부여
- 무기 스탯 유지
- 대미지/회복 계산
- Health, Defense, Resistance, AttackPower 같은 전투 값

캐릭터/무기 코드에 둔 것:

- 콤보 상태 머신
- 카운터 판정
- 히트 리액션 재생
- 예리도 색상과 세그먼트 계산
- 기인 게이지/레벨
- 공격 윈도우 충돌 처리

이건 코드상 직접 확인되는 사실이고, 그 의도는 다음처럼 해석된다.

- 애니메이션 타이밍과 입력 분기가 많은 액션 로직은 캐릭터/무기 코드에 두고
- 수치 계산과 지속 상태만 GAS로 옮겨 복잡도를 분산하려는 선택

즉, "액션 제어는 수동 코드, 수치 실행은 GAS"라는 하이브리드 철학이다.

## 7. 설계 의도 해석

## 7.1 공격자와 피격자 결합도를 낮추려는 의도

`IMHDamageSpecReceiverInterface`와 `FMHHitAcknowledge` 조합은 매우 의도적이다.

- 무기는 타깃 클래스 상세 구현을 모른다.
- 타깃은 공격이 들어왔을 때 자체 규칙으로 수락/거절/무적/바운스를 결정한다.
- 공격자는 결과만 받아서 후속 연출을 정리한다.

이건 전형적인 "전달과 판정의 분리" 구조다.

## 7.2 공격 수치는 공격 Ability에서, 방어 수치는 Execution에서 처리하려는 의도

코드상 플레이어 공격 Ability는 이미 `AttackPower * MoveMultiplier`를 계산해 `BasePhysicalDamage`를 만든다.
반면 Execution은 방어력, 내성, 예리도, 크리티컬을 적용한다.

따라서 의도는 아래처럼 읽힌다.

- 공격자 로직: 이번 공격의 기본 대미지는 얼마인가
- 피격자 로직: 그 기본 대미지가 타깃에게 실제 얼마가 되는가

이는 PvE 액션 게임에서 자주 쓰는 분리 방식이다.

여기서 공격 Ability가 소스 어트리뷰트 값을 읽고 DamageSpec을 미리 만드는 이유는,
공격 시작 시점에 버프, 장착 효과, 일시적 강화 등이 반영된 값을 피격 시점에도 그대로 적용하기 위해서다.

즉, 현재는 근접 무기 히트박스에 먼저 쓰이고 있지만, 추후 투사체 무기처럼
공격 생성과 실제 피격 시점이 분리되는 구조에도 같은 방식으로 확장 적용하려는 설계로 볼 수 있다.

## 7.3 플레이어 피격만 별도 파이프라인으로 분리하려는 의도

플레이어는 몬스터와 달리

- 카운터
- 무적 판정
- 포션 취소
- 히트 리액션
- 상태 이상 시작

같은 특별 규칙이 많다.

그래서 플레이어만 `ReceiveDamageSpec()`에서 먼저 규칙을 소화하고, 그 뒤 자기 전용 GE로 재포장하는 구조를 갖는다.

즉, 플레이어는 "피격 자체가 하나의 시스템"으로 분리될 가능성을 열어 둔 구조다.

## 7.4 무기 장착 효과를 데이터 기반으로 흘리려는 의도

무기 스탯을 `AttackStats`에서 읽고, 이를 `WeaponStat GE`의 `SetByCaller`로 넘겨 ASC에 적용하는 방식은 아래 의도를 가진다.

- 아이템 데이터와 실제 전투 수치 반영을 분리
- 장착/해제 시 효과를 Active GE 핸들로 관리
- 장비 변경 시 수치 재적용을 단순화

즉, 장착 중 유지되는 전투 수치는 GAS 쪽이 더 잘 관리한다고 본 설계다.

## 8. 장점

### 8.1 대미지 파이프라인이 계층별로 명확하다

- Ability: 공격 의도 생성
- Weapon: 충돌 전달
- Receiver: 피격 수락 판단
- Execution: 최종 수치 계산
- AttributeSet: 실제 체력 반영

역할이 명확해서 디버깅 포인트도 비교적 선명하다.

### 8.2 런타임 무브 배율 대응이 좋다

`SetByCaller` 기반이라 무브 태그, 차지, 콤보, 특수 상태를 쉽게 반영할 수 있다.

### 8.3 피격 후처리를 시스템적으로 분리했다

실제 체력 감소와 피격 연출이 분리되어 있어,

- Damage Text
- 피격 VFX/SFX
- 카메라 셰이크
- 상태 이상

같은 기능을 추가하기 쉽다.

### 8.4 플레이어와 몬스터를 같은 계산기 위에 올려두었다

둘 다 `UMHDamageExecutionCalculation`과 `UMHHealthAttributeSet`을 공유하므로, 기본 대미지 규칙을 한 곳에서 통제할 수 있다.

## 9. 현재 구조의 한계와 주의점

### 9.1 `AttackPower` 캡처가 현재 계산식에는 직접 쓰이지 않는다

`UMHDamageExecutionCalculation`은 `AttackPower`를 캡처하지만, 실제 최종 계산은 `BasePhysicalDamage`를 기준으로 한다.

이 말은 현재 구조에서 `AttackPower`의 실질 사용처가 공격 Ability의 Spec 생성 단계라는 뜻이다.

이 선택은 공격 시작 시점의 버프 등이 반영된 값을 Spec에 담아 두고,
실제 피격 시점에는 그 값을 전달/적용하려는 의도와 맞닿아 있다.

즉, 구조적으로는 합리적일 수 있지만, 코드를 읽는 사람 입장에서는

- 공격력은 Ability에서 이미 굽는 값인지
- Execution에서 다시 쓰는 값인지

경계가 다소 헷갈릴 수 있다.

### 9.2 몬스터의 속성 대미지 필드는 아직 전달 경로에 연결되지 않았다

`FMonsterAbilityEntry`에는 `FireDamage`가 있지만, 현재 `BuildMonsterDamageSpec()`는 물리값만 입력받아 속성값을 모두 0으로 넣는다.

즉, 데이터는 준비되어 있지만 실제 전달 파이프라인은 아직 물리 대미지 중심이다.

### 9.3 몬스터 수신 경로가 베이스 템플릿과 일부 중복된다

`AMHMonsterCharacterBase::ReceiveDamageSpec_Implementation()`는 베이스와 유사한 흐름을 다시 구현하고 있다.

이 구조는 당장은 괜찮지만, 이후 공통 로직이 늘어나면 플레이어/몬스터 간 드리프트가 생길 수 있다.

### 9.4 예리도는 완전히 GE 기반이 아니라 수동 동기화가 필요하다

예리도 수치와 색상은 캐릭터 코드가 관리하고, `SharpnessModifier`만 ASC에 밀어 넣는다.

즉, 예리도는 GAS 위에 얹힌 하이브리드 상태다.

- 장착 해제 시 수동 리셋
- 색상 변화 시 수동 modifier 갱신

이 필요하다.

### 9.5 플레이어와 몬스터의 초기화 패턴이 다르다

- 플레이어: `SetNumericAttributeBase()`로 기본 체력/방어력 세팅
- 몬스터: `GASAsset` 기반 Startup Tag/Ability/Effect 적용

이 차이는 의도적으로 볼 수도 있지만, 프로젝트가 커질수록 데이터 주도 초기화 기준이 갈릴 수 있다.

### 9.6 치명타 결과가 UI 페이로드까지는 전달되지 않는다

`FMHDamageTextPayload`에는 `bCritical`이 있지만, 현재 `FinalizeDamageTextContext()`에서는 항상 `false`로 채운다.

즉, 계산은 치명타를 적용하지만, 결과 메타데이터는 아직 UI까지 이어지지 않는다.

## 10. 유지보수 관점의 권장 해석

현 구조는 버릴 설계가 아니라, 아래 방향으로 정리하면 더 강해지는 설계다.

1. "Ability는 BaseDamage 생성, Execution은 FinalDamage 계산"이라는 규칙을 문서화한다.
2. 플레이어 전용 `PlayerIncomingDamageEffectClass`는 유지한다.
3. 몬스터 수신 로직은 가능한 범위에서 베이스 템플릿에 더 맞춘다.
4. 몬스터 속성 대미지 데이터는 실제 `SetByCaller` 경로로 연결한다.
5. 치명타 여부 같은 결과 메타데이터는 `EffectContext` 또는 별도 payload로 확장한다.

## 11. 최종 해석

이 프로젝트의 대미지 전달 시스템은 GAS를 "액션 전투의 수치 실행 백엔드"로 사용하고 있다.

- 공격 결정과 애니메이션 타이밍은 캐릭터/무기/Ability 코드가 담당한다.
- 대미지 수치의 전달과 최종 적용은 GAS가 담당한다.

즉, 설계 의도는 완전한 GAS 게임플레이 전환이 아니라,

"실시간 액션 제어는 수동 코드로 유지하고, 계산과 상태 반영만 GAS에 위임한다"

로 정리할 수 있다.
