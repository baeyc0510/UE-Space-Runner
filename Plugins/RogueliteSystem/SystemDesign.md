# 로그라이트 시스템 플러그인 설계안

## 설계 원칙

| 원칙 | 설명 |
|-----|------|
| **사용 편의성 최우선** | 80%의 케이스를 최소 코드로 해결 |
| 최소 강제 | 게임에 특정 구조를 강요하지 않음 |
| 선택적 확장 | 필요한 기능만 사용 |
| 데이터 중심 | 코드 없이 에디터에서 대부분 설정 |
| GAS 독립 | 코어는 GAS 없이 동작, GAS는 선택적 확장 |
| 계층적 복잡도 | 단순→중급→고급 순으로 확장 |

---

## 모듈 구조

```
RogueliteSystem/
├── RogueliteCore        ← 필수, GAS 무관
└── RogueliteGAS         ← 선택, GAS 프로젝트용
```

---

## 계층적 복잡도

```
Level 1 (80%): 구조체 + 기본 API만으로 동작
Level 2 (15%): 델리게이트/이벤트로 커스터마이징
Level 3 (5%):  인터페이스 구현으로 완전 제어
```

---

## 핵심 개념 (Level 1)

사용자가 반드시 알아야 할 4가지:

| 개념 | 설명 |
|-----|------|
| ActionData | 획득 가능한 것의 정의 (무기, 스킬, 패시브 등) |
| ActionDB | 모든 ActionData가 등록되는 중앙 저장소 |
| RunState | 현재 런의 상태 (획득 목록, 태그 등) |
| Query | DB에서 조건에 맞는 선택지 추출 |

---

## ActionData

게임에서 획득 가능한 모든 것의 기본 정의.

```
URogueliteActionData : UPrimaryDataAsset
├── 표시
│   ├── DisplayName: FText
│   ├── Description: FText (템플릿 지원: "{Value.Amount}")
│   └── Icon: TSoftObjectPtr<UTexture2D>
│
├── 분류 태그
│   └── Tags: FGameplayTagContainer
│       ├── Type.Weapon, Type.Passive 등
│       ├── Rarity.Common, Rarity.Rare 등
│       └── Pool.LevelUp, Pool.Shop 등    ← 소속 풀도 태그로!
│
├── 수치
│   └── Values: TArray<FRogueliteValueEntry>
│       └── FRogueliteValueEntry
│           ├── Key: FGameplayTag
│           ├── Value: float
│           └── ApplyMode: ERogueliteApplyMode (Add, Multiply, Set, Max, Min)
│
├── 자동 적용 옵션
│   ├── bAutoApplyToRunState: bool = true
│   └── bAutoGrantTags: bool = false
│
├── 풀 설정
│   ├── BaseWeight: float (기본 등장 가중치)
│   └── MaxStacks: int32 (0 = 무제한)
│
└── 조건
    ├── RequiredTags: FGameplayTagContainer (이 태그들 보유 시 등장)
    └── BlockedByTags: FGameplayTagContainer (이 태그들 보유 시 제외)
```

### 커스텀 ActionData

게임별로 상속하여 확장 가능:

```cpp
// 예시: VS식 무기
UCLASS()
class UWeaponActionData : public URogueliteActionData
{
    float BaseDamage;
    float AttackInterval;
    TSubclassOf<AProjectile> ProjectileClass;

    // 진화 정보
    TSoftObjectPtr<URogueliteActionData> RequiredPassive;
    TSoftObjectPtr<UWeaponActionData> EvolvedWeapon;
};
```

---

## ActionDB (중앙 저장소)

모든 ActionData가 등록되는 Subsystem 내 저장소.

```
URogueliteSubsystem
└── ActionDB
    ├── AllActions: TSet<URogueliteActionData*>
    ├── TagIndex: TMap<FGameplayTag, TSet<ActionData*>>  // 태그별 인덱스
    │
    ├── RegisterAction(ActionData)
    ├── UnregisterAction(ActionData)
    ├── RegisterActionsFromPath(AssetPath)    // 폴더 일괄 등록
    ├── RegisterActionsFromTable(DataTable)   // 테이블에서 등록
    │
    ├── GetAllActions() → TArray
    ├── GetActionsByTag(Tag) → TArray
    └── GetActionsByTags(Tags, MatchType) → TArray
```

### 등록 방식

```cpp
// 방식 1: 설정에서 자동 등록
UCLASS(Config=Game)
class URogueliteSettings : UDeveloperSettings
{
    // 게임 시작 시 자동 등록할 경로
    UPROPERTY(Config, EditAnywhere)
    TArray<FDirectoryPath> AutoRegisterPaths;
    // 예: /Game/Data/Actions/
};

// 방식 2: 런타임 등록
Subsystem->RegisterAction(MyAction);
Subsystem->RegisterActionsFromPath("/Game/DLC/Pack1/Actions/");

// 방식 3: Asset Registry 자동 스캔 (옵션)
Subsystem->ScanAndRegisterAllActions();
```

### Pool → 태그 기반

```
기존 Pool 방식:
Pool_LevelUp (DataAsset)
├── Weapon_Whip      ← 중복 등록 필요
├── Weapon_Knife
└── Passive_Spinach

DB + 태그 방식:
ActionDB에 모든 액션 등록, 태그로 소속 표현:
├── Weapon_Whip      [Type.Weapon, Pool.LevelUp, Pool.Shop]
├── Weapon_Knife     [Type.Weapon, Pool.LevelUp]
└── Passive_Spinach  [Type.Passive, Pool.LevelUp, Pool.Shop]

쿼리 시: RequireTags = [Pool.LevelUp]
```

---

## PoolPreset (쿼리 프리셋)

자주 쓰는 쿼리 조건을 에셋으로 정의.

```
URoguelitePoolPreset : UDataAsset
├── 기본 필터
│   ├── PoolTags: FGameplayTagContainer      // Pool.LevelUp
│   ├── RequireTags: FGameplayTagContainer
│   └── ExcludeTags: FGameplayTagContainer
│
├── 기본 설정
│   ├── DefaultMode: ERogueliteQueryMode = NewOrUpgradeable
│   └── bExcludeMaxStacked: bool = true
│
└── 고급 필터 (선택)
    └── AdditionalFilter: URogueliteQueryFilter* (Instanced)
```

에디터에서:
```
▼ PoolPreset_LevelUp
  ├── Pool Tags: [Pool.LevelUp]
  ├── Default Mode: NewOrUpgradeable
  └── Exclude Max Stacked: ☑
```

---

## RunState

현재 런의 모든 상태.

```
FRogueliteRunState
├── bActive: bool
│
├── 획득 목록
│   └── AcquiredActions: TMap<ActionData*, FRogueliteAcquiredInfo>
│       └── FRogueliteAcquiredInfo
│           ├── Stacks: int32
│           └── AcquiredTime: float
│
├── 슬롯 (순서 있음)
│   └── Slots: TMap<FGameplayTag, TArray<ActionData*>>
│
├── 태그
│   └── ActiveTags: FGameplayTagContainer
│
└── 수치 데이터
    └── NumericData: TMap<FGameplayTag, float>
```

---

## Query (하이브리드 구조)

DB에서 조건에 맞는 ActionData 추출.

```
FRogueliteQuery
├── 풀 지정 (택1, 모두 비면 전체 DB 대상)
│   ├── PoolPreset: URoguelitePoolPreset*   // 프리셋 사용
│   └── PoolTags: FGameplayTagContainer     // 직접 태그 지정
│
├── Count: int32 = 3
├── RandomSeed: int32 = 0
│
├── 공통 필터 (항상 적용)
│   ├── RequireTags: FGameplayTagContainer
│   ├── ExcludeTags: FGameplayTagContainer
│   └── bExcludeMaxStacked: bool = true
│
├── 모드 (흔한 패턴 프리셋)
│   └── Mode: ERogueliteQueryMode
│       ├── All              // 모든 액션
│       ├── OnlyNew          // 미보유만
│       ├── OnlyAcquired     // 보유한 것만
│       ├── NewOrUpgradeable // 새 것 OR 강화 가능 ← VS 레벨업용
│       └── Custom           // CustomFilter 사용
│
├── 가중치 조정
│   └── WeightModifiers: TMap<FGameplayTag, float>
│
└── 커스텀 필터 (Mode=Custom 또는 추가 조건)
    └── CustomFilter: URogueliteQueryFilter* (Instanced)
```

### 쿼리 모드 동작

| Mode | 동작 |
|------|------|
| All | 조건 맞는 모든 액션 |
| OnlyNew | 미보유 액션만 |
| OnlyAcquired | 보유 중인 액션만 |
| NewOrUpgradeable | (미보유) OR (보유 + 미맥스스택) |
| Custom | CustomFilter로 완전 제어 |

### QueryFilter (Instanced)

```cpp
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class URogueliteQueryFilter : UObject
{
    virtual bool PassesFilter(URogueliteActionData* Action,
                              const FRogueliteRunState& RunState) const;
};

// 내장 필터
URogueliteFilter_IsAcquired       // 보유 중
URogueliteFilter_NotAcquired      // 미보유
URogueliteFilter_NotMaxStacked    // 최대 스택 미도달
URogueliteFilter_HasTags          // 태그 보유
URogueliteFilter_ValueCompare     // 수치 비교
URogueliteFilter_And              // 논리 AND (중첩 가능)
URogueliteFilter_Or               // 논리 OR
URogueliteFilter_Not              // 논리 NOT
```

### 복잡한 OR 조건 예시

```cpp
// "새 아이템" OR "보유 중이면서 강화 가능"
auto* OrFilter = NewObject<URogueliteFilter_Or>();
OrFilter->SubFilters.Add(NewObject<URogueliteFilter_NotAcquired>());

auto* AndFilter = NewObject<URogueliteFilter_And>();
AndFilter->SubFilters.Add(NewObject<URogueliteFilter_IsAcquired>());
AndFilter->SubFilters.Add(NewObject<URogueliteFilter_NotMaxStacked>());
OrFilter->SubFilters.Add(AndFilter);

Query.Mode = ERogueliteQueryMode::Custom;
Query.CustomFilter = OrFilter;

// 또는 Mode로 간단히:
Query.Mode = ERogueliteQueryMode::NewOrUpgradeable;
```

---

## Subsystem

```
URogueliteSubsystem : UGameInstanceSubsystem
├── ActionDB (중앙 저장소)
│   ├── AllActions
│   ├── TagIndex
│   ├── RegisterAction / UnregisterAction
│   └── GetActionsByTag
│
├── RunState: FRogueliteRunState
│
├── 델리게이트 (Level 2)
│   ├── OnRunStarted
│   ├── OnRunEnded(bCompleted)
│   ├── OnActionAcquired(Action, NewStacks)
│   ├── OnActionRemoved(Action, RemovedStacks)
│   ├── OnPreAcquireCheck(Action, RunState) → bool
│   ├── OnQueryComplete(Query, Results)
│   └── OnStackChanged(Action, OldStacks, NewStacks)
│
└── 핸들러 등록 (Level 3)
    ├── RegisterEffectHandler(Handler)
    └── RegisterProcessor(Processor)
```

---

## API

### DB 관리

```cpp
URogueliteLibrary::RegisterAction(ActionData)
URogueliteLibrary::UnregisterAction(ActionData)
URogueliteLibrary::RegisterActionsFromPath(Path)
URogueliteLibrary::RegisterActionsFromDatabaseAsset(Path)
URogueliteLibrary::RegisterActionsFromDatatable(Path)
URogueliteLibrary::GetAllRegisteredActions() → TArray
URogueliteLibrary::GetActionsByTag(Tag) → TArray
```

### 런 관리

```cpp
URogueliteLibrary::StartRun()
URogueliteLibrary::EndRun(bCompleted)
URogueliteLibrary::IsRunActive() → bool
URogueliteLibrary::GetRunState() → FRogueliteRunState&
```

### 쿼리

```cpp
// 간단 버전 (L1)
URogueliteLibrary::QuickQuery(PoolPreset, Count) → TArray<ActionData*>
URogueliteLibrary::QuickQueryByTag(PoolTag, Count) → TArray<ActionData*>

// 전체 버전
URogueliteLibrary::Query(FRogueliteQuery) → TArray<ActionData*>
```

### 액션

```cpp
URogueliteLibrary::Acquire(Action, Target) → bool
URogueliteLibrary::TryAcquire(Action, Target, OutFailReason) → bool
URogueliteLibrary::Remove(Action, Target, bRemoveAll) → bool
URogueliteLibrary::GetStacks(Action) → int32
URogueliteLibrary::HasAction(Action) → bool
URogueliteLibrary::GetAllAcquired() → TArray<ActionData*>
URogueliteLibrary::GetAcquiredWithTag(Tag) → TArray<ActionData*>
```

### 태그

```cpp
URogueliteLibrary::AddTag(Tag)
URogueliteLibrary::RemoveTag(Tag)
URogueliteLibrary::HasTag(Tag) → bool
URogueliteLibrary::GetAllTags() → FGameplayTagContainer
```

### 수치 데이터

```cpp
URogueliteLibrary::SetNumeric(Key, Value)
URogueliteLibrary::GetNumeric(Key) → float
URogueliteLibrary::AddNumeric(Key, Delta) → float
URogueliteLibrary::GetAllNumeric() → TMap<FGameplayTag, float>
```

### 슬롯

```cpp
URogueliteLibrary::EquipToSlot(Action, SlotTag) → bool
URogueliteLibrary::UnequipFromSlot(Action, SlotTag)
URogueliteLibrary::GetSlotContents(SlotTag) → TArray<ActionData*>
URogueliteLibrary::IsSlotFull(SlotTag) → bool
```

---

## 효과 적용 시스템

### Level 1: 자동 적용

ActionData의 `bAutoApplyToRunState = true`면:
- Values → RunState.NumericData에 ApplyMode대로 적용
- bAutoGrantTags = true면 Tags → RunState.ActiveTags에 추가

**대부분의 패시브/스탯 아이템은 코드 없이 동작**

### Level 2: 델리게이트

```cpp
Subsystem->OnActionAcquired.AddDynamic(this, &UMySystem::HandleAcquire);

void UMySystem::HandleAcquire(URogueliteActionData* Action, int32 Stacks)
{
    if (Action->Tags.HasTag(WeaponTag))
    {
        SpawnWeapon(Action);
    }
}
```

### Level 3: EffectHandler 인터페이스

```cpp
IRogueliteEffectHandler
├── CanHandleAction(Action) → bool
├── ApplyEffect(Action, Target, NewStacks, OldStacks) → FRogueliteEffectResult
├── RemoveEffect(Action, Target, RemainingStacks, PrevResult)
├── GetPriority() → int32
└── ShouldBlockOthers() → bool
```

---

## 범용 구현체 (플러그인 제공)

### EffectHandlers

| 핸들러 | 용도 |
|-------|------|
| URogueliteNumericHandler | Values → NumericData 적용 |
| URogueliteTagGrantHandler | Tags → ActiveTags 부여 |
| URogueliteLogHandler | 디버그 로깅 |

### QueryFilters

| 필터 | 용도 |
|-----|------|
| URogueliteFilter_HasTags | 태그 필터 |
| URogueliteFilter_ValueCompare | 수치 비교 |
| URogueliteFilter_And/Or/Not | 논리 조합 |
| URogueliteFilter_IsAcquired | 보유 체크 |
| URogueliteFilter_NotMaxStacked | 강화 가능 체크 |

### GAS 모듈 (RogueliteGAS)

```
URogueliteGASActionData : URogueliteActionData
├── Effects: TArray<TSubclassOf<UGameplayEffect>>
└── Abilities: TArray<TSubclassOf<UGameplayAbility>>

URogueliteGEHandler : IRogueliteEffectHandler
└── GE 적용, SetByCaller로 Values 전달

URogueliteGAHandler : IRogueliteEffectHandler
└── GA 부여/제거
```

---

## 런 저장/복원

```
FRogueliteRunSaveData
├── AcquiredActions: TMap<FSoftObjectPath, int32>
├── Slots: TMap<FGameplayTag, TArray<FSoftObjectPath>>
├── ActiveTags: FGameplayTagContainer
├── NumericData: TMap<FGameplayTag, float>
├── RandomSeed: int32
└── PlayTime: float

API:
├── CreateRunSaveData() → FRogueliteRunSaveData
└── RestoreRunFromSaveData(SaveData)
```

---

## 데이터 설계 가이드

### 태그 체계 권장

```
Pool (소속 풀)
├── Pool.LevelUp
├── Pool.Shop
├── Pool.Chest
└── Pool.Boss

Type (타입)
├── Type.Weapon
├── Type.Passive
├── Type.Skill
└── Type.Consumable

Rarity (희귀도)
├── Rarity.Common
├── Rarity.Rare
├── Rarity.Epic
└── Rarity.Legendary

Stat (스탯 키)
├── Stat.Attack
├── Stat.Defense
├── Stat.MaxHealth
└── Stat.MoveSpeed

Slot (슬롯)
├── Slot.Weapon
├── Slot.Passive
└── Slot.Consumable
```

### 폴더 구조 권장

```
Content/Data/Actions/
├── Weapons/
│   ├── DA_Weapon_Whip.uasset
│   ├── DA_Weapon_Knife.uasset
│   └── ...
├── Passives/
│   ├── DA_Passive_Spinach.uasset
│   └── ...
├── Evolutions/
│   └── DA_Weapon_BloodyTear.uasset
└── PoolPresets/
    ├── PP_LevelUp.uasset
    └── PP_Shop.uasset
```

---

## 게임별 확장 예시

### Vampire Survivors식 레벨업

```cpp
void UVSGameMode::OnLevelUp()
{
    // L1: 한 줄로 끝
    FRogueliteQuery Query;
    Query.PoolTags.AddTag(Tag_Pool_LevelUp);
    Query.Mode = ERogueliteQueryMode::NewOrUpgradeable;
    Query.Count = 4;

    // 슬롯 풀이면 새 아이템 제외
    if (IsWeaponSlotFull()) Query.ExcludeTags.AddTag(Tag_Type_Weapon);
    if (IsPassiveSlotFull()) Query.ExcludeTags.AddTag(Tag_Type_Passive);

    auto Choices = URogueliteLibrary::Query(Query);
    ShowLevelUpUI(Choices);
}
```

### Vampire Survivors식 무기 진화

```cpp
// L2: 델리게이트
Subsystem->OnStackChanged.AddDynamic(this, &ThisClass::CheckEvolution);

void UVSEvolutionSystem::CheckEvolution(URogueliteActionData* Action,
                                         int32 OldStacks, int32 NewStacks)
{
    auto* Weapon = Cast<UVSWeaponData>(Action);
    if (!Weapon || !Weapon->EvolvedWeapon) return;
    if (NewStacks < Weapon->MaxStacks) return;
    if (!URogueliteLibrary::HasAction(Weapon->RequiredPassive.Get())) return;

    EvolutionReady.Add(Weapon);
}
```

### 상점 비용 체크

```cpp
// L2: 획득 전 체크
Subsystem->OnPreAcquireCheck.AddLambda([](Action, RunState) -> bool
{
    float Cost = Action->GetValue(Tag_Value_Cost);
    if (Cost > CurrentGold) return false;
    CurrentGold -= Cost;
    return true;
});
```

### DLC 액션 추가

```cpp
// 런타임에 동적 등록
void UDLCManager::LoadDLCPack(FString PackName)
{
    FString Path = FString::Printf(TEXT("/Game/DLC/%s/Actions/"), *PackName);
    URogueliteLibrary::RegisterActionsFromPath(Path);
    // 등록된 액션들은 Pool 태그에 따라 자동으로 쿼리에 포함됨
}
```

---

## 확장 지점 정리

| 레벨 | 방법 | 용도 |
|-----|------|-----|
| L1 | ActionData 필드 | 자동 스탯/태그 적용 |
| L1 | Query 구조체 + Mode | 기본 필터링 |
| L1 | PoolPreset | 쿼리 조건 재사용 |
| L2 | 델리게이트 | 획득/제거/쿼리 이벤트 처리 |
| L2 | Instanced Filter | 복잡한 쿼리 조건 |
| L2 | ActionData 상속 | 게임별 데이터 확장 |
| L2 | 동적 DB 등록 | DLC, 모드 지원 |
| L3 | EffectHandler | 완전 커스텀 효과 |
| L3 | Processor | 복잡한 처리 로직 |

---

## 제공하지 않는 것 (의도적)

| 기능 | 이유 |
|-----|------|
| XP/레벨 시스템 | 게임마다 공식이 다름 |
| 재화 시스템 | 게임마다 종류/사용처 다름 |
| 메타 진행 | SaveGame 구조 게임마다 다름 |
| UI | 스타일 게임마다 다름 |
| 전투 시스템 | 완전히 범위 밖 |
| 시너지/진화 로직 | 델리게이트로 게임이 구현 |

# 코딩 컨벤션
1. Header에서는 함수들이 어느 클래스에서 override되었는지 
/*~ SomeClass1 Interface ~*/
void Class1Func1() override;
void Class1Func2() override;
/*~ SomeClass2 Interface ~*/
void Class2Func1() override;
와 같은 형태로 작성된다.

2. 1번처럼 어떤 영역을 나타내는 주석은 /*~ Some Area ~*/ 와 같이 나타낸다. 
3. if문 아래에 무조건 {} 블록을 사용한다.
4. nullcheck는 가급적 IsValid(Some Object) 형태의 인터페이스를 활용한다.
5. 구조체 각 필드 위에는 간략한 주석을 작성한다.
6. public 함수 및 protected 함수 위에는 간략한 주석을 작성한다.
7. 주석은 한국어로 작성한다.