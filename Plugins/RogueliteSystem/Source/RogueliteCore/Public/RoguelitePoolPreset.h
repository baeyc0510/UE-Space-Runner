#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RoguelitePoolPreset.generated.h"

class URogueliteQueryFilter;

/**
 * 자주 사용하는 쿼리 조건을 저장하는 프리셋.
 * 레벨업 풀, 상점 풀 등 반복 사용되는 조건을 에셋으로 정의.
 */
UCLASS(BlueprintType, Blueprintable)
class ROGUELITECORE_API URoguelitePoolPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/*~ Basic Filters ~*/

	// 풀 태그 (Pool.LevelUp, Pool.Shop 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Filters", meta = (Categories = "Pool"))
	FGameplayTagContainer PoolTags;

	// 필수 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Filters")
	FGameplayTagContainer RequireTags;

	// 제외 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Filters")
	FGameplayTagContainer ExcludeTags;

	/*~ Default Settings ~*/

	// 최대 스택 도달 액션 제외 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	bool bExcludeMaxStacked = true;
	
	// 기본 쿼리 모드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	ERogueliteQueryMode DefaultMode = ERogueliteQueryMode::NewOrAcquired;

	/*~ Advanced ~*/

	// 추가 필터 (선택)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Advanced", Instanced)
	URogueliteQueryFilter* AdditionalFilter = nullptr;
};
