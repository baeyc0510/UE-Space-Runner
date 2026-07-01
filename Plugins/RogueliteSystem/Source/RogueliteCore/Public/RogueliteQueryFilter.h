#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteQueryFilter.generated.h"

class URogueliteActionData;

/**
 * 쿼리 필터 기본 클래스.
 * 커스텀 필터링 로직 구현용.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class ROGUELITECORE_API URogueliteQueryFilter : public UObject
{
	GENERATED_BODY()

public:
	// 필터 통과 여부 확인
	UFUNCTION(BlueprintNativeEvent, Category = "Roguelite|Filter")
	bool PassesFilter(URogueliteActionData* Action, const FRogueliteRunState& RunState) const;
	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const;
};

/*~ Built-in Filters ~*/

/**
 * 보유 중인 액션만 통과
 */
UCLASS(DisplayName = "Is Acquired")
class ROGUELITECORE_API URogueliteFilter_IsAcquired : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 미보유 액션만 통과
 */
UCLASS(DisplayName = "Not Acquired")
class ROGUELITECORE_API URogueliteFilter_NotAcquired : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 최대 스택 미도달 액션만 통과
 */
UCLASS(DisplayName = "Not Max Stacked")
class ROGUELITECORE_API URogueliteFilter_NotMaxStacked : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 특정 태그 보유 액션만 통과
 */
UCLASS(DisplayName = "Has Tags")
class ROGUELITECORE_API URogueliteFilter_HasTags : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 필요한 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FGameplayTagContainer RequiredTags;

	// 모든 태그 필요 여부 (false면 하나만 있어도 통과)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bRequireAll = true;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

UENUM(BlueprintType)
enum class ERogueliteCompareOp : uint8
{
	// 같음
	Equal,
	// 같지 않음
	NotEqual,
	// 보다 큼
	Greater,
	// 보다 크거나 같음
	GreaterOrEqual,
	// 보다 작음
	Less,
	// 보다 작거나 같음
	LessOrEqual
};

/**
 * 수치 비교 필터
 */
UCLASS(DisplayName = "Value Compare")
class ROGUELITECORE_API URogueliteFilter_ValueCompare : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 비교할 값의 키
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (Categories = "Stat"))
	FGameplayTag Key;

	// 비교 연산자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	ERogueliteCompareOp Operator = ERogueliteCompareOp::GreaterOrEqual;

	// 비교 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	float CompareValue = 0.f;

	// RunState의 NumericData 사용 (false면 Action의 Values 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bUseRunStateValue = true;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 논리 AND 필터 (모든 서브필터 통과 필요)
 */
UCLASS(DisplayName = "AND")
class ROGUELITECORE_API URogueliteFilter_And : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 서브 필터 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", Instanced)
	TArray<URogueliteQueryFilter*> SubFilters;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 논리 OR 필터 (하나라도 통과하면 통과)
 */
UCLASS(DisplayName = "OR")
class ROGUELITECORE_API URogueliteFilter_Or : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 서브 필터 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", Instanced)
	TArray<URogueliteQueryFilter*> SubFilters;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 논리 NOT 필터 (서브필터 결과 반전)
 */
UCLASS(DisplayName = "NOT")
class ROGUELITECORE_API URogueliteFilter_Not : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 반전할 필터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", Instanced)
	URogueliteQueryFilter* SubFilter = nullptr;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};

/**
 * 미보유 액션 중 특정 태그 제외 필터.
 * 보유 중이면 통과, 미보유 + 해당 태그면 제외.
 * 용도: 슬롯 풀 상태에서 "새 아이템만 제외하고 업그레이드는 허용"
 */
UCLASS(DisplayName = "Exclude New With Tag")
class ROGUELITECORE_API URogueliteFilter_ExcludeNewWithTag : public URogueliteQueryFilter
{
	GENERATED_BODY()

public:
	// 미보유 시 제외할 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FGameplayTagContainer ExcludeTags;

	virtual bool PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const override;
};
