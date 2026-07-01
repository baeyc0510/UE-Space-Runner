# RogueliteEditor 모듈

RogueliteCore의 에디터 확장 모듈. Action 에셋 관리 및 쿼리 시뮬레이션 도구 제공.

## 아키텍처

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           RogueliteEditor Module                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─ Action Browser ───────────────────────────────────────────────────────┐ │
│  │                                                                         │ │
│  │  ┌─ UActionBrowserSettings ─┐    ┌─ SActionBrowserTab ────────────────┐│ │
│  │  │ (UObject)                │    │ (Slate Widget)                     ││ │
│  │  │                          │    │                                    ││ │
│  │  │  FActionBrowserQuery     │◄──►│  IDetailsView (Query)              ││ │
│  │  │  FSimulatedRunState      │    │  SListView (Action List)           ││ │
│  │  │                          │    │  IDetailsView (Action Details)     ││ │
│  │  └──────────────────────────┘    └────────────────────────────────────┘│ │
│  │                                             │                          │ │
│  │                                             ▼                          │ │
│  │                               ┌─────────────────────────────┐          │ │
│  │                               │  URogueliteActionDatabase   │          │ │
│  │                               │  (from RogueliteCore)       │          │ │
│  │                               │                             │          │ │
│  │                               │  - CalculateProbabilities() │          │ │
│  │                               │  - Asset Registry 연동       │          │ │
│  │                               └─────────────────────────────┘          │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 설계 원칙

| 원칙 | 설명 |
|------|------|
| **IDetailsView 활용** | UPROPERTY 기반 자동 UI 생성, 커스텀 위젯 최소화 |
| **런타임 비의존** | Editor 전용, RogueliteCore만 참조 |
| **쿼리 로직 재사용** | URogueliteActionDatabase의 기존 로직 활용 |
| **시뮬레이션 간소화** | 에디터용 경량 상태 구조체 사용 |

---

## 핵심 구성요소

### Action Browser

프로젝트 내 모든 RogueliteActionData를 조회하고 쿼리 시뮬레이션을 수행하는 에디터 탭.

**기능**:
- 전체 ActionData 에셋 스캔 및 테이블 표시
- 쿼리 조건 설정 및 확률 계산
- 가상 RunState로 다양한 게임 상황 시뮬레이션
- 필터 통과/미통과 사유 확인

**메뉴 위치**: `Window > Roguelite > Action Browser`

---

## 데이터 구조

### FSimulatedRunState (에디터 전용)

실제 `FRogueliteRunState`의 경량 버전. 에디터 시뮬레이션에 필요한 최소 정보만 포함.

```cpp
USTRUCT()
struct FSimulatedAcquiredAction
{
    // 획득한 액션
    UPROPERTY(EditAnywhere)
    URogueliteActionData* Action;

    // 스택 수
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
    int32 Stacks = 1;
};

USTRUCT()
struct FSimulatedRunState
{
    // 획득한 액션 목록
    UPROPERTY(EditAnywhere)
    TArray<FSimulatedAcquiredAction> AcquiredActions;

    // 활성 태그
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer ActiveTags;

    // FRogueliteRunState로 변환
    FRogueliteRunState ToRunState() const;
};
```

**FRogueliteRunState 대비 차이점**:

| 항목 | FRogueliteRunState | FSimulatedRunState |
|------|--------------------|--------------------|
| 획득 액션 | `TMap<Action*, AcquiredInfo>` | `TArray<AcquiredAction>` |
| 활성 태그 | `FRogueliteTagCountContainer` | `FGameplayTagContainer` |
| Slots | 포함 | 제외 (시뮬레이션 불필요) |
| NumericData | 포함 | 제외 (시뮬레이션 불필요) |
| AcquiredTime | 포함 | 제외 (시뮬레이션 불필요) |

**TArray 사용 이유**: `TMap<UObject*, Struct>`는 DetailsView에서 편집이 어려움. TArray는 행 추가/삭제가 자연스럽고 기본 UI가 잘 동작.

### FActionBrowserQuery

쿼리 파라미터와 시뮬레이션 상태를 묶은 에디터용 구조체.

```cpp
USTRUCT()
struct FActionBrowserQuery
{
    /*~ Query Parameters ~*/

    UPROPERTY(EditAnywhere, Category = "Query")
    URoguelitePoolPreset* PoolPreset;

    UPROPERTY(EditAnywhere, Category = "Query")
    ERogueliteQueryMode Mode = ERogueliteQueryMode::NewOrAcquired;

    UPROPERTY(EditAnywhere, Category = "Query")
    FGameplayTagContainer PoolTags;

    UPROPERTY(EditAnywhere, Category = "Query")
    FGameplayTagContainer RequireTags;

    UPROPERTY(EditAnywhere, Category = "Query")
    FGameplayTagContainer ExcludeTags;

    UPROPERTY(EditAnywhere, Category = "Query")
    bool bExcludeMaxStacked = true;

    UPROPERTY(EditAnywhere, Category = "Query", meta = (ClampMin = "1", ClampMax = "100"))
    int32 Count = 3;

    UPROPERTY(EditAnywhere, Category = "Query")
    int32 RandomSeed = 0;

    /*~ Simulation ~*/

    UPROPERTY(EditAnywhere, Category = "Simulation")
    bool bUseSimulatedState = false;

    UPROPERTY(EditAnywhere, Category = "Simulation", meta = (EditCondition = "bUseSimulatedState"))
    FSimulatedRunState SimulatedState;

    // FRogueliteQuery로 변환
    FRogueliteQuery ToQuery() const;
};
```

---

## UI 구현 전략

### IDetailsView 활용

| 프로퍼티 타입 | 자동 생성 위젯 |
|--------------|---------------|
| `URoguelitePoolPreset*` | SObjectPropertyEntryBox |
| `ERogueliteQueryMode` | SEnumComboBox |
| `FGameplayTagContainer` | SGameplayTagContainerWidget |
| `bool` | SCheckBox |
| `int32` | SSpinBox |
| `TArray<FSimulatedAcquiredAction>` | 배열 편집 UI (행 추가/삭제) |

**장점**:
- 커스텀 위젯 구현 불필요
- UPROPERTY 메타데이터 자동 적용 (ClampMin, EditCondition 등)
- 일관된 에디터 UX

### 위젯 구조

```
SActionBrowserTab
├── SVerticalBox (Left Panel)
│   ├── SExpandableArea (Query Settings)
│   │   └── IDetailsView ← UActionBrowserSettings::Query
│   │       └── FActionBrowserQuery (자동 렌더링)
│   │           ├── Query Parameters (자동)
│   │           └── FSimulatedRunState (자동)
│   │
│   └── SVerticalBox (Action List)
│       ├── SSearchBox
│       └── SListView<FActionRowData>
│
├── SSplitter
│
└── IDetailsView (Right Panel) ← 선택된 URogueliteActionData (읽기 전용)
```

---

## 클래스 구조

```
FRogueliteEditorModule
├── StartupModule()
│   └── RegisterTabSpawner("RogueliteActionBrowser")
└── ShutdownModule()
    └── UnregisterTabSpawner()

UActionBrowserSettings : UObject
├── FActionBrowserQuery Query
├── ToQuery() → FRogueliteQuery
└── ToRunState() → FRogueliteRunState

SActionBrowserTab : SCompoundWidget
├── UActionBrowserSettings* Settings
├── TSharedPtr<IDetailsView> QueryDetailsView
├── TSharedPtr<IDetailsView> ActionDetailsView
├── TSharedPtr<SListView<FActionRowData>> ActionListView
├── URogueliteActionDatabase* Database
│
├── Construct()
├── RefreshActionList()
├── ApplyQuery()
├── OnActionSelected()
└── OnQueryPropertyChanged()

FActionRowData
├── TWeakObjectPtr<URogueliteActionData> Action
├── float Probability
├── float Weight
├── bool bPassedFilter
├── FString FilterReason
├── int32 OwnedStacks
└── int32 MaxStacks
```

---

## 주요 흐름

### 초기화

```
1. 탭 스폰
2. UActionBrowserSettings 생성
3. IDetailsView 생성 및 Settings 바인딩
4. URogueliteActionDatabase 생성
5. AssetRegistry로 모든 URogueliteActionData 스캔
6. Database에 등록
7. 기본 쿼리로 확률 계산
8. ActionListView 갱신
```

### Query 적용

```
1. IDetailsView에서 프로퍼티 변경 감지 (OnFinishedChangingProperties)
2. Settings->ToQuery()로 FRogueliteQuery 생성
3. bUseSimulatedState 확인
   - true: Settings->Query.SimulatedState.ToRunState()
   - false: 빈 FRogueliteRunState
4. Database->CalculateProbabilities(Query, RunState)
5. FActionRowData 배열 생성
6. ActionListView 갱신
7. StatusBar 업데이트
```

---

## 문서 목록

| 문서 | 설명 |
|------|------|
| README.md | 모듈 개요 및 시스템 설계 (현재 문서) |
| ActionBrowserDesign.md | Action Browser UI 상세 설계서 |
