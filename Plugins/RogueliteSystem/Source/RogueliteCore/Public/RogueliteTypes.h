#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.generated.h"

class URogueliteActionData;
class URogueliteQueryFilter;
class URoguelitePoolPreset;

/*~ Delegates ~*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRogueliteActionChangedSignature, URogueliteActionData*, Action, int32, OldStacks, int32, NewStacks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRogueliteValueChangedSignature, FGameplayTag, Key, float, OldValue, float, NewValue);
// 비동기 액션 로드 완료 콜백 델리게이트
DECLARE_DYNAMIC_DELEGATE_OneParam(FRogueliteActionsLoadedSignature, int32, TotalCount);

/*~ Enums ~*/

UENUM(BlueprintType)
enum class ERogueliteApplyMode : uint8
{
	// 현재 값에 더하기
	Add,
	// 현재 값에 곱하기
	Multiply,
	// 값으로 덮어쓰기
	Set,
	// 현재 값과 비교해 큰 값 사용
	Max,
	// 현재 값과 비교해 작은 값 사용
	Min
};

UENUM(BlueprintType)
enum class ERogueliteFormatMode : uint8
{
	// 실수형
	Float,
	// 정수형
	Integer,
	// 퍼센트형
	Percent,
};

UENUM(BlueprintType)
enum class ERogueliteQueryMode : uint8
{
	// 필터 조건에 맞는 모든 액션
	All,
	// 미보유 액션만
	OnlyNew,
	// 보유 중인 액션만
	OnlyAcquired,
	// 미보유 OR (보유 + 미맥스스택)
	NewOrAcquired,
	// CustomFilter로 완전 제어
	Custom
};

/*~ Value Entry ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteValueEntry
{
	GENERATED_BODY()

	// 값의 키 (예: Stat.Attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Key;

	// 수치 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	// 기존 데이터에 적용하는 방식
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERogueliteApplyMode ApplyMode = ERogueliteApplyMode::Add;
	
	// 포맷팅 표시 규칙
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERogueliteFormatMode FormatMode = ERogueliteFormatMode::Float;
};

/*~ Acquired Info ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteAcquiredInfo
{
	GENERATED_BODY()

	// 현재 스택 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stacks = 0;

	// 최초 획득 시점 (게임 시간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AcquiredTime = 0.f;
};

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRoguelitePendingAcquireInfo
{
	GENERATED_BODY()

	// 획득할 Action
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<URogueliteActionData> ActionToAcquire = nullptr;
	
	// 획득할 스택 수
	UPROPERTY(BlueprintReadWrite)
	int32 Stacks = 0;
};


/*~ Slot Array Wrapper ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteSlotArray
{
	GENERATED_BODY()

	// 슬롯에 장착된 액션 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<URogueliteActionData*> Actions;
};

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteSlotSaveArray
{
	GENERATED_BODY()

	// 슬롯에 장착된 액션 경로 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSoftObjectPath> ActionPaths;
};

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteTagCountContainer
{
	GENERATED_BODY()

public:
	/** Container 초기화 */
	void Reset()
	{
		TagCountMap.Empty();
		TagContainer.Reset();
	}
	
	/** 태그 Count 증가. Count가 0에서 1이 되면 TagContainer에 추가 */
	void AddTag(const FGameplayTag& Tag, int32 Count = 1)
	{
		if (!Tag.IsValid() || Count <= 0) return;
    
		const int32 OldCount = GetTagCount(Tag);
		const int32 NewCount = OldCount + Count;
    
		TagCountMap.Add(Tag, NewCount);
    
		if (OldCount == 0)
		{
			TagContainer.AddTag(Tag);
		}
	}
    
	/** 태그 Count 감소. Count가 0이 되면 TagContainer에서 제거 */
	void RemoveTag(const FGameplayTag& Tag, int32 Count = 1)
	{
		if (!Tag.IsValid() || Count <= 0) return;
    
		const int32 OldCount = GetTagCount(Tag);
		if (OldCount <= 0) return;
    
		const int32 NewCount = OldCount - Count;
    
		if (NewCount <= 0)
		{
			TagCountMap.Remove(Tag);
			TagContainer.RemoveTag(Tag);
		}
		else
		{
			TagCountMap.Add(Tag, NewCount);
		}
	}
    
	/** 특정 태그의 현재 Count 반환 */
	int32 GetTagCount(const FGameplayTag& Tag) const
	{
		const int32* CountPtr = TagCountMap.Find(Tag);
		return CountPtr ? *CountPtr : 0;
	}
	
	/** TagContainer의 모든 Tag의 스택을 1씩 증가 */
	void AppendTags(const FGameplayTagContainer& TagsToAppend)
	{
		for (const auto& Tag : TagsToAppend)
		{
			AddTag(Tag);
		}
	}
	
	/** TagContainer의 모든 Tag의 스택을 1씩 제거  */
    void RemoveTags(const FGameplayTagContainer& TagsToRemove)
	{
		for (const auto& Tag : TagsToRemove)
		{
			RemoveTag(Tag);
		}
	}
	
	/** 현재 활성화된(Count >= 1) 태그들의 컨테이너 반환 */
	const FGameplayTagContainer& GetTags() const { return TagContainer; }
    
	/** 태그 보유 여부 (계층 구조 포함) */
	bool HasTag(const FGameplayTag& Tag) const { return TagContainer.HasTag(Tag); }
    
	/** 정확히 일치하는 태그 보유 여부 */
	bool HasTagExact(const FGameplayTag& Tag) const { return TagContainer.HasTagExact(Tag); }
    
	/** 주어진 태그들 중 하나라도 보유하는지 */
	bool HasAny(const FGameplayTagContainer& Tags) const { return TagContainer.HasAny(Tags); }
    
	/** 주어진 태그들을 모두 보유하는지 */
	bool HasAll(const FGameplayTagContainer& Tags) const { return TagContainer.HasAll(Tags); }

private:
	UPROPERTY()
	TMap<FGameplayTag, int32> TagCountMap;
    
	UPROPERTY()
	FGameplayTagContainer TagContainer;
};

/*~ Run State ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteRunState
{
	GENERATED_BODY()

	// 런 활성화 여부
	UPROPERTY(BlueprintReadOnly)
	bool bActive = false;

	// 획득한 모든 액션과 정보
	UPROPERTY(BlueprintReadOnly)
	TMap<URogueliteActionData*, FRogueliteAcquiredInfo> AcquiredActions;

	// 슬롯별 장착된 액션 목록
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FRogueliteSlotArray> Slots;

	// 현재 런의 활성 태그
	UPROPERTY(BlueprintReadOnly)
	FRogueliteTagCountContainer ActiveTagStacks;

	// 태그 키 기반 수치 데이터
	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, float> NumericData;

	// 상태 초기화
	void Reset()
	{
		bActive = false;
		AcquiredActions.Empty();
		Slots.Empty();
		ActiveTagStacks.Reset();
		NumericData.Empty();
	}

	// 액션 보유 여부 확인
	bool HasAction(URogueliteActionData* Action) const
	{
		return Action != nullptr && AcquiredActions.Contains(Action);
	}

	// 액션의 현재 스택 수 반환
	int32 GetStacks(URogueliteActionData* Action) const
	{
		if (const FRogueliteAcquiredInfo* Info = AcquiredActions.Find(Action))
		{
			return Info->Stacks;
		}
		return 0;
	}

	// 수치 데이터 조회
	float GetNumericValue(FGameplayTag Key, float DefaultValue = 0.f) const
	{
		if (const float* Value = NumericData.Find(Key))
		{
			return *Value;
		}
		return DefaultValue;
	}

	// 수치 데이터 설정
	void SetNumericValue(FGameplayTag Key, float Value)
	{
		NumericData.Add(Key, Value);
	}

	// ApplyMode에 따라 값 적용 후 결과 반환
	float ApplyValue(FGameplayTag Key, float Value, ERogueliteApplyMode Mode)
	{
		float Current = GetNumericValue(Key);
		float NewValue = Current;

		switch (Mode)
		{
		case ERogueliteApplyMode::Add:
			NewValue = Current + Value;
			break;
		case ERogueliteApplyMode::Multiply:
			NewValue = Current * Value;
			break;
		case ERogueliteApplyMode::Set:
			NewValue = Value;
			break;
		case ERogueliteApplyMode::Max:
			NewValue = FMath::Max(Current, Value);
			break;
		case ERogueliteApplyMode::Min:
			NewValue = FMath::Min(Current, Value);
			break;
		}

		SetNumericValue(Key, NewValue);
		return NewValue;
	}
};

/*~ Query ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteQuery
{
	GENERATED_BODY()
	
	// 풀 태그 직접 지정 (PoolPreset 미사용 시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Pool"))
	FGameplayTagContainer PoolTags;

	// 결과 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	// 필수 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer RequireTags;

	// 제외 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ExcludeTags;

	// 최대 스택 도달 액션 제외 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExcludeMaxStacked = true;

	// 획득 상태 필터링 모드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERogueliteQueryMode Mode = ERogueliteQueryMode::All;

	// 태그별 가중치 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> WeightModifiers;

	// 가중치 선택용 랜덤 시드 (0 = 무작위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RandomSeed = 0;
	
	// 쿼리 조건 프리셋 (선택)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URoguelitePoolPreset* PoolPreset = nullptr;
	
	// 커스텀 필터 (Mode=Custom 또는 추가 조건)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	URogueliteQueryFilter* CustomFilter = nullptr;
};

/*~ Run Save Data ~*/

USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteRunSaveData
{
	GENERATED_BODY()

	// 획득 액션 (소프트 경로 + 스택 수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FSoftObjectPath, int32> AcquiredActions;

	// 슬롯별 장착 액션 (소프트 경로)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FRogueliteSlotSaveArray> Slots;

	// 활성 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ActiveTags;

	// 수치 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> NumericData;

	// 재현용 랜덤 시드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RandomSeed = 0;

	// 총 플레이 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayTime = 0.f;
};
