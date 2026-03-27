# Portfolio_PPT_5min_Detailed_Plan

## 문서 목적
- `Portfolio_PPT_5min_Summary.md`를 실제 PPT 제작 단계로 옮기기 위한 세부 기획 문서다.
- 5분 발표 기준으로 페이지를 정확히 나누고, 각 페이지에 들어갈 메시지와 참고 자료를 정리한다.
- 발표 자료 제작 시 바로 사용할 수 있도록 `UML`, `코드`, `이미지 예시 문구`를 함께 적는다.
- 메인컬러는 몬스터 헌터 월드의 메뉴 컬러팔레트를 사용한다
- 배경 이미지 파일을 사용해 배경에 적용시킨다

## 발표 운영 기준
- 총 발표 시간: `5분`
- 총 슬라이드 수: `7페이지`
- 구성 원칙:
  - 한 페이지에 한 메시지만 전달
  - 기능 설명보다 설계 의도와 책임 분리를 강조
  - UML은 발표용으로 필요한 부분만 잘라 사용
  - 코드 캡처는 함수 전체보다 핵심 부분만 사용

## 전체 페이지 구성표

| Page | 시간 | 주제 |
|---|---:|---|
| 1 | 25초 | 타이틀 / 프로젝트 한 줄 소개 |
| 2 | 25초 | 전체 설계 방향 |
| 3 | 70초 | 아이템 데이터 레지스트리 |
| 4 | 80초 | 대미지 시스템 |
| 5 | 50초 | UI Viewer 역할 |
| 6 | 30초 | 한계와 개선안 |
| 7 | 20초 | 마무리 |

---

## Page 1

### 슬라이드 제목
`ProjectMHW 전투 시스템 설계`

### 발표 목표
- 이 프로젝트가 단순 기능 구현이 아니라, 데이터와 전투와 UI를 분리해 설계한 사례라는 점을 먼저 인식시킨다.

### 핵심 카피
`아이템 데이터, 전투 수치, UI를 분리된 레이어로 설계해 확장성과 유지보수성을 확보한 액션 전투 시스템`

### 본문 구성
- 프로젝트 키워드
  - `Unreal Engine`
  - `C++`
  - `GAS`
  - `UMG`
- 오늘 설명할 3가지
  - 아이템 데이터 레지스트리
  - 대미지 시스템
  - UI Viewer 설계

### 레이아웃 가이드
- 좌측: 프로젝트 대표 전투 이미지
- 우측: 제목 + 한 줄 설명 + 기술 스택

### 참고 UML
- 없음

### 참고 코드
- 없음

### 이미지 예시 문구
- `예시 이미지: 플레이어가 몬스터를 공격하는 대표 전투 장면. 무기, 히트 이펙트, HUD가 함께 보이도록 캡처`

---

## Page 2

### 슬라이드 제목
`왜 이 구조로 설계했는가`

### 발표 목표
- 이후 슬라이드의 공통 기준이 되는 설계 방향을 먼저 설명한다.

### 핵심 카피
`콘텐츠 정의, 런타임 적용, 전투 계산, UI 표시를 한곳에 몰지 않고 레이어별로 분리했다`

### 본문 구성
- 문제 정의
  - 통합 데이터 구조는 아이템 종류별 예외가 많아진다.
  - 액션 전투는 연출 타이밍과 수치 계산이 서로 다르다.
  - UI는 표시만 해야 유지보수가 쉽다.
- 설계 방향
  - `ItemData`: 콘텐츠 원본 정의
  - `ItemInstance`: 런타임 해석 및 적용
  - `Player + GAS`: 장비/전투 수치 처리
  - `UI`: 표시 전용

### 레이아웃 가이드
- 상단: 한 줄 설계 문장
- 하단: 4단 레이어 다이어그램

### 참고 UML
- `Docs/ItemHierarchy_Reference_UML.md`
  - 1번 레이어 구조도 사용

### 참고 코드
- `Source/ProjectMHW/Public/Items/Data/ItemDataBase.h`
- `Source/ProjectMHW/Public/Items/Instance/MHItemInstanceBase.h`
- `Source/ProjectMHW/Public/Character/Player/MHPlayerCharacter.h`
- `Source/ProjectMHW/Public/Widgets/MHUserWidgetBase.h`

### 이미지 예시 문구
- `이미지 대신 UML 사용 권장`

---

## Page 3

### 슬라이드 제목
`아이템 데이터 레지스트리`

### 발표 목표
- DataTable 대신 `Registry + KeyName + 계층형 ItemData`를 선택한 이유를 설명한다.

### 핵심 카피
`서로 다른 아이템 데이터를 하나의 통합 스키마로 묶지 않고, 계층형 데이터와 Key 기반 레지스트리로 관리했다`

### 본문 구성
- 설계 이유
  - 무기, 방어구, 소비 아이템은 필요한 프로퍼티가 다르다.
  - 통합 DataTable은 불필요한 데이터와 예외 처리가 많아진다.
- 해결 방식
  - `UItemDataRegistry`
  - `TMap<FName, UMHItemDataBase*>`
  - 각 ItemData는 계층 구조로 필요한 데이터만 보유
- 사용 방식
  - 장착 시: `KeyName`으로 스탯 조회 후 GE 생성/적용
  - 메인 메뉴: `KeyName`으로 무기 스탯 패널 조회
  - 확장: 인벤토리, 상점, 아이콘/설명 조회
- 인스턴스 자동화
  - 인스턴스는 최소 데이터만 적용
  - `ApplyItemData()` 가상 함수 기반 자동 적용
  - 파생 클래스별 오버라이드로 메시/사야 등 반영

### 레이아웃 가이드
- 좌측: `문제 -> 해결` 비교
- 우측: Registry 흐름도 + KeyName 활용처 3개

### 참고 UML
- `Docs/ItemHierarchy_Reference_UML.md`
  - 2번 클래스 구조도
  - 3번 프론트 미리보기 시퀀스
  - 4번 실제 장비 및 스탯 적용 시퀀스
- `Docs/ItemData_Instance_Analysis.md`
  - 7.1 클래스 다이어그램
  - 7.2 장착 시 데이터 적용 시퀀스

### 참고 코드
- `Source/ProjectMHW/Public/Items/Data/ItemDataRegistry.h`
- `Source/ProjectMHW/Private/Items/Data/ItemDataRegistry.cpp`
- `Source/ProjectMHW/Public/Items/Data/MHItemDataBase.h`
- `Source/ProjectMHW/Public/Items/Instance/MHItemInstanceBase.h`
- `Source/ProjectMHW/Private/Items/Instance/MHItemInstanceBase.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHWeaponInstance.cpp`

### 코드 캡처 추천
- `ItemDataRegistry::GetItemData(...)`
- `AMHItemInstanceBase::ApplyItemData()`
- `PostEditChangeProperty()`에서 자동 반영되는 부분

### 이미지 예시 문구
- `예시 이미지: 레지스트리 에셋 에디터 화면. 여러 아이템 데이터 엔트리가 KeyName으로 정리된 모습`
- `예시 이미지: 메인 메뉴에서 무기 선택 시 스탯 패널이 함께 바뀌는 화면`

---

## Page 4

### 슬라이드 제목
`대미지 시스템`

### 발표 목표
- GAS를 전투 수치 백엔드로 사용하면서도, 액션 게임 특성에 맞게 공격 생성과 최종 계산을 분리한 구조를 설명한다.

### 핵심 카피
`공격 시점의 상태를 Spec으로 스냅샷하고, 명중 시점에는 피격자와 Execution이 최종 결과를 결정한다`

### 본문 구성
- 설계 이유
  - 공격 시작 시점과 실제 명중 시점이 다를 수 있다.
  - 버프/장비 상태를 공격 시작 순간 기준으로 고정해야 한다.
  - 향후 투사체 공격에도 확장 가능해야 한다.
- 흐름
  - Ability가 ASC 상태를 읽고 공격 Spec 생성
  - 무기 인스턴스가 Spec 보관
  - 히트박스 충돌 시 타깃에게 전달
  - 피격자가 수락 여부 판단
  - `ExecutionCalculation`이 방어력/내성/크리티컬 반영
  - `AttributeSet`이 실제 HP 반영
- 역할 분리
  - 타격자: 공격 의도 생성
  - 피격자: 수락/거절과 후처리
  - GAS: 수치 계산과 체력 반영
- 피드백
  - 델리게이트 기반 UI 갱신
  - 피격 위치 VFX/SFX/대미지 텍스트

### 레이아웃 가이드
- 상단: 한 줄 설계 문장
- 중앙: 공격 시작 -> 전달 -> 계산 -> 반영 4단 플로우
- 하단: `타격자 / 피격자 / Execution` 책임 표

### 참고 UML
- `Docs/DamageDelivery_UML.md`
  - 2번 플레이어가 몬스터를 타격하는 시퀀스
  - 4번 장착 스탯이 ASC로 반영되는 시퀀스
- `Docs/DamageTextWidget_Design.md`
  - 11.1 Sequence Diagram
  - 11.2 Damage Text Spawn Flow

### 참고 코드
- `Source/ProjectMHW/Public/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHMeleeWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHMeleeWeaponInstance.cpp`
- `Source/ProjectMHW/Public/Interfaces/MHDamageSpecReceiverInterface.h`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Public/Combat/Execution/MHDamageExecutionCalculation.h`
- `Source/ProjectMHW/Private/Combat/Execution/MHDamageExecutionCalculation.cpp`
- `Source/ProjectMHW/Public/Combat/Attributes/MHHealthAttributeSet.h`
- `Source/ProjectMHW/Private/Combat/Attributes/MHHealthAttributeSet.cpp`

### 코드 캡처 추천
- Ability에서 `DamageSpec` 만드는 부분
- 무기에서 `CurrentDamageSpec`을 전달하는 부분
- `ExecutionCalculation`의 최종 대미지 계산 부분

### 이미지 예시 문구
- `예시 이미지: 플레이어 공격이 몬스터에 명중하는 순간. 히트 이펙트와 대미지 숫자가 같이 보이는 캡처`

---

## Page 5

### 슬라이드 제목
`UI는 Viewer 역할만 담당`

### 발표 목표
- UI가 값을 계산하거나 소유하지 않고, 외부에서 전달받은 상태를 보여주기만 하는 구조임을 설명한다.

### 핵심 카피
`UI는 상태의 주인이 아니라, 외부 상태를 소비하는 최종 표시 레이어다`

### 본문 구성
- 원칙
  - UI는 값과 로직을 소유하지 않음
  - 캐릭터/ASC/컨트롤러/레지스트리가 상태를 관리
- 인게임 UI
  - 플레이어가 Attribute 변경 델리게이트를 받고 UI로 전달
  - UI는 체력, 스태미나, 예리도만 표시
- 메인 메뉴 UI
  - 선택 무기의 `KeyName`으로 레지스트리 조회
  - 슬롯과 스탯 패널은 조회 결과만 표시
- 강조 포인트
  - UI와 게임플레이 로직이 강하게 결합되지 않음
  - 데이터 흐름 추적이 쉬움

### 레이아웃 가이드
- 좌측: 인게임 HUD 갱신 흐름
- 우측: 메인 메뉴 프리뷰 흐름

### 참고 UML
- `Docs/UI_Viewer_Role_UML.md`
  - 1번 클래스 구조
  - 2번 인게임 HUD 갱신 시퀀스
  - 3번 메인 메뉴 프리뷰 시퀀스
- `Docs/Sharpness_UML.md`
  - UI Flow
- `Docs/Frontend_WeaponStatPanel_UML.md`
  - 스탯 패널 갱신 시퀀스

### 참고 코드
- `Source/ProjectMHW/Public/Widgets/MHUserWidgetBase.h`
- `Source/ProjectMHW/Private/Widgets/MHUserWidgetBase.cpp`
- `Source/ProjectMHW/Public/Widgets/MHPlayerStatusWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHPlayerStatusWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHProgressBarWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHProgressBarWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHMainMenuWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHMainMenuWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHWeaponStatPanelWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHWeaponStatPanelWidget.cpp`

### 코드 캡처 추천
- `GetGameplayAttributeValueChangeDelegate` 바인딩 부분
- `MHProgressBarWidget`의 값 반영 함수
- 메인 메뉴에서 무기 스탯 패널을 갱신하는 부분

### 이미지 예시 문구
- `예시 이미지: 인게임 HUD의 체력/스태미나/예리도 UI가 보이는 화면`
- `예시 이미지: 메인 메뉴에서 무기 선택 슬롯과 스탯 패널이 같이 보이는 화면`

---

## Page 6

### 슬라이드 제목
`한계와 개선안`

### 발표 목표
- 결과만 보여주지 않고, 구조적으로 어디가 아쉬웠는지와 다음 단계 방향을 함께 제시한다.

### 핵심 카피
`현재 구조는 확장 가능하지만, 책임 분리와 공용화 측면에서 더 개선할 여지가 있다`

### 본문 구성
- 한계
  - 아이템 데이터 레지스트리
    - 전역 접근을 위한 아이템 데이터 서브시스템이 구축하지 못했다. 현재는 각 사용자가 레지스트리를 직접 참조하는 방식이다
  - 대미지 시스템
    - VFX, SFX, 대미지 텍스트 생성 책임이 피격자 쪽에 모여 있어 역할 분리가 덜 되어 있다.
    - 현재 구조는 잘 동작하지만, 피드백 시스템이 커질수록 관리 포인트가 늘어날 수 있다.

- 개선안
  - 아이템 데이터 레지스트리
    - 레지스트리 조회 API를 Subsystem 하나로 통일해 참조 경로를 단순화
  - 대미지 시스템
    - VFX, SFX, 대미지 텍스트를 `Subsystem`으로 분리해 피격자 책임 축소
    - 대미지 텍스트와 피드백은 풀링 시스템까지 고려해 최적화 가능

### 레이아웃 가이드
- 좌측: `Current`
- 우측: `Better`
- 비교표 또는 Before/After 구조도 권장

### 참고 UML
- 새 UML 필요 없음
- 발표용으로 직접 `Current -> Better` 도식 제작 권장

### 참고 코드
- `Source/ProjectMHW/Private/Items/Data/ItemDataRegistry.cpp`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Private/Items/Instance/MHMeleeWeaponInstance.cpp`
- `Source/ProjectMHW/Private/Widgets/MHProgressBarWidget.cpp`

### 이미지 예시 문구
- `이미지 대신 Current vs Better 구조 비교 도식 제작 권장`

---

## Page 7

### 슬라이드 제목
`정리`

### 발표 목표
- 프로젝트의 핵심 역량을 한 문장으로 회수하고 마무리한다.

### 핵심 카피
`아이템 데이터를 중심으로 전투 시스템과 UI를 연결하되, 각 레이어의 책임을 분리해 확장 가능한 구조를 만들었다`

### 본문 구성
- 보여준 역량
  - Unreal C++ 시스템 설계
  - GAS 실전 적용
  - 데이터 구조화
  - UI와 게임플레이 연동
  - 구조적 회고

### 레이아웃 가이드
- 중앙 정렬 텍스트 1문장
- 하단에 역량 키워드 4~5개만 배치

### 참고 UML
- `Docs/ItemHierarchy_Reference_UML.md`
  - 1번 레이어 구조도를 축약해 재사용 가능

### 참고 코드
- 없음

### 이미지 예시 문구
- `예시 이미지: 발표 전체를 대표하는 전투 장면 1장 또는 축약 아키텍처 다이어그램`

---

## 전체 참고 UML 리스트
- `Docs/ItemHierarchy_Reference_UML.md`
- `Docs/ItemData_Instance_Analysis.md`
- `Docs/DamageDelivery_UML.md`
- `Docs/DamageTextWidget_Design.md`
- `Docs/Sharpness_UML.md`
- `Docs/Frontend_WeaponStatPanel_UML.md`
- `Docs/UI_Viewer_Role_UML.md`

## 전체 참고 코드 리스트
- `Source/ProjectMHW/Public/Items/Data/ItemDataRegistry.h`
- `Source/ProjectMHW/Private/Items/Data/ItemDataRegistry.cpp`
- `Source/ProjectMHW/Public/Items/Data/MHItemDataBase.h`
- `Source/ProjectMHW/Public/Items/Instance/MHItemInstanceBase.h`
- `Source/ProjectMHW/Private/Items/Instance/MHItemInstanceBase.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHWeaponInstance.cpp`
- `Source/ProjectMHW/Public/Items/Instance/MHMeleeWeaponInstance.h`
- `Source/ProjectMHW/Private/Items/Instance/MHMeleeWeaponInstance.cpp`
- `Source/ProjectMHW/Public/Character/Player/MHPlayerCharacter.h`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Public/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.h`
- `Source/ProjectMHW/Private/AbilitySystem/Abilities/Weapon/LongSword/MHGA_LongSwordCombo.cpp`
- `Source/ProjectMHW/Public/Interfaces/MHDamageSpecReceiverInterface.h`
- `Source/ProjectMHW/Public/Combat/Execution/MHDamageExecutionCalculation.h`
- `Source/ProjectMHW/Private/Combat/Execution/MHDamageExecutionCalculation.cpp`
- `Source/ProjectMHW/Public/Combat/Attributes/MHHealthAttributeSet.h`
- `Source/ProjectMHW/Private/Combat/Attributes/MHHealthAttributeSet.cpp`
- `Source/ProjectMHW/Public/Widgets/MHUserWidgetBase.h`
- `Source/ProjectMHW/Private/Widgets/MHUserWidgetBase.cpp`
- `Source/ProjectMHW/Public/Widgets/MHPlayerStatusWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHPlayerStatusWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHProgressBarWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHProgressBarWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHMainMenuWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHMainMenuWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHWeaponStatPanelWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHWeaponStatPanelWidget.cpp`

## 제작 체크리스트
- 발표용 UML은 원본 전체보다 필요한 노드만 남긴 축약본으로 재배치
- 코드 캡처는 6~8줄 이내
- 한 슬라이드에는 메시지 1개만 유지
- 이미지가 없다면 UML과 텍스트만으로도 충분히 구성 가능
