#include "RogueliteActionDatabase.h"
#include "RogueliteActionData.h"
#include "RoguelitePoolPreset.h"
#include "RogueliteQueryFilter.h"
#include "Engine/AssetManager.h"

/*~ 등록 ~*/

void URogueliteActionDatabase::RegisterAction(URogueliteActionData* Action)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (AllActions.Contains(Action))
	{
		return;
	}

	AllActions.Add(Action);

	// 태그 인덱스 업데이트
	for (const FGameplayTag& Tag : Action->ActionTags)
	{
		TagIndex.FindOrAdd(Tag).Add(Action);
	}
}

void URogueliteActionDatabase::UnregisterAction(URogueliteActionData* Action)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (!AllActions.Contains(Action))
	{
		return;
	}

	AllActions.Remove(Action);

	// 태그 인덱스에서 제거
	for (const FGameplayTag& Tag : Action->ActionTags)
	{
		if (TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
		{
			Set->Remove(Action);
		}
	}
}

void URogueliteActionDatabase::UnregisterAllActions()
{
	AllActions.Empty();
	TagIndex.Empty();
}

void URogueliteActionDatabase::LoadAndRegisterActions(FPrimaryAssetType AssetType)
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!IsValid(AssetManager))
	{
		return;
	}

	TArray<FPrimaryAssetId> AssetIds;
	AssetManager->GetPrimaryAssetIdList(AssetType, AssetIds);

	if (AssetIds.Num() == 0)
	{
		return;
	}

	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		FSoftObjectPath AssetPath = AssetManager->GetPrimaryAssetPath(AssetId);
		if (UObject* LoadedObject = AssetPath.TryLoad())
		{
			if (URogueliteActionData* Action = Cast<URogueliteActionData>(LoadedObject))
			{
				RegisterAction(Action);
			}
		}
	}
}

void URogueliteActionDatabase::LoadAndRegisterActionsAsync(FPrimaryAssetType AssetType, FRogueliteActionsLoadedSignature OnLoaded)
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!IsValid(AssetManager))
	{
		OnLoaded.ExecuteIfBound(0);
		return;
	}

	TArray<FPrimaryAssetId> AssetIds;
	AssetManager->GetPrimaryAssetIdList(AssetType, AssetIds);

	if (AssetIds.Num() == 0)
	{
		OnLoaded.ExecuteIfBound(0);
		return;
	}

	AssetManager->LoadPrimaryAssets(
		AssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateWeakLambda(this, [this, AssetIds, OnLoaded]()
		{
			UAssetManager* Manager = UAssetManager::GetIfInitialized();
			if (IsValid(Manager))
			{
				for (const FPrimaryAssetId& AssetId : AssetIds)
				{
					UObject* LoadedObject = Manager->GetPrimaryAssetObject(AssetId);
					if (URogueliteActionData* Action = Cast<URogueliteActionData>(LoadedObject))
					{
						RegisterAction(Action);
					}
				}
			}
			OnLoaded.ExecuteIfBound(AssetIds.Num());
		})
	);
}

/*~ 조회 ~*/

int32 URogueliteActionDatabase::GetActionCount() const
{
	return AllActions.Num();
}

TArray<URogueliteActionData*> URogueliteActionDatabase::GetAllActions() const
{
	return AllActions.Array();
}

TArray<URogueliteActionData*> URogueliteActionDatabase::GetActionsByTag(FGameplayTag Tag) const
{
	if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
	{
		return Set->Array();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteActionDatabase::GetActionsByTags(const FGameplayTagContainer& Tags, bool bRequireAll) const
{
	TArray<URogueliteActionData*> Result;

	if (bRequireAll)
	{
		for (URogueliteActionData* Action : AllActions)
		{
			if (IsValid(Action) && Action->HasAllTags(Tags))
			{
				Result.Add(Action);
			}
		}
	}
	else
	{
		TSet<URogueliteActionData*> ResultSet;
		for (const FGameplayTag& Tag : Tags)
		{
			if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
			{
				ResultSet.Append(*Set);
			}
		}
		Result = ResultSet.Array();
	}

	return Result;
}

/*~ 쿼리 ~*/

TArray<URogueliteActionData*> URogueliteActionDatabase::ExecuteQuery(const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState)
{
	// 풀 태그 결정
	FGameplayTagContainer EffectivePoolTags = InQuery.PoolTags;

	// 프리셋 적용
	if (IsValid(InQuery.PoolPreset))
	{
		EffectivePoolTags.AppendTags(InQuery.PoolPreset->PoolTags);
	}

	// 후보 수집
	TArray<URogueliteActionData*> Candidates;
	if (EffectivePoolTags.IsEmpty())
	{
		Candidates = AllActions.Array();
	}
	else
	{
		TSet<URogueliteActionData*> CandidateSet;
		for (const FGameplayTag& Tag : EffectivePoolTags)
		{
			if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
			{
				CandidateSet.Append(*Set);
			}
		}
		Candidates = CandidateSet.Array();
	}

	// 필터링
	TArray<URogueliteActionData*> Filtered = FilterCandidates(Candidates, InQuery, RunState);

	// 가중치 기반 선택
	return WeightedSelect(Filtered, InQuery);
}

TArray<URogueliteActionData*> URogueliteActionDatabase::QuerySimple(URoguelitePoolPreset* Preset, int32 Count, const FRogueliteRunState* RunState)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolPreset = Preset;
	QueryStruct.Count = Count;
	return ExecuteQuery(QueryStruct, RunState);
}

TArray<URogueliteActionData*> URogueliteActionDatabase::QueryByTag(FGameplayTag PoolTag, int32 Count, const FRogueliteRunState* RunState)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolTags.AddTag(PoolTag);
	QueryStruct.Count = Count;
	QueryStruct.Mode = ERogueliteQueryMode::NewOrAcquired;
	return ExecuteQuery(QueryStruct, RunState);
}

/*~ 확률 계산 ~*/

TArray<FRogueliteActionProbability> URogueliteActionDatabase::CalculateProbabilities(const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState)
{
	TArray<FRogueliteActionProbability> Results;

	// 풀 태그 결정
	FGameplayTagContainer EffectivePoolTags = InQuery.PoolTags;
	if (IsValid(InQuery.PoolPreset))
	{
		EffectivePoolTags.AppendTags(InQuery.PoolPreset->PoolTags);
	}

	// 후보 수집
	TArray<URogueliteActionData*> Candidates;
	if (EffectivePoolTags.IsEmpty())
	{
		Candidates = AllActions.Array();
	}
	else
	{
		TSet<URogueliteActionData*> CandidateSet;
		for (const FGameplayTag& Tag : EffectivePoolTags)
		{
			if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
			{
				CandidateSet.Append(*Set);
			}
		}
		Candidates = CandidateSet.Array();
	}

	// 필터링된 후보
	TArray<URogueliteActionData*> Filtered = FilterCandidates(Candidates, InQuery, RunState);
	TSet<URogueliteActionData*> FilteredSet(Filtered);

	// 가중치 계산
	TArray<float> Weights = CalculateWeights(Filtered, InQuery);

	// 총 가중치
	float TotalWeight = 0.f;
	for (float W : Weights)
	{
		TotalWeight += W;
	}

	// 모든 후보에 대해 결과 생성
	for (URogueliteActionData* Action : Candidates)
	{
		FRogueliteActionProbability Prob;
		Prob.Action = Action;
		Prob.bPassedFilter = FilteredSet.Contains(Action);

		if (Prob.bPassedFilter)
		{
			int32 FilteredIndex = Filtered.Find(Action);
			if (FilteredIndex != INDEX_NONE)
			{
				Prob.Weight = Weights[FilteredIndex];
				Prob.Probability = (TotalWeight > 0.f) ? (Prob.Weight / TotalWeight) : 0.f;
			}
		}

		Results.Add(Prob);
	}

	// 확률 높은 순으로 정렬
	Results.Sort([](const FRogueliteActionProbability& A, const FRogueliteActionProbability& B)
	{
		return A.Probability > B.Probability;
	});

	return Results;
}

/*~ 내부 헬퍼 ~*/

bool URogueliteActionDatabase::PassesQueryMode(URogueliteActionData* Action, ERogueliteQueryMode Mode, const FRogueliteRunState* RunState) const
{
	// RunState가 없으면 모든 Action을 미보유로 가정
	bool bAcquired = RunState ? RunState->HasAction(Action) : false;
	int32 CurrentStacks = RunState ? RunState->GetStacks(Action) : 0;
	bool bMaxStacked = Action->IsMaxStacked(CurrentStacks);

	switch (Mode)
	{
	case ERogueliteQueryMode::All:
		return true;

	case ERogueliteQueryMode::OnlyNew:
		return !bAcquired;

	case ERogueliteQueryMode::OnlyAcquired:
		return bAcquired;

	case ERogueliteQueryMode::NewOrAcquired:
		return !bAcquired || (bAcquired && !bMaxStacked);

	case ERogueliteQueryMode::Custom:
		return true;
	}

	return true;
}

TArray<URogueliteActionData*> URogueliteActionDatabase::FilterCandidates(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery, const FRogueliteRunState* RunState) const
{
	// Effective 값 계산
	FGameplayTagContainer EffectiveRequireTags = InQuery.RequireTags;
	FGameplayTagContainer EffectiveExcludeTags = InQuery.ExcludeTags;
	ERogueliteQueryMode EffectiveMode = InQuery.Mode;
	bool bEffectiveExcludeMaxStacked = InQuery.bExcludeMaxStacked;
	URogueliteQueryFilter* EffectiveCustomFilter = InQuery.CustomFilter;

	if (IsValid(InQuery.PoolPreset))
	{
		URoguelitePoolPreset* Preset = InQuery.PoolPreset;
		EffectiveRequireTags.AppendTags(Preset->RequireTags);
		EffectiveExcludeTags.AppendTags(Preset->ExcludeTags);

		if (InQuery.Mode == ERogueliteQueryMode::All)
		{
			EffectiveMode = Preset->DefaultMode;
		}
		bEffectiveExcludeMaxStacked = bEffectiveExcludeMaxStacked || Preset->bExcludeMaxStacked;

		if (!IsValid(EffectiveCustomFilter))
		{
			EffectiveCustomFilter = Preset->AdditionalFilter;
		}
	}

	// 활성 태그 (RunState가 없으면 빈 컨테이너)
	FGameplayTagContainer ActiveTags;
	if (RunState)
	{
		ActiveTags = RunState->ActiveTagStacks.GetTags();
	}

	TArray<URogueliteActionData*> Filtered;
	for (URogueliteActionData* Action : Candidates)
	{
		if (!IsValid(Action))
		{
			continue;
		}

		// RequireTags 체크
		if (!EffectiveRequireTags.IsEmpty() && !Action->HasAllTags(EffectiveRequireTags))
		{
			continue;
		}

		// ExcludeTags 체크
		if (!EffectiveExcludeTags.IsEmpty() && Action->HasAnyTags(EffectiveExcludeTags))
		{
			continue;
		}

		// 조건 체크 (RequiredTags, BlockedByTags)
		if (!Action->MeetsConditions(ActiveTags))
		{
			continue;
		}

		// MaxStacked 체크
		if (bEffectiveExcludeMaxStacked)
		{
			int32 CurrentStacks = RunState ? RunState->GetStacks(Action) : 0;
			if (Action->IsMaxStacked(CurrentStacks))
			{
				continue;
			}
		}

		// 모드 체크
		if (!PassesQueryMode(Action, EffectiveMode, RunState))
		{
			continue;
		}

		// 커스텀 필터 체크 (RunState가 필요하면 빈 상태 전달)
		if (IsValid(EffectiveCustomFilter))
		{
			FRogueliteRunState EmptyState;
			const FRogueliteRunState& StateRef = RunState ? *RunState : EmptyState;
			if (!EffectiveCustomFilter->PassesFilter(Action, StateRef))
			{
				continue;
			}
		}

		Filtered.Add(Action);
	}

	return Filtered;
}

TArray<float> URogueliteActionDatabase::CalculateWeights(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery) const
{
	TArray<float> Weights;
	Weights.Reserve(Candidates.Num());

	for (URogueliteActionData* Action : Candidates)
	{
		float Weight = Action->BaseWeight;

		// 가중치 배율 적용
		for (const auto& Modifier : InQuery.WeightModifiers)
		{
			if (Action->HasTag(Modifier.Key))
			{
				Weight *= Modifier.Value;
			}
		}

		Weights.Add(FMath::Max(Weight, 0.f));
	}

	return Weights;
}

TArray<URogueliteActionData*> URogueliteActionDatabase::WeightedSelect(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery)
{
	if (Candidates.Num() == 0 || InQuery.Count <= 0)
	{
		return TArray<URogueliteActionData*>();
	}

	if (Candidates.Num() <= InQuery.Count)
	{
		return Candidates;
	}

	// 랜덤 스트림 설정
	FRandomStream RandomStream;
	if (InQuery.RandomSeed != 0)
	{
		RandomStream.Initialize(InQuery.RandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}

	// 가중치 계산
	TArray<float> Weights = CalculateWeights(Candidates, InQuery);

	// 가중치 기반 선택
	TArray<URogueliteActionData*> Results;
	TArray<int32> AvailableIndices;
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		AvailableIndices.Add(i);
	}

	for (int32 i = 0; i < InQuery.Count && AvailableIndices.Num() > 0; ++i)
	{
		float TotalWeight = 0.f;
		for (int32 Idx : AvailableIndices)
		{
			TotalWeight += Weights[Idx];
		}

		if (TotalWeight <= 0.f)
		{
			// 모든 가중치가 0이면 균등 확률
			int32 RandomIdx = RandomStream.RandRange(0, AvailableIndices.Num() - 1);
			Results.Add(Candidates[AvailableIndices[RandomIdx]]);
			AvailableIndices.RemoveAt(RandomIdx);
		}
		else
		{
			float Random = RandomStream.FRandRange(0.f, TotalWeight);
			float Cumulative = 0.f;

			for (int32 j = 0; j < AvailableIndices.Num(); ++j)
			{
				int32 Idx = AvailableIndices[j];
				Cumulative += Weights[Idx];

				if (Random <= Cumulative)
				{
					Results.Add(Candidates[Idx]);
					AvailableIndices.RemoveAt(j);
					break;
				}
			}
		}
	}

	return Results;
}
