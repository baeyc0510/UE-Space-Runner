# Roguelite Action Browser - UI 설계서

## 1. 개요

### 1.1 목적
- 프로젝트 내 모든 RogueliteActionData 에셋을 테이블 형태로 조회
- 쿼리 조건 설정 및 각 Action의 선택 확률 시각화
- 가상 RunState를 설정하여 다양한 게임 상황 시뮬레이션

### 1.2 기술 스택
- Slate UI Framework
- **IDetailsView**: UPROPERTY 기반 자동 UI 생성 (커스텀 위젯 최소화)
- URogueliteActionDatabase (쿼리 로직 재사용)
- Editor Module (Runtime 비의존)

---

## 2. 전체 레이아웃

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│  Title Bar                                                              [_][□][X]│
├──────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│  ┌─ Left Panel ─────────────────────────────┬─ Right Panel ───────────────────┐ │
│  │                                          │                                  │ │
│  │ ┌─ Query Settings (Collapsible) ── [▼] ┐ │  ┌─ Details ──────────────────┐ │ │
│  │ │                                       │ │  │                            │ │ │
│  │ │  [Query Parameters]                   │ │  │  [Action Name]             │ │ │
│  │ │                                       │ │  │                            │ │ │
│  │ │  ┌─ Simulated RunState ─────── [▼] ┐ │ │  │  [Description]             │ │ │
│  │ │  │                                  │ │ │  │                            │ │ │
│  │ │  │  [RunState Parameters]           │ │ │  │  [Basic Info]              │ │ │
│  │ │  │                                  │ │ │  │                            │ │ │
│  │ │  └──────────────────────────────────┘ │ │  │  [Tags]                    │ │ │
│  │ │                                       │ │  │                            │ │ │
│  │ │  [Apply] [Reset]                      │ │  │  [Values]                  │ │ │
│  │ └───────────────────────────────────────┘ │  │                            │ │ │
│  │                                          │  │  [Conditions]              │ │ │
│  │ ┌─ Action List ────────────────────────┐ │  │                            │ │ │
│  │ │                                       │ │  │  [Simulated State]         │ │ │
│  │ │  Search: [______________] [☑] Filter  │ │  │                            │ │ │
│  │ │ ┌─────────────────────────────────┐  │ │  │  [Filter Status]           │ │ │
│  │ │ │ Name    │Owned│Weight│Prob│Graph│  │ │  │                            │ │ │
│  │ │ ├─────────┼─────┼──────┼────┼─────┤  │ │  │                            │ │ │
│  │ │ │▶ Action1│ 2/5 │ 100  │15% │████ │  │ │  │                            │ │ │
│  │ │ │▶ Action2│ 1/3 │  80  │12% │███  │  │ │  │                            │ │ │
│  │ │ │✗ Action3│ 0/1 │  50  │ -- │     │  │ │  │                            │ │ │
│  │ │ │▶ Action4│ 3/5 │ 120  │18% │█████│  │ │  │                            │ │ │
│  │ │ │         ...                      │  │ │  │                            │ │ │
│  │ │ └─────────────────────────────────┘  │ │  │                            │ │ │
│  │ └───────────────────────────────────────┘ │  └────────────────────────────┘ │ │
│  │                                          │                                  │ │
│  └──────────────────────────────────────────┴──────────────────────────────────┘ │
│                                                                                  │
├──────────────────────────────────────────────────────────────────────────────────┤
│  Total: 47 │ Filtered: 32 │ Weight: 2840 │ Simulated: ON                [Refresh]│
└──────────────────────────────────────────────────────────────────────────────────┘
```

**레이아웃 구조:**
- **Left Panel** (60%): Query Settings (접이식) + Action List (세로 배치)
- **Right Panel** (40%): Details (고정)
- **SSplitter**: 좌우 패널 분할 (드래그 조절 가능)

---

## 3. 컴포넌트 상세

### 3.1 Title Bar

| 요소 | 설명 |
|------|------|
| 타이틀 | "Roguelite Action Browser" |
| 윈도우 컨트롤 | 최소화, 최대화, 닫기 (표준 에디터 탭) |

---

### 3.2 Query Settings Panel

**위젯**: `SExpandableArea` + `IDetailsView`
**기본 상태**: 펼침

> **구현 전략**: 개별 위젯을 직접 구현하지 않고 `IDetailsView`를 사용하여 `UActionBrowserSettings` 객체의 UPROPERTY를 자동 렌더링한다.

#### 3.2.1 Query Parameters

`FActionBrowserQuery` 구조체의 UPROPERTY로 정의. IDetailsView가 자동 생성하는 위젯:

| 필드 | UPROPERTY 타입 | 자동 생성 위젯 | 기본값 |
|------|----------------|----------------|--------|
| Pool Preset | `URoguelitePoolPreset*` | SObjectPropertyEntryBox | None |
| Mode | `ERogueliteQueryMode` | SEnumComboBox | NewOrAcquired |
| Pool Tags | `FGameplayTagContainer` | SGameplayTagContainerWidget | Empty |
| Require Tags | `FGameplayTagContainer` | SGameplayTagContainerWidget | Empty |
| Exclude Tags | `FGameplayTagContainer` | SGameplayTagContainerWidget | Empty |
| Exclude Max Stacked | `bool` | SCheckBox | true |
| Count | `int32` | SSpinBox (ClampMin=1, ClampMax=100) | 3 |
| Random Seed | `int32` | SSpinBox | 0 |

#### 3.2.2 Simulated RunState

`FSimulatedRunState` 구조체로 분리. 실제 `FRogueliteRunState`의 경량 버전.

> **설계 의도**: `FRogueliteRunState`는 Slots, NumericData, AcquiredTime 등 시뮬레이션에 불필요한 필드를 포함하고, `TMap<UObject*, Struct>` 형태는 DetailsView 편집이 어려움. 에디터 전용 경량 구조체를 사용하고 검색 시 변환.

| 필드 | UPROPERTY 타입 | 자동 생성 위젯 | 기본값 |
|------|----------------|----------------|--------|
| Use Simulated State | `bool` | SCheckBox | false |
| Acquired Actions | `TArray<FSimulatedAcquiredAction>` | 배열 편집 UI (자동) | Empty |
| Active Tags | `FGameplayTagContainer` | SGameplayTagContainerWidget | Empty |

**FSimulatedAcquiredAction 구조체:**

| 필드 | 타입 | 설명 |
|------|------|------|
| Action | `URogueliteActionData*` | 획득한 액션 |
| Stacks | `int32` (ClampMin=1) | 스택 수 |

**TArray 사용 이유:**
- `TMap<UObject*, Struct>`는 DetailsView에서 편집 불편
- TArray는 행 추가/삭제가 자연스럽고 기본 UI 지원
- IDetailCustomization 없이 동작

#### 3.2.3 Action Buttons

| 버튼 | 동작 |
|------|------|
| Apply Query | 현재 설정으로 쿼리 실행, 확률 재계산 |
| Reset | 모든 설정 초기화 |

> **참고**: IDetailsView의 `OnFinishedChangingProperties` 델리게이트로 실시간 갱신도 가능.

---

### 3.3 Left Panel

**위젯**: `SVerticalBox`
**내용**: Query Settings (접이식) + Action List (나머지 공간)

#### 3.3.1 Action List

##### Search Bar

| 요소 | 위젯 | 설명 |
|------|------|------|
| Search Input | `SSearchBox` | 이름/태그 필터 (실시간) |
| Show Filtered Only | `SCheckBox` | 체크 시 통과 Action만 표시 |

##### Table View

**위젯**: `SListView<FActionRowData>` + `SHeaderRow`

| 컬럼 | 너비 | 정렬 | 설명 |
|------|------|------|------|
| Status | 30px | Center | 필터 통과 아이콘 |
| Name | 150px | Left | Action 이름 (정렬 가능) |
| Owned | 60px | Center | 현재/최대 스택 |
| Weight | 70px | Right | 기본 가중치 (정렬 가능) |
| Probability | 70px | Right | 선택 확률 % (정렬 가능) |
| Graph | 120px | Left | 확률 막대그래프 |

**Status 아이콘:**

| 아이콘 | 의미 |
|--------|------|
| ▶ (녹색) | 필터 통과, 선택 가능 |
| ✗ (회색) | 필터 미통과 |
| ★ (노란색) | 이미 MaxStack 도달 |

**행 인터랙션:**
- 클릭: 해당 Action 선택, Details 패널 업데이트
- 더블클릭: Action 에셋 에디터 열기
- 우클릭: 컨텍스트 메뉴 (에셋 열기, 탐색기에서 보기)

**정렬:**
- 헤더 클릭 시 오름차순/내림차순 토글
- 기본: 확률 내림차순

#### 3.3.2 Details Panel (Right Panel)

| 섹션 | 내용 |
|------|------|
| **Header** | Action 이름 (굵게) |
| **Description** | Action.Description 텍스트 |
| **Basic Info** | Base Weight, Max Stacks |
| **Tags** | ActionTags 목록 |
| **Values** | FRogueliteValueEntry 목록 (Key, Value, Mode) |
| **Conditions** | RequiredTags, BlockedByTags |
| **Simulated State** | 현재 시뮬레이션에서의 보유 스택, 남은 스택 |
| **Filter Status** | 통과/미통과 사유 |

---

### 3.4 Status Bar

| 요소 | 설명 |
|------|------|
| Total | 등록된 전체 Action 수 |
| Filtered | 필터 통과 Action 수 |
| Weight Sum | 통과 Action들의 총 가중치 |
| Simulated | ON/OFF (시뮬레이션 상태) |
| Refresh | 에셋 재스캔 버튼 |

---

## 4. 데이터 구조

### 4.1 FSimulatedAcquiredAction

```cpp
/*~ 에디터 시뮬레이션용 획득 액션 항목 ~*/
USTRUCT()
struct FSimulatedAcquiredAction
{
    GENERATED_BODY()

    // 획득한 액션
    UPROPERTY(EditAnywhere)
    URogueliteActionData* Action = nullptr;

    // 스택 수
    UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
    int32 Stacks = 1;
};
```

### 4.2 FSimulatedRunState

```cpp
/*~ 에디터 시뮬레이션용 런 상태 (경량 버전) ~*/
USTRUCT()
struct FSimulatedRunState
{
    GENERATED_BODY()

    // 획득한 액션 목록
    UPROPERTY(EditAnywhere)
    TArray<FSimulatedAcquiredAction> AcquiredActions;

    // 활성 태그
    UPROPERTY(EditAnywhere)
    FGameplayTagContainer ActiveTags;

    // FRogueliteRunState로 변환
    FRogueliteRunState ToRunState() const
    {
        FRogueliteRunState Result;
        Result.bActive = true;

        for (const auto& Entry : AcquiredActions)
        {
            if (IsValid(Entry.Action))
            {
                FRogueliteAcquiredInfo Info;
                Info.Stacks = Entry.Stacks;
                Info.AcquiredTime = 0.f;
                Result.AcquiredActions.Add(Entry.Action, Info);
            }
        }

        Result.ActiveTagStacks.AppendTags(ActiveTags);
        return Result;
    }
};
```

### 4.3 FActionBrowserQuery

```cpp
/*~ 에디터 쿼리 설정 ~*/
USTRUCT()
struct FActionBrowserQuery
{
    GENERATED_BODY()

    /*~ Query Parameters ~*/

    // 쿼리 조건 프리셋
    UPROPERTY(EditAnywhere, Category = "Query")
    URoguelitePoolPreset* PoolPreset = nullptr;

    // 쿼리 모드
    UPROPERTY(EditAnywhere, Category = "Query")
    ERogueliteQueryMode Mode = ERogueliteQueryMode::NewOrAcquired;

    // 풀 태그 직접 지정
    UPROPERTY(EditAnywhere, Category = "Query", meta = (Categories = "Pool"))
    FGameplayTagContainer PoolTags;

    // 필수 태그
    UPROPERTY(EditAnywhere, Category = "Query")
    FGameplayTagContainer RequireTags;

    // 제외 태그
    UPROPERTY(EditAnywhere, Category = "Query")
    FGameplayTagContainer ExcludeTags;

    // 맥스스택 도달 액션 제외
    UPROPERTY(EditAnywhere, Category = "Query")
    bool bExcludeMaxStacked = true;

    // 결과 개수
    UPROPERTY(EditAnywhere, Category = "Query", meta = (ClampMin = "1", ClampMax = "100"))
    int32 Count = 3;

    // 랜덤 시드 (0 = 무작위)
    UPROPERTY(EditAnywhere, Category = "Query")
    int32 RandomSeed = 0;

    /*~ Simulation ~*/

    // 시뮬레이션 활성화
    UPROPERTY(EditAnywhere, Category = "Simulation")
    bool bUseSimulatedState = false;

    // 시뮬레이션 상태
    UPROPERTY(EditAnywhere, Category = "Simulation", meta = (EditCondition = "bUseSimulatedState"))
    FSimulatedRunState SimulatedState;

    // FRogueliteQuery로 변환
    FRogueliteQuery ToQuery() const;
};
```

### 4.4 FActionRowData

```cpp
/*~ Action List 행 데이터 ~*/
USTRUCT()
struct FActionRowData
{
    GENERATED_BODY()

    // Action 참조
    TWeakObjectPtr<URogueliteActionData> Action;

    // 계산된 값
    float Probability = 0.f;
    float Weight = 0.f;
    bool bPassedFilter = false;
    FString FilterReason;

    // 시뮬레이션 상태
    int32 OwnedStacks = 0;
    int32 MaxStacks = 0;
};
```

---

## 5. 클래스 구조

```
FRogueliteEditorModule
└── RegisterTabSpawner("RogueliteActionBrowser")

UActionBrowserSettings : UObject
├── FActionBrowserQuery Query
├── ToQuery() → FRogueliteQuery
└── ToRunState() → FRogueliteRunState (Query.SimulatedState.ToRunState())

SActionBrowserTab (Main Window)
├── UActionBrowserSettings* Settings
├── SExpandableArea (Query Settings)
│   └── IDetailsView ← Settings 바인딩 (자동 UI 생성)
│       ├── Query Parameters (자동)
│       └── FSimulatedRunState (자동)
├── SSplitter (Content)
│   ├── SVerticalBox (Left)
│   │   ├── SSearchBox
│   │   └── SListView<FActionRowData>
│   │       └── SActionRowWidget (per row)
│   │           └── SProbabilityBar
│   └── IDetailsView (Right) ← 선택된 URogueliteActionData (읽기 전용)
└── SHorizontalBox (StatusBar)

URogueliteActionDatabase (from RogueliteCore)
└── CalculateProbabilities() 재사용
```

**커스텀 구현 필요 위젯:**
- `SActionRowWidget`: 테이블 행 (확률 막대그래프 포함)
- `SProbabilityBar`: 확률 시각화 막대

**IDetailsView로 대체된 위젯:**
- ~~SQueryParametersWidget~~ → IDetailsView (FActionBrowserQuery)
- ~~SSimulatedRunStateWidget~~ → IDetailsView (FSimulatedRunState)
- ~~SActionDetailsWidget~~ → IDetailsView (URogueliteActionData)

---

## 6. 주요 흐름

### 6.1 초기화

```
1. 탭 열림
2. URogueliteActionDatabase 생성
3. AssetRegistry로 모든 URogueliteActionData 스캔
4. Database에 등록
5. 기본 쿼리로 CalculateProbabilities() 호출
6. 결과로 테이블 채움
```

### 6.2 Apply Query

```
1. IDetailsView에서 프로퍼티 변경 감지 (OnFinishedChangingProperties)
   또는 Apply 버튼 클릭
2. Settings->Query.ToQuery()로 FRogueliteQuery 생성
3. bUseSimulatedState 확인
   - true: Settings->Query.SimulatedState.ToRunState()
   - false: 빈 FRogueliteRunState
4. Database->CalculateProbabilities(Query, RunState) 호출
5. FActionRowData 배열 생성
6. SListView 갱신
7. Status Bar 업데이트
```

### 6.3 Action 선택

```
1. 행 클릭 이벤트
2. 선택된 FActionRowData 저장
3. SActionDetailsWidget 갱신
   - Action 정보 표시
   - 시뮬레이션 상태 표시
   - 필터 통과/미통과 사유 표시
```

---

## 7. 스타일 가이드

### 7.1 색상

| 요소 | 색상 (Hex) | 용도 |
|------|------------|------|
| 필터 통과 | #4CAF50 | 아이콘, 막대그래프 |
| 필터 미통과 | #9E9E9E | 아이콘, 행 배경 (투명도 50%) |
| 맥스스택 | #FFC107 | 아이콘 |
| 선택된 행 | #2196F3 | 행 배경 (투명도 30%) |
| 확률 막대 | #4CAF50 → #F44336 | 그라데이션 (높음→낮음) |

### 7.2 폰트

| 요소 | 크기 | 스타일 |
|------|------|--------|
| 섹션 헤더 | 12pt | Bold |
| 테이블 헤더 | 10pt | Bold |
| 테이블 내용 | 10pt | Regular |
| 확률 텍스트 | 10pt | Bold |
| Details 제목 | 14pt | Bold |
| Details 내용 | 10pt | Regular |

### 7.3 간격

| 요소 | 값 |
|------|-----|
| 섹션 간격 | 8px |
| 위젯 간격 | 4px |
| 패널 패딩 | 8px |
| 테이블 행 높이 | 24px |

---

## 8. 단축키

| 키 | 동작 |
|----|------|
| Ctrl+F | 검색창 포커스 |
| Enter | Apply Query |
| Escape | 검색 초기화 |
| F5 | Refresh (에셋 재스캔) |
| Delete | 선택된 Simulated Action 삭제 |

---

## 9. 에디터 통합

### 9.1 메뉴 위치

```
Window > Roguelite > Action Browser
```

### 9.2 탭 등록

```cpp
// 탭 ID
static const FName ActionBrowserTabId("RogueliteActionBrowser");

// 메뉴 확장
FLevelEditorModule::FLevelEditorMenuExtender
```

---

## 10. 향후 확장

| 기능 | 설명 | 우선순위 |
|------|------|----------|
| 프리셋 저장/로드 | 쿼리 설정을 에셋으로 저장 | 중 |
| 히스토리 | 이전 쿼리 기록 | 하 |
| 비교 모드 | 두 쿼리 결과 비교 | 하 |
| Export | CSV/JSON 내보내기 | 하 |
| 실시간 연동 | PIE 세션의 실제 RunState 연동 | 중 |
