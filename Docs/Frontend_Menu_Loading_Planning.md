# 프론트엔드 메뉴 → 로딩 → 전투 레벨 진입 기획서

## 문서 목적
이 문서는 현재 코드베이스에 구현된 프론트엔드 진입 플로우를 한국어 기준으로 정리한 기획 문서다.

대상 범위는 다음과 같다.
- 메인 메뉴 표시
- 무기 선택
- 로딩 화면 전환
- 전투 레벨 사전 로드
- 레벨 오픈
- 전투 시작 시 선택 무기 적용

## 한 줄 요약
이 플로우는 `메뉴 UI`에서 무기를 고르고, `Frontend PlayerController`가 전환을 지휘하며, `GameInstance`가 레벨 간 상태를 들고 가고, `플레이어 캐릭터`가 전투 시작 시 그 상태를 읽어 실제 장비로 반영하는 구조다.

## 설계 목표
- 프론트엔드 레벨에서 전투 진입 전용 UX를 제공한다.
- 플레이어가 전투 시작 전에 무기를 고를 수 있어야 한다.
- 선택한 무기 클래스는 레벨이 바뀌어도 유지되어야 한다.
- 전투 레벨 오픈 전에 필요한 리소스를 먼저 로드해야 한다.
- 로딩 화면은 진행률과 상태 문구를 보여줘야 한다.
- 실제 장착 로직은 기존 `AMHPlayerCharacter`의 무기 장착 경로를 재사용해야 한다.

## 현재 구현 기준 핵심 책임 분리
### 1. `UMHMainMenuWidget`
- 무기 슬롯 목록을 만든다.
- 현재 선택된 무기를 관리한다.
- 시작 버튼 클릭 시 선택된 무기 클래스를 외부로 전달한다.
- 직접 레벨을 열지 않는다.

### 2. `AMHFrontendPlayerController`
- 프론트엔드 진입 시 메인 메뉴를 띄운다.
- 메인 메뉴에서 전달된 시작 요청을 받는다.
- 선택 무기를 `GameInstance`에 저장한다.
- 로딩 화면을 띄우고 갱신한다.
- 사전 로드 완료 후 실제 레벨 오픈을 실행한다.

### 3. `UMHFrontendGameInstance`
- 선택된 무기 클래스와 목적지 레벨을 임시 상태로 보관한다.
- 프리로드 진행 상태를 관리한다.
- 로딩 화면에서 읽을 진행률과 상태 문자열을 제공한다.
- 최종적으로 `OpenLevel` 대상 레벨을 연다.

### 4. `AMHPlayerCharacter`
- 전투 레벨 시작 시 어떤 무기를 스폰할지 결정한다.
- `PendingWeaponClass`가 있으면 그것을 우선 사용한다.
- 없으면 기존 `DefaultWeaponClass`로 폴백한다.
- 최종 장착 자체는 기존 `EquipWeaponInstance()` 경로를 그대로 사용한다.

## 사용자 경험 시나리오
1. 플레이어가 프론트엔드 맵에 진입한다.
2. 메인 메뉴가 표시된다.
3. 무기 목록이 동적으로 생성된다.
4. 첫 번째 유효 무기가 기본 선택된다.
5. 플레이어가 다른 무기를 클릭하면 선택 상태와 무기 정보 패널이 갱신된다.
6. 플레이어가 시작 버튼을 누른다.
7. 메뉴는 선택된 무기 클래스만 전달한다.
8. 컨트롤러는 메뉴를 숨기고 로딩 화면을 표시한다.
9. `GameInstance`는 선택 무기와 목적지 레벨을 프리로드한다.
10. 로딩이 완료되면 로딩 화면이 페이드아웃된다.
11. 목적지 레벨이 열린다.
12. 플레이어 캐릭터 `BeginPlay()`에서 선택 무기가 실제 시작 장비로 장착된다.

## 상세 플로우 설계
### A. 프론트엔드 진입
- 프로젝트 기본 맵은 프론트엔드 레벨이다.
- 전역 `GameInstance`는 `UMHFrontendGameInstance`를 사용한다.
- 프론트엔드 전용 `PlayerController` 블루프린트가 메인 메뉴 위젯과 로딩 위젯 클래스를 참조한다.

### B. 메인 메뉴 표시
- `AMHFrontendPlayerController::BeginPlay()`에서 `ShowMainMenu()`를 호출한다.
- 입력 모드는 `UIOnly`로 설정된다.
- 마우스 커서는 표시된다.

### C. 무기 선택 UI 구성
- `UMHMainMenuWidget`은 `WeaponSlotClasses` 배열을 기준으로 슬롯을 생성한다.
- 각 슬롯은 무기 클래스의 CDO에서 `ItemRegistry + ItemDataKey`를 읽어 아이콘과 이름을 구성한다.
- 첫 번째 유효 클래스가 기본 선택값이 된다.
- 선택이 바뀌면 다음 요소가 갱신된다.
  - 선택 하이라이트
  - 선택 무기 이름
  - 무기 스탯 패널
  - 시작 버튼 활성 여부

### D. 시작 버튼 클릭
- `HandleStartButtonClicked()`는 직접 전환하지 않는다.
- 현재 선택된 무기 클래스가 유효하면 `OnStartBattleRequested` 델리게이트를 브로드캐스트한다.
- 이 설계 덕분에 메뉴 위젯은 전환 정책을 몰라도 된다.

### E. 로딩 화면 전환
- `AMHFrontendPlayerController::HandleStartBattleRequested()`가 실제 전환 시작점이다.
- 컨트롤러는 먼저 `GameInstance`에 `PendingWeaponClass`를 저장한다.
- 이어서 메인 메뉴를 접고 로딩 위젯을 최상단 뷰포트에 붙인다.
- 로딩 위젯은 단순 표시 전용이며, 진행 상태 계산은 하지 않는다.

### F. 사전 로드
- `UMHFrontendGameInstance::StartBattleTransition()`는 중복 전환을 막는다.
- 내부 상태는 다음 값으로 초기화된다.
  - `bBattleTransitionInProgress = true`
  - `bPreloadCompleted = false`
  - `CachedLoadingProgress = 0.0`
  - `CachedLoadingStatusText = "Preparing battle..."`
- 이후 `BeginPreload()`에서 비동기 로드를 시작한다.

### G. 프리로드 대상
현재 코드 기준 필수 프리로드 대상은 다음 두 가지다.
- 선택된 무기 클래스
- 목적지 전투 레벨

필요한 소프트 레퍼런스가 늘어나면 여기서 함께 확장할 수 있다.

### H. 로딩 진행률 모델
현재 진행률은 실제 맵 로딩 퍼센트를 직접 읽는 방식이 아니라, 사전 로드 진행률을 UI에 환산하는 구조다.

표현 규칙:
- 시작 직후: `0.0`
- 프리로드 시작 직후: `0.1`
- 비동기 로드 진행 중: `0.1 + HandleProgress * 0.9`
- 프리로드 완료: `1.0`

장점:
- UMG에서 안정적으로 다룰 수 있다.
- `OpenLevel` 내부 퍼센트에 의존하지 않는다.
- 플레이어가 체감하는 "전투 준비 중" 상태를 충분히 설명할 수 있다.

### I. 로딩 화면 갱신
- `AMHFrontendPlayerController::Tick()`에서 매 프레임 `RefreshLoadingScreen()`을 호출한다.
- 컨트롤러는 `GameInstance`의 진행률과 상태 텍스트를 읽어 위젯에 반영한다.
- 프리로드가 완료되면 로딩 위젯의 페이드아웃 애니메이션을 재생한다.
- 애니메이션 시간이 0보다 크면 타이머로 대기하고, 아니면 즉시 레벨 오픈으로 넘어간다.

### J. 레벨 오픈
- `HandleOpenBattleLevelAfterFadeOut()`에서 입력 모드를 `GameOnly`로 되돌린다.
- 마우스 커서를 숨긴 뒤 `GameInstance->OpenPendingBattleLevel()`을 호출한다.
- 실제 레벨 오픈은 `UGameplayStatics::OpenLevelBySoftObjectPtr()`로 처리된다.

### K. 전투 시작 시 무기 반영
- `AMHPlayerCharacter::BeginPlay()`는 `SpawnAndEquipDefaultWeapon()`을 호출한다.
- 이 함수는 `ResolveStartupWeaponClass()`로 시작 무기 클래스를 결정한다.
- 우선순위는 다음과 같다.
1. `UMHFrontendGameInstance::PendingWeaponClass`
2. `AMHPlayerCharacter::DefaultWeaponClass`
- 결정된 클래스로 무기를 스폰하고, 기존 `EquipWeaponInstance()`로 장착을 마친다.

## 현재 자산 배선 현황
코드와 별도로, 현재 에셋 배선에서 확인된 내용은 다음과 같다.

### 프론트엔드 맵
- 기본 맵은 `/Game/_BP/UI/Frontend/Lvl_Frontend`
- 프론트엔드 GameMode는 `BP_FrontendGamemode`
- 프론트엔드 PlayerController는 `BP_FrontendPlayerController`

### 프론트엔드 위젯 배선
- 메인 메뉴 위젯: `/Game/_BP/UI/Menu/WBP_MainMenu`
- 로딩 위젯: `/Game/_BP/UI/Menu/WBP_LoadingScreen`

### 목적지 레벨 배선
현재 `BP_FrontendPlayerController`에 설정된 `BattleLevel`은 `/Game/ThirdPerson/Lvl_ThirdPerson`으로 보인다.

즉, 구조는 "전투 레벨 진입 플로우"로 설계되어 있지만 현재 자산 설정상 실제 목적지는 `Variant_Combat/Lvl_Combat`가 아니라 `ThirdPerson` 맵일 가능성이 높다.

## 예외 처리 정책
### 무기 슬롯이 비어 있는 경우
- 시작 버튼을 비활성화한다.
- 선택 텍스트는 빈 상태 문구를 유지한다.

### 무기 미리보기 데이터가 없는 경우
- 슬롯 생성은 유지한다.
- 아이콘 또는 이름은 폴백 표시를 사용한다.

### 목적지 레벨이 설정되지 않은 경우
- 전환 시작은 실패한다.
- 로딩 화면에는 `"Battle level asset is not configured."` 문구가 표시된다.
- 진행률은 `0.0`으로 유지된다.

### `PendingWeaponClass`가 비어 있거나 무효인 경우
- 전투 시작 시 `DefaultWeaponClass`로 폴백한다.

### 둘 다 무효인 경우
- 플레이어는 무기 없이 시작할 수 있다.
- 이 경우 로그와 디자인 정책을 별도로 강화할 필요가 있다.

## 이 설계의 장점
- UI, 전환 제어, 세션 상태, 실제 장착 로직의 책임이 분리되어 있다.
- 레벨이 바뀌어도 `GameInstance`를 통해 선택 상태를 보존할 수 있다.
- 기존 전투 캐릭터의 장착 로직을 재사용하므로 리스크가 낮다.
- 로딩 UI가 `OpenLevel` 내부 구현에 강하게 결합되지 않는다.
- 블루프린트 자산에서 목적지 레벨이나 위젯을 쉽게 바꿀 수 있다.

## 현재 한계와 주의점
### 1. 진짜 스트리밍 기반 로딩 맵은 아니다
- 현재 로딩 화면은 별도 로딩 레벨이 아니라 프론트엔드 레벨 위 오버레이다.
- `OpenLevel`이 시작되면 이후 프레임의 세밀한 내부 퍼센트는 추적하지 않는다.

### 2. `PendingWeaponClass` 정리 시점이 없다
- `ClearPendingWeaponClass()`는 존재하지만 현재 플로우에서 호출되지 않는다.
- 마지막에 선택한 무기 클래스가 세션 중 남아 있을 수 있다.

### 3. 전환 완료 보장은 프리로드 기준이다
- 현재 로딩 완료 조건은 "필요 자산 프리로드 완료"이지 "맵 내부 초기화 전부 완료"는 아니다.

### 4. 실제 목적지 레벨은 블루프린트 배선에 의존한다
- 코드만 보면 전투 레벨 진입 구조다.
- 하지만 어떤 맵으로 가는지는 `BP_FrontendPlayerController` 설정이 최종 결정한다.

## 후속 개선 권장 사항
- 전투 레벨 진입 직후 `PendingWeaponClass`를 명시적으로 비우는 정책 추가
- 목적지 레벨이 전투 전용 맵인지 검증하는 보호 로직 추가
- 로딩 상태 문구를 단계별로 더 세분화
- 실패 시 메인 메뉴로 복귀하는 복구 플로우 추가
- 장비 선택 외에도 난이도, 맵, 파티 옵션 등을 동일한 세션 상태로 확장

## 코드 앵커
- `Source/ProjectMHW/Private/Frontend/MHFrontendPlayerController.cpp`
- `Source/ProjectMHW/Public/Frontend/MHFrontendPlayerController.h`
- `Source/ProjectMHW/Private/System/MHFrontendGameInstance.cpp`
- `Source/ProjectMHW/Public/System/MHFrontendGameInstance.h`
- `Source/ProjectMHW/Private/Widgets/MHMainMenuWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHMainMenuWidget.h`
- `Source/ProjectMHW/Private/Widgets/MHLoadingWidget.cpp`
- `Source/ProjectMHW/Public/Widgets/MHLoadingWidget.h`
- `Source/ProjectMHW/Private/Character/Player/MHPlayerCharacter.cpp`
- `Source/ProjectMHW/Public/Character/Player/MHPlayerCharacter.h`

## 결론
현재 구현은 "프론트엔드 메뉴에서 무기를 고르고, 로딩 오버레이를 거친 뒤, 선택 무기를 유지한 채 전투 플레이를 시작한다"는 목적에 맞게 설계돼 있다.

핵심은 다음 세 줄로 요약된다.
- 메뉴는 선택만 만든다.
- 컨트롤러와 `GameInstance`가 전환을 관리한다.
- 전투 캐릭터는 기존 장착 로직으로 선택 결과를 소비한다.
