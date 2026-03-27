# 아이템 계층구조 및 외부 참조/적용 기획서

## 1. 문서 목적

이 문서는 현재 프로젝트에서 아이템 데이터가 어떤 계층으로 나뉘어 있고, 외부 시스템이 그 데이터를 어떻게 참조해서 실제 게임 상태에 반영하는지 정리한다.

분석 범위는 아래 네 가지다.

- 아이템 데이터 계층 구조
- 런타임 아이템 인스턴스 구조
- 스탯과 어트리뷰트 적용 구조
- 프론트/인게임 UI 참조 구조

## 2. 핵심 결론

- 현재 주력 구조는 `DataTable Row` 기반이 아니라 `UItemDataRegistry + Instanced UObject ItemData` 기반이다.
- 아이템 데이터는 `무엇을 표현할지`를 들고 있고, 실제 장비 적용은 `AMHPlayerCharacter`가 수행한다.
- 프론트 미리보기는 라이브 인스턴스를 만들지 않고, `무기 클래스 CDO -> ItemRegistry -> ItemData` 경로로 데이터를 읽는다.
- 전투 수치는 `FMHAttackStats`를 직접 쓰지 않고, 장비 시점에 `GameplayEffect`와 `AttributeSet`으로 한 번 변환된다.
- 인게임 HUD는 `AttributeSet`을 직접 조회하지 않고, `AMHPlayerCharacter`가 브로드캐스트하는 델리게이트를 구독한다.

## 3. 현재 아이템 계층 구조

## 3.1 정적 데이터 계층

현재 정적 데이터의 실제 사용 루트는 아래와 같다.

| 계층 | 주요 타입 | 역할 |
|---|---|---|
| 공통 베이스 | `UMHItemDataBase` | 이름, 아이콘, 설명, 가격, `ItemTag` 보유 |
| 일반 아이템 | `UMHCommonItemData` | 일반 아이템 계층의 중간 베이스 |
| 일반 파생 | `UMHConsumableItemData`, `UMHMaterialItemData` | 소비/재료 분기 |
| 장비 베이스 | `UMHEquipItemData` | 장비 계층의 중간 베이스 |
| 장비 파생 | `UMHWeaponItemData`, `UMHArmorItemData` | 무기/방어구 분기 |
| 무기 세부 파생 | `UMHMeleeWeaponItemData`, `UMHLongSwordItemData`, `UMHGreatSwordItemData` | 무기별 추가 데이터 보유 |

### 관찰 포인트

- `UMHItemDataBase`는 `DefaultToInstanced`, `EditInlineNew`라서 독립 `PrimaryDataAsset`보다 `Registry 안에 박힌 인스턴스 객체`에 가깝다.
- `UMHWeaponItemData`는 공격 스탯을 `FMHAttackStats`로 보유하고, 시각 리소스는 `WeaponMeshData`를 통해 소프트 레퍼런스로 들고 있다.
- `UMHLongSwordItemData`만 사야(`SayaMeshData`)를 별도 보유한다.
- `UMHGreatSwordItemData`는 현재 별도 필드를 추가하지 않고 타입 분기 지점만 제공한다.
- `AMHChargeBladeInstance`는 존재하지만 대응하는 전용 `ItemData` 파생 클래스는 아직 없다.

## 3.2 병행 존재하는 레거시 스키마

`Source/ProjectMHW/Public/Type/MHItemStructType.h`에는 아래 구조가 남아 있다.

- `FItemBaseData`
- `FWeaponItemData`
- `FArmorItemData`
- `FMHItemDataRow`

이 스키마는 `DataTable` 스타일의 행 중심 설계지만, 현재 실제 참조 경로에서는 쓰이지 않는다. 현재 런타임이 사용하는 경로는 `UMHItemDataBase` 계열과 `UItemDataRegistry`다.

즉, 프로젝트에는 `구조체 기반 설계 흔적`과 `실제 사용 중인 UObject 기반 설계`가 함께 존재한다.

## 3.3 레지스트리 계층

아이템 데이터의 실제 진입점은 `UItemDataRegistry`다.

- `ItemDataMap : TMap<FName, UMHItemDataBase*>`
- `GetItemData(FName)` 제공

### 구조적 의미

- 모든 아이템 데이터는 `FName Key`로 조회된다.
- 아이템 인스턴스는 데이터를 직접 소유하지 않고, `Registry + Key`를 통해 역참조한다.
- 디자이너 입장에서는 `개별 아이템 애셋`보다 `레지스트리 내부 엔트리`를 편집하는 모델에 가깝다.

## 3.4 런타임 인스턴스 계층

| 계층 | 주요 타입 | 역할 |
|---|---|---|
| 인스턴스 베이스 | `AMHItemInstanceBase` | `ItemRegistry`, `ItemDataKey`, `CachedItemData` 보유 |
| 장비 인스턴스 | `AMHEquipItemInstance` | 장비용 공통 베이스, Ability Handle 저장용 배열 보유 |
| 무기 인스턴스 | `AMHWeaponInstance` | 무기 메시 적용, 공격 능력 부여, `FMHAttackStats` 제공 |
| 근접 무기 | `AMHMeleeWeaponInstance` | 히트박스, 현재 공격 `DamageSpec`, `AttackTag` 관리 |
| 무기별 파생 | `AMHLongSwordInstance`, `AMHGreatSwordInstance`, `AMHChargeBladeInstance` | 무기별 메시/컴포넌트/콤보 그래프 연결 |

### 인스턴스 계층의 핵심 동작

- `AMHItemInstanceBase::BeginPlay()`가 `ApplyItemData()`를 호출한다.
- `ApplyItemData()`는 `ItemRegistry + ItemDataKey`로 데이터를 찾아 `CachedItemData`에 캐시한다.
- `AMHWeaponInstance::ApplyItemData()`는 무기 메시를 읽어 `WeaponMesh`에 적용한다.
- `AMHLongSwordInstance::ApplyItemData()`는 `SayaMesh`를 추가 적용한다.
- `AMHGreatSwordInstance::BeginPlay()`는 액션/차지 컴포넌트와 콤보 그래프를 연결한다.

### 주의점

- `AMHItemInstanceBase`는 `UItemDataRegistry::GetItemData()`를 쓰지 않고 `ItemDataMap.Find()`를 직접 호출한다.
- `ApplyItemData()`가 적용하는 범위는 대부분 시각 리소스와 인스턴스 내부 캐시다.
- 실제 장비 효과, ASC 수치 반영, 무기 부착 상태는 인스턴스가 아니라 플레이어가 담당한다.

## 4. 외부 참조 및 적용 구조

## 4.1 프론트 미리보기 참조 구조

프론트 메뉴는 무기 액터를 스폰하지 않는다. 대신 무기 클래스의 CDO를 통해 `ItemRegistry`와 `ItemDataKey`를 읽는다.

### 참조 흐름

1. `UMHMainMenuWidget`가 `WeaponSlotClasses`를 순회한다.
2. 각 `TSubclassOf<AMHWeaponInstance>`의 `GetDefaultObject<AMHWeaponInstance>()`를 호출한다.
3. CDO를 `AMHItemInstanceBase`로 캐스팅해 `GetItemRegistry()`, `GetItemDataKey()`를 읽는다.
4. `UItemDataRegistry::GetItemData()`로 `UMHItemDataBase`를 가져온다.
5. 아이콘, 이름, 무기 스탯을 UI에 반영한다.

### 의미

- 프론트는 런타임 인스턴스를 만들지 않는다.
- 프론트는 `클래스 기본값`을 통해 아이템 데이터를 역참조한다.
- 미리보기 경로와 실제 장비 경로가 분리되어 있다.

## 4.2 실제 장비 적용 구조

실제 게임 시작 시 장비는 `AMHPlayerCharacter`가 생성하고 적용한다.

### 적용 흐름

- `BeginPlay()`에서 `SpawnAndEquipDefaultWeapon()` 호출
- `UMHFrontendGameInstance`에 선택 무기가 있으면 그 클래스를 우선 사용
- `SpawnActor<AMHWeaponInstance>()`로 무기 액터 생성
- 무기 액터 `BeginPlay()`에서 `ApplyItemData()` 수행
- `AMHPlayerCharacter::EquipWeaponInstance()` 호출
- 무기 소유자 지정, 무기 Ability 부여, 무기 부착 및 애니메이션 레이어 갱신
- `RefreshEquippedWeaponStatEffect()`로 스탯 적용

### 적용 책임

| 적용 항목 | 실제 적용 주체 | 데이터 원천 |
|---|---|---|
| 무기 메시 | `AMHWeaponInstance` | `UMHWeaponItemData::WeaponMeshData` |
| 롱소드 사야 | `AMHLongSwordInstance` | `UMHLongSwordItemData::SayaMeshData` |
| 공격 Ability | `AMHWeaponInstance::PrimaryAttackAbilityClass` | 무기 액터/블루프린트 기본값 |
| 공격력/회심률 | `AMHPlayerCharacter + UMHGameplayEffect_WeaponStat` | `FMHAttackStats` |
| 예리도 현재값/최대값 | `AMHPlayerCharacter` | `FMHAttackStats::SharpnessLength`, `MaxSharpnessColor` |

## 4.3 스탯과 어트리뷰트 적용 구조

무기 데이터가 곧바로 데미지 계산에 들어가는 구조는 아니다. 먼저 `GameplayEffect`와 `AttributeSet`으로 변환된다.

### 장비 시점 변환

- `AMHPlayerCharacter::ApplyEquippedWeaponStatEffect()`는 `EquippedWeapon->GetAttackStats()`를 읽는다.
- `UMHGameplayEffect_WeaponStat`는 아래 두 값을 `SetByCaller`로 받는다.
  - `Data.Weapon.AttackPower`
  - `Data.Weapon.Affinity`
- 이 GE는 `UMHCombatAttributeSet`의 아래 값에 Additive로 누적된다.
  - `AttackPower`
  - `CriticalRate`

### 예리도의 이중 표현

예리도는 정적 데이터와 런타임 값이 분리되어 있다.

| 구분 | 저장 위치 | 의미 |
|---|---|---|
| 정적 분포 | `FMHAttackStats::SharpnessLength` | 색상별 최대 길이 |
| 정적 최대 등급 | `FMHAttackStats::MaxSharpnessColor` | 무기가 가질 수 있는 최고 색 |
| 런타임 현재값 | `UMHPlayerAttributeSet::Sharpness` | 현재 남은 예리도 총량 |
| 런타임 최대값 | `UMHPlayerAttributeSet::MaxSharpness` | 현재 무기의 총 예리도 |
| 런타임 캐시 | `AMHPlayerCharacter::CurrentSharpnessColor`, `CurrentSharpnessValue` | HUD/전투 계산용 캐시 |
| 전투 보정 | `UMHCombatAttributeSet::SharpnessModifier` | 실제 데미지 계산에 쓰이는 승수 |

### 의미

- 무기 데이터는 예리도 분포를 정의한다.
- 플레이어는 이를 현재값/최대값 Attribute로 재구성한다.
- 실제 데미지 계산은 색상에 따라 `SharpnessModifier`를 다시 계산해서 사용한다.

## 4.4 공격 DamageSpec 적용 구조

무기 스탯이 적용된 뒤의 실제 공격은 `GameplayAbility -> DamageSpec -> 히트박스 -> ExecutionCalculation` 흐름으로 이어진다.

### 롱소드/대검 공통 흐름

1. 무기 공격 Ability가 `Source ASC`에서 `AttackPower`를 읽는다.
2. `MakeOutgoingSpec(DamageEffectClass)`로 공격용 GE Spec 생성
3. 콤보 배율 또는 차지 배율을 곱한다.
4. `SetByCallerMagnitude(Data.Damage.*)`로 Spec에 값 기록
5. 무기 인스턴스에 `SetCurrentDamageSpec()`, `SetCurrentAttackTag()` 전달
6. `AMHMeleeWeaponInstance` 히트박스가 대상과 겹치면 인터페이스로 Spec 전달
7. `UMHDamageExecutionCalculation`이 Source/Target Attribute를 캡처해 최종 데미지 계산
8. 결과가 `UMHHealthAttributeSet::IncomingDamage`로 누적된다.

여기서 Ability가 공격 시작 시점의 `Source ASC` 값을 읽어 Spec을 만드는 이유는,
그 시점에 걸려 있던 버프/장비 효과 등을 반영한 공격 값을 피격 시점까지 유지하기 위해서다.

즉, 지금은 근접 히트박스 기반이지만, 공격 생성 시점과 실제 명중 시점이 분리되는
투사체 무기나 지연 타격 구조에도 같은 DamageSpec 전달 모델을 그대로 확장할 수 있게 설계된 셈이다.

### 캡처되는 주요 어트리뷰트

| 출처 | 어트리뷰트 |
|---|---|
| Source | `AttackPower`, `CriticalRate`, `SharpnessModifier` |
| Target | `Defense`, `FireResist`, `WaterResist`, `ThunderResist`, `IceResist`, `DragonResist` |

### 구조적 의미

- 정적 아이템 데이터는 직접 데미지를 계산하지 않는다.
- 장비 데이터는 먼저 ASC 스탯으로 들어간다.
- 공격 Ability는 ASC의 현재 상태를 읽어 공격 Spec을 만든다.
- 결국 아이템 데이터는 `초기값`, 실제 전투는 `Attribute 기반 현재값`으로 계산된다.

## 4.5 UI 참조 구조

UI는 프론트와 인게임이 서로 다른 경로를 사용한다.

### 프론트 UI

| UI | 참조 방식 | 설명 |
|---|---|---|
| `UMHMainMenuWidget` | 무기 클래스 CDO 역참조 | 슬롯 이름/아이콘 구성 |
| `UMHWeaponStatPanelWidget` | 무기 클래스 CDO 역참조 | 공격력, 예리도, 회심률, 속성 표시 |
| `UMHWeaponMenuSlotWidget` | 상위 위젯에서 전달받은 데이터 사용 | 슬롯 텍스트/선택 강조 |

### 인게임 UI

| UI | 참조 방식 | 설명 |
|---|---|---|
| `UMHPlayerStatusWidget` | `AMHPlayerCharacter` 델리게이트 구독 | 체력, 회복 가능 체력, 스태미나, 기인 게이지, 예리도 |
| `UMHItemSelectionWidget` | `AMHPlayerCharacter` 델리게이트 구독 기대 | 숫자 대신 선택 강조 상태 표시 |
| `UMHItemSlotWidget` | 상위 위젯이 전달한 브러시/선택 상태 사용 | 공용 아이템 슬롯 외형 |

### 공통 UI 베이스

`UMHUserWidgetBase`는 아래 역할을 수행한다.

- Owning Player, Pawn, PlayerState 캐시
- ASC 조회
- 위젯 초기화 시점에 플레이어 문맥 재해석

즉, 인게임 UI는 직접 월드 탐색하기보다 공통 베이스에서 플레이어 문맥을 얻는다.

## 5. 책임 분리 요약

| 영역 | 책임 |
|---|---|
| `UMHItemDataBase` 계열 | 콘텐츠 정의 |
| `UItemDataRegistry` | 데이터 인덱싱 |
| `AMHItemInstanceBase` 계열 | 데이터 해석 및 인스턴스 내부 적용 |
| `AMHPlayerCharacter` | 장비 장착, ASC 반영, 런타임 상태 동기화 |
| `GameplayEffect / AttributeSet / Execution` | 전투 수치 시스템 |
| 프론트 UI | CDO 기반 정적 프리뷰 |
| 인게임 UI | 플레이어 런타임 상태 소비 |

## 6. 확인된 구조적 특징과 리스크

## 6.1 장점

- 무기 데이터와 전투 계산이 완전히 한 클래스에 묶이지 않아 확장 여지가 있다.
- 프론트에서 라이브 액터를 만들지 않고 CDO만으로 프리뷰를 만들 수 있어 메뉴 비용이 낮다.
- 데미지 계산은 `SetByCaller + ExecutionCalculation` 구조라서 밸런싱 포인트가 비교적 명확하다.

## 6.2 리스크

- `MHItemStructType.h`의 구조체 기반 모델과 실제 사용 중인 UObject 기반 모델이 병행 존재해 설계 기준이 이중화되어 있다.
- `AMHItemInstanceBase::ApplyItemData()`와 UI 프리뷰는 둘 다 같은 데이터를 읽지만 경로가 다르다.
  - 인스턴스는 `ItemDataMap.Find()` 직접 호출
  - UI는 `GetItemData()` 호출
- `ApplyItemData()`는 주로 비주얼 적용까지만 담당하고, 장비 효과/Attribute 반영은 플레이어가 담당해서 책임이 분산되어 있다.
- 무기 Ability는 ItemData가 아니라 무기 액터 기본값에 들어 있어, 순수 데이터 기반 장비 정의로 보기 어렵다.
- `AMHEquipItemInstance`의 `GrantedAbilitySpecHandles`는 현재 장비/해제 흐름에 실사용되지 않는다.
- 소비 아이템 선택 UI는 설계 의도와 현재 구현이 어긋나 있다.
  - `UMHItemSelectionWidget`은 `OnConsumableSelectionChanged`를 구독한다.
  - 하지만 입력 처리 쪽은 `SelectedConsumable`만 직접 바꾸고 브로드캐스트를 하지 않는다.
  - 결과적으로 위젯은 초기 동기화 이후 자동 갱신이 끊긴 상태다.
- 방어구/재료/소비 아이템 데이터 클래스는 존재하지만, 무기만큼 강한 런타임 인스턴스 계층과 적용 루프는 아직 없다.

## 7. 기획 관점 해석

현재 구조는 완전한 데이터 드리븐 장비 시스템이라기보다 아래 방식에 가깝다.

- 아이템 데이터는 콘텐츠의 원본 정의를 가진다.
- 아이템 인스턴스는 그 데이터를 자기 자신에게 읽어 적용한다.
- 플레이어는 장비 상태를 ASC와 애니메이션 시스템에 연결한다.
- UI는 프론트에서는 정적 데이터, 인게임에서는 플레이어 런타임 상태를 소비한다.

즉, `데이터 정의`, `장비 적용`, `전투 계산`, `UI 표시`가 느슨하게 연결된 다단계 구조다.

## 8. 확장 제안

### 제안 1. 참조 경로 단일화

- 인스턴스와 UI가 모두 `UItemDataRegistry::GetItemData()`를 사용하도록 맞추는 것이 좋다.
- 조회 정책이 하나로 통일되면 디버깅 포인트가 줄어든다.

### 제안 2. 장비 적용 책임 분리

- `AMHPlayerCharacter`가 맡는 장비 적용 로직을 `EquipmentApplier` 성격의 컴포넌트나 서비스로 분리하면 유지보수가 쉬워진다.

### 제안 3. 아이템 선택 UI 이벤트 정리

- `SetSelectedConsumable()`을 실제 구현하고 내부에서 브로드캐스트하도록 통일하는 편이 맞다.
- 입력 함수가 직접 값을 만지지 않고 공용 세터를 통과하면 UI 누락을 방지할 수 있다.

### 제안 4. 아이템 데이터의 역할 명확화

- 무기 Ability, 장비 시 적용할 GE, UI 프리뷰용 추가 메타데이터를 어디까지 ItemData가 소유할지 기준을 정해야 한다.
- 현재처럼 일부는 ItemData, 일부는 Weapon Actor 기본값에 섞여 있으면 확장 시 규칙이 흔들릴 수 있다.

## 9. 최종 정리

현재 프로젝트의 아이템 구조는 아래 문장으로 정리할 수 있다.

`ItemData가 콘텐츠를 정의하고, ItemInstance가 데이터를 해석하며, PlayerCharacter가 장비 상태를 전투 시스템과 UI 시스템으로 연결한다.`

이 구조는 무기 중심 전투 루프에는 이미 잘 연결되어 있지만, 소비 아이템/방어구까지 일반화하려면 참조 경로와 적용 책임을 조금 더 표준화할 필요가 있다.
