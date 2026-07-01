#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteActionDatabase.generated.h"

class URogueliteActionData;
class URoguelitePoolPreset;
class URogueliteQueryFilter;

/**
 * 쿼리 결과 확률 정보
 */
USTRUCT(BlueprintType)
struct ROGUELITECORE_API FRogueliteActionProbability
{
	GENERATED_BODY()

	// 대상 Action
	UPROPERTY(BlueprintReadOnly)
	URogueliteActionData* Action = nullptr;

	// 선택 확률 (0.0 ~ 1.0)
	UPROPERTY(BlueprintReadOnly)
	float Probability = 0.f;

	// 계산에 사용된 가중치
	UPROPERTY(BlueprintReadOnly)
	float Weight = 0.f;

	// 필터 통과 여부
	UPROPERTY(BlueprintReadOnly)
	bool bPassedFilter = false;
};

/**
 * Action 데이터베이스.
 * Action 등록/검색/쿼리를 담당.
 * Editor와 Runtime 모두에서 사용 가능.
 */
UCLASS(BlueprintType)
class ROGUELITECORE_API URogueliteActionDatabase : public UObject
{
	GENERATED_BODY()

public:
	/*~ 등록 ~*/

	// Action 등록
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	void RegisterAction(URogueliteActionData* Action);

	// Action 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	void UnregisterAction(URogueliteActionData* Action);

	// 모든 Action 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	void UnregisterAllActions();

	// PrimaryAssetType으로부터 Action 로드 및 등록 (동기)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	void LoadAndRegisterActions(FPrimaryAssetType AssetType);

	// PrimaryAssetType으로부터 Action 로드 및 등록 (비동기)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	void LoadAndRegisterActionsAsync(FPrimaryAssetType AssetType, FRogueliteActionsLoadedSignature OnLoaded);

	/*~ 조회 ~*/

	// 등록된 Action 수
	UFUNCTION(BlueprintPure, Category = "Roguelite|Database")
	int32 GetActionCount() const;

	// 모든 등록된 Action 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	TArray<URogueliteActionData*> GetAllActions() const;

	// 특정 태그를 가진 Action 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	TArray<URogueliteActionData*> GetActionsByTag(FGameplayTag Tag) const;

	// 태그 컨테이너로 Action 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Database")
	TArray<URogueliteActionData*> GetActionsByTags(const FGameplayTagContainer& Tags, bool bRequireAll = false) const;

	/*~ 쿼리 ~*/

	// 쿼리 실행 (RunState가 nullptr이면 모든 Action 미보유로 가정)
	TArray<URogueliteActionData*> ExecuteQuery(const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState = nullptr);

	// 프리셋으로 간편 쿼리
	TArray<URogueliteActionData*> QuerySimple(URoguelitePoolPreset* Preset, int32 Count = 3, const FRogueliteRunState* RunState = nullptr);

	// 태그로 간편 쿼리
	TArray<URogueliteActionData*> QueryByTag(FGameplayTag PoolTag, int32 Count = 3, const FRogueliteRunState* RunState = nullptr);

	/*~ 확률 계산 (Editor용) ~*/

	// 쿼리에 대한 각 Action의 선택 확률 계산
	TArray<FRogueliteActionProbability> CalculateProbabilities(const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState = nullptr);

protected:
	/*~ 내부 헬퍼 ~*/

	// 쿼리 모드에 따른 필터링
	bool PassesQueryMode(URogueliteActionData* Action, ERogueliteQueryMode Mode, const FRogueliteRunState* RunState) const;

	// 후보 필터링
	TArray<URogueliteActionData*> FilterCandidates(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState) const;

	// 가중치 기반 선택
	TArray<URogueliteActionData*> WeightedSelect(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery);

	// 가중치 계산
	TArray<float> CalculateWeights(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery) const;

private:
	/*~ 데이터 ~*/

	// 모든 등록된 Action
	UPROPERTY()
	TSet<URogueliteActionData*> AllActions;

	// 태그별 인덱스
	TMap<FGameplayTag, TSet<URogueliteActionData*>> TagIndex;
};
