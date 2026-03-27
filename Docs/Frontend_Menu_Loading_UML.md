# 프론트엔드 메뉴 → 로딩 → 전투 레벨 진입 UML

## 클래스 다이어그램
```mermaid
classDiagram
    direction LR

    class AMHFrontendPlayerController {
        +BeginPlay()
        +Tick(float)
        +HandleStartBattleRequested(UClass*)
        +ShowMainMenu()
        +ShowLoadingScreen()
        +RefreshLoadingScreen()
        +HandleOpenBattleLevelAfterFadeOut()
        +BattleLevel
    }

    class UMHMainMenuWidget {
        +WeaponSlotClasses
        +SelectedSlotIndex
        +OnStartBattleRequested
        +BuildWeaponSlots()
        +GetSelectedWeaponClass()
        +HandleStartButtonClicked()
        +RefreshWeaponStatPanel()
    }

    class UMHWeaponMenuSlotWidget {
        +SetSelected(bool)
        +SetItemIconBrush(FSlateBrush)
        +SetWeaponName(FText)
    }

    class UMHWeaponStatPanelWidget {
        +ApplyWeaponClass(TSubclassOf)
    }

    class UMHLoadingWidget {
        +SetProgress(float)
        +SetStatusText(FText)
        +PlayFadeIn()
        +PlayFadeOut()
    }

    class UMHFrontendGameInstance {
        +SetPendingWeaponClass(TSubclassOf)
        +GetPendingWeaponClass()
        +StartBattleTransition(TSoftObjectPtr)
        +GetLoadingProgress()
        +GetLoadingStatusText()
        +IsBattleTransitionInProgress()
        +IsPreloadCompleted()
        +OpenPendingBattleLevel()
        -PendingWeaponClass
        -PendingBattleLevel
        -PreloadHandle
    }

    class AMHWeaponInstance {
        +GetDefaultObject()
    }

    class AMHItemInstanceBase {
        +GetItemRegistry()
        +GetItemDataKey()
    }

    class UItemDataRegistry {
        +GetItemData(FName)
    }

    class UMHItemDataBase {
        +Name
        +Icon
        +IconSprite
    }

    class AMHPlayerCharacter {
        +BeginPlay()
        +SpawnAndEquipDefaultWeapon()
        +ResolveStartupWeaponClass()
        +EquipWeaponInstance()
        +DefaultWeaponClass
    }

    AMHFrontendPlayerController --> UMHMainMenuWidget : 생성 및 이벤트 바인딩
    AMHFrontendPlayerController --> UMHLoadingWidget : 생성 및 갱신
    AMHFrontendPlayerController --> UMHFrontendGameInstance : 전환 요청
    UMHMainMenuWidget --> UMHWeaponMenuSlotWidget : 슬롯 생성
    UMHMainMenuWidget --> UMHWeaponStatPanelWidget : 선택 반영
    UMHMainMenuWidget --> AMHWeaponInstance : 무기 클래스 참조
    AMHWeaponInstance --|> AMHItemInstanceBase
    AMHItemInstanceBase --> UItemDataRegistry : 데이터 조회
    UItemDataRegistry --> UMHItemDataBase : 아이콘/이름 반환
    UMHFrontendGameInstance --> AMHWeaponInstance : 선택 클래스 유지
    AMHPlayerCharacter --> UMHFrontendGameInstance : 시작 무기 조회
    AMHPlayerCharacter --> AMHWeaponInstance : 스폰 및 장착
```

## 메뉴 구성 시퀀스
```mermaid
sequenceDiagram
    participant PC as 프론트엔드 컨트롤러
    participant Menu as 메인 메뉴 위젯
    participant WeaponCDO as 무기 클래스 CDO
    participant Registry as ItemDataRegistry
    participant Data as ItemData
    participant Slot as 무기 슬롯 위젯
    participant Stat as 무기 스탯 패널

    PC->>Menu: ShowMainMenu()
    Menu->>Menu: BuildWeaponSlots()

    loop WeaponSlotClasses 순회
        Menu->>WeaponCDO: GetDefaultObject()
        Menu->>WeaponCDO: GetItemRegistry()
        Menu->>WeaponCDO: GetItemDataKey()
        WeaponCDO->>Registry: 아이템 데이터 요청
        Registry->>Data: 키로 조회
        Data-->>Menu: 이름 / 아이콘 / 스프라이트 반환
        Menu->>Slot: 슬롯 생성 및 데이터 반영
    end

    Menu->>Menu: 첫 유효 슬롯 기본 선택
    Menu->>Stat: ApplyWeaponClass(선택 무기)
```

## 전환 시작 시퀀스
```mermaid
sequenceDiagram
    participant User as 사용자
    participant Menu as 메인 메뉴 위젯
    participant PC as 프론트엔드 컨트롤러
    participant GI as 프론트엔드 게임 인스턴스
    participant Loading as 로딩 위젯

    User->>Menu: 시작 버튼 클릭
    Menu->>Menu: GetSelectedWeaponClass()
    Menu->>PC: OnStartBattleRequested(선택 무기)
    PC->>GI: SetPendingWeaponClass(선택 무기)
    PC->>PC: ShowLoadingScreen()
    PC->>Loading: AddToViewport + FadeIn
    PC->>GI: StartBattleTransition(BattleLevel)
    GI->>GI: PendingBattleLevel 저장
    GI->>GI: BeginPreload()
```

## 로딩 갱신 및 레벨 오픈 시퀀스
```mermaid
sequenceDiagram
    participant PC as 프론트엔드 컨트롤러
    participant GI as 프론트엔드 게임 인스턴스
    participant Loading as 로딩 위젯
    participant Timer as 타이머
    participant World as 목적지 월드

    loop Tick마다
        PC->>GI: GetLoadingProgress()
        GI-->>PC: 진행률
        PC->>GI: GetLoadingStatusText()
        GI-->>PC: 상태 문구
        PC->>Loading: SetProgress()
        PC->>Loading: SetStatusText()
    end

    GI-->>PC: IsPreloadCompleted() == true
    PC->>Loading: PlayFadeOut()
    alt 페이드아웃 길이 > 0
        PC->>Timer: 타이머 등록
        Timer-->>PC: 만료
    else 즉시 전환
        PC->>PC: HandleOpenBattleLevelAfterFadeOut()
    end

    PC->>GI: OpenPendingBattleLevel()
    GI->>World: OpenLevelBySoftObjectPtr()
```

## 전투 시작 시 무기 적용 시퀀스
```mermaid
sequenceDiagram
    participant GI as 프론트엔드 게임 인스턴스
    participant World as 전투 레벨
    participant Player as MHPlayerCharacter
    participant Weapon as MHWeaponInstance

    GI->>World: 레벨 오픈
    World->>Player: 플레이어 스폰
    Player->>Player: BeginPlay()
    Player->>Player: SpawnAndEquipDefaultWeapon()
    Player->>Player: ResolveStartupWeaponClass()

    alt PendingWeaponClass 유효
        Player->>GI: GetPendingWeaponClass()
        GI-->>Player: 선택 무기 클래스
        Player->>Weapon: 선택 무기 스폰
    else 기본 무기 사용
        Player->>Weapon: DefaultWeaponClass 스폰
    end

    Player->>Player: EquipWeaponInstance(Weapon)
```

## 상태 흐름도
```mermaid
stateDiagram-v2
    [*] --> 프론트엔드대기
    프론트엔드대기 --> 메뉴표시: BeginPlay
    메뉴표시 --> 무기선택완료: 유효 무기 선택
    무기선택완료 --> 로딩표시: 시작 버튼 클릭
    로딩표시 --> 프리로드중: StartBattleTransition
    프리로드중 --> 전환대기: PreloadCompleted
    전환대기 --> 레벨오픈: FadeOut 완료
    레벨오픈 --> 플레이어초기화: 목적지 레벨 BeginPlay
    플레이어초기화 --> 무기장착완료: PendingWeapon 또는 DefaultWeapon 장착
    무기장착완료 --> [*]
```

## 진행률 흐름도
```mermaid
flowchart TD
    A["시작 버튼 수락"] --> B["로딩 위젯 표시"]
    B --> C["선택 무기 저장"]
    C --> D["프리로드 시작"]
    D --> E["무기 클래스 / 목적지 레벨 비동기 로드"]
    E --> F["진행률 1.0 도달"]
    F --> G["로딩 화면 FadeOut"]
    G --> H["목적지 레벨 OpenLevel"]
    H --> I["플레이어 BeginPlay"]
    I --> J["선택 무기 장착"]
```

## 설계 메모
- 메뉴 위젯은 전환 정책을 모르는 상태로 유지되고, 선택 결과만 외부에 전달한다.
- 로딩 위젯은 표시 전용이며, 로딩 상태 계산 책임은 `UMHFrontendGameInstance`에 있다.
- `AMHPlayerCharacter`는 기존 장착 파이프라인을 유지하고 시작 무기 결정 우선순위만 프론트엔드 상태를 읽도록 확장한다.
- 현재 구조는 "전환 오버레이"에 가깝고, 별도 스트리밍 로딩 맵 구조는 아니다.
