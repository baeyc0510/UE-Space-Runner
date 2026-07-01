#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "ActionBrowserTypes.generated.h"

class URogueliteActionData;
class URoguelitePoolPreset;

/*~ 에디터 시뮬레이션용 획득 액션 항목 ~*/
USTRUCT()
struct ROGUELITEEDITOR_API FSimulatedAcquiredAction
{
	GENERATED_BODY()

	// 획득한 액션
	UPROPERTY(EditAnywhere)
	URogueliteActionData* Action = nullptr;

	// 스택 수
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 Stacks = 1;
};

/*~ 에디터 시뮬레이션용 런 상태 ~*/
USTRUCT()
struct ROGUELITEEDITOR_API FSimulatedRunState
{
	GENERATED_BODY()

	// 획득한 액션 목록
	UPROPERTY(EditAnywhere)
	TArray<FSimulatedAcquiredAction> AcquiredActions;

	// 활성 태그
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ActiveTags;

	// FRogueliteRunState로 변환
	FRogueliteRunState ToRunState() const;
};

/*~ 에디터 쿼리 설정 ~*/
USTRUCT()
struct ROGUELITEEDITOR_API FActionBrowserQuery
{
	GENERATED_BODY()

	// 쿼리 설정 (기존 구조체 재사용)
	UPROPERTY(EditAnywhere, Category = "Query")
	FRogueliteQuery Query;

	// 시뮬레이션 활성화
	UPROPERTY(EditAnywhere, Category = "Simulation")
	bool bUseSimulatedState = false;

	// 시뮬레이션 상태
	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (EditCondition = "bUseSimulatedState"))
	FSimulatedRunState SimulatedState;
};

/*~ Action List 행 데이터 ~*/
struct ROGUELITEEDITOR_API FActionRowData
{
	// Action 참조
	TWeakObjectPtr<URogueliteActionData> Action;

	// 에셋 폴더 경로
	FString Folder;

	// 계산된 값
	float Probability = 0.f;
	float Weight = 0.f;
	bool bPassedFilter = false;

	// 시뮬레이션 상태
	int32 OwnedStacks = 0;
	int32 MaxStacks = 0;
};

// TSharedPtr용 타입 별칭
using FActionRowDataPtr = TSharedPtr<FActionRowData>;
