#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "RogueliteActionDatabase.h"
#include "RoguelitePoolPreset.h"
#include "RogueliteQueryFilter.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"

/*~ USubsystem Interface ~*/

void URogueliteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Database 생성
	Database = NewObject<URogueliteActionDatabase>(this);
}

void URogueliteSubsystem::Deinitialize()
{
	if (RunState.bActive)
	{
		EndRun(false);
	}

	if (IsValid(Database))
	{
		Database->UnregisterAllActions();
	}
	PreAcquireChecks.Empty();

	Super::Deinitialize();
}

/*~ Static Access ~*/

URogueliteSubsystem* URogueliteSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<URogueliteSubsystem>();
}

/*~ ActionDB ~*/

URogueliteActionDatabase* URogueliteSubsystem::GetDatabase() const
{
	return Database;
}

void URogueliteSubsystem::RegisterAction(URogueliteActionData* Action)
{
	if (IsValid(Database))
	{
		Database->RegisterAction(Action);
	}
}

void URogueliteSubsystem::UnregisterAction(URogueliteActionData* Action)
{
	if (IsValid(Database))
	{
		Database->UnregisterAction(Action);
	}
}

void URogueliteSubsystem::LoadAndRegisterActions(FPrimaryAssetType AssetType)
{
	if (IsValid(Database))
	{
		Database->LoadAndRegisterActions(AssetType);
	}
}

void URogueliteSubsystem::LoadAndRegisterActionsAsync(FPrimaryAssetType AssetType, FRogueliteActionsLoadedSignature OnLoaded)
{
	if (IsValid(Database))
	{
		Database->LoadAndRegisterActionsAsync(AssetType, OnLoaded);
	}
	else
	{
		OnLoaded.ExecuteIfBound(0);
	}
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAllActions() const
{
	if (IsValid(Database))
	{
		return Database->GetAllActions();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetActionsByTag(FGameplayTag Tag) const
{
	if (IsValid(Database))
	{
		return Database->GetActionsByTag(Tag);
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetActionsByTags(const FGameplayTagContainer& Tags, bool bRequireAll) const
{
	if (IsValid(Database))
	{
		return Database->GetActionsByTags(Tags, bRequireAll);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Run Management ~*/

void URogueliteSubsystem::StartRun()
{
	if (RunState.bActive)
	{
		EndRun(false);
	}

	RunState.Reset();
	RunState.bActive = true;

	OnRunStarted.Broadcast();
	
	for (const FRoguelitePendingAcquireInfo& PendingAction : PendingActions)
	{
		AcquireAction(PendingAction.ActionToAcquire,PendingAction.Stacks);
	}
	PendingActions.Reset();
}

void URogueliteSubsystem::EndRun(bool bCompleted)
{
	if (!RunState.bActive)
	{
		return;
	}

	RunState.bActive = false;

	OnRunEnded.Broadcast(bCompleted);
}

bool URogueliteSubsystem::IsRunActive() const
{
	return RunState.bActive;
}

/*~ Query ~*/

TArray<URogueliteActionData*> URogueliteSubsystem::ExecuteQuery(const FRogueliteQuery& InQuery)
{
	TArray<URogueliteActionData*> Results;

	if (IsValid(Database))
	{
		Results = Database->ExecuteQuery(InQuery, &RunState);
	}

	// 이벤트 발생
	OnQueryComplete.Broadcast(InQuery, Results);

	return Results;
}

TArray<URogueliteActionData*> URogueliteSubsystem::QuerySimple(URoguelitePoolPreset* Preset, int32 Count)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolPreset = Preset;
	QueryStruct.Count = Count;
	return ExecuteQuery(QueryStruct);
}

TArray<URogueliteActionData*> URogueliteSubsystem::QueryByTag(FGameplayTag PoolTag, int32 Count)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolTags.AddTag(PoolTag);
	QueryStruct.Count = Count;
	QueryStruct.Mode = ERogueliteQueryMode::NewOrAcquired;
	return ExecuteQuery(QueryStruct);
}

/*~ Action Management ~*/

bool URogueliteSubsystem::AcquireAction(URogueliteActionData* Action, int32 StacksToAdd)
{
	FString FailReason;
	return TryAcquireAction(Action, FailReason, StacksToAdd);
}

bool URogueliteSubsystem::TryAcquireAction(URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd)
{
	if (!IsValid(Action))
	{
		OutFailReason = TEXT("Invalid action");
		return false;
	}

	if (!RunState.bActive)
	{
		OutFailReason = TEXT("Run not active");
		return false;
	}

	if (StacksToAdd <= 0)
	{
		OutFailReason = TEXT("Invalid stack count");
		return false;
	}

	// 획득 전 체크
	for (const FRoguelitePreAcquireCheckSignature& Check : PreAcquireChecks)
	{
		if (Check.IsBound() && !Check.Execute(Action, RunState))
		{
			OutFailReason = TEXT("Pre-acquire check failed");
			return false;
		}
	}

	int32 OldStacks = RunState.GetStacks(Action);

	// 최대 스택 체크
	int32 NewStacks = OldStacks + StacksToAdd;
	if (Action->MaxStacks > 0)
	{
		NewStacks = FMath::Min(NewStacks, Action->MaxStacks);
	}

	if (NewStacks == OldStacks)
	{
		OutFailReason = TEXT("Already at max stacks");
		return false;
	}

	int32 ActualStacksAdded = NewStacks - OldStacks;

	// 상태 업데이트
	FRogueliteAcquiredInfo& Info = RunState.AcquiredActions.FindOrAdd(Action);
	if (OldStacks == 0)
	{
		Info.AcquiredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	}
	Info.Stacks = NewStacks;

	// 자동 효과 적용
	if (Action->bAutoApplyToRunState)
	{
		ApplyActionEffects(Action, ActualStacksAdded);	
	}
	

	// 이벤트 발생
	OnActionAcquired.Broadcast(Action, OldStacks, NewStacks);
	OnStackChanged.Broadcast(Action, OldStacks, NewStacks);

	return true;
}

void URogueliteSubsystem::AddPendingAction(URogueliteActionData* Action, int32 StacksToAdd)
{
	if (!IsValid(Action) || StacksToAdd <= 0 )
	{
		return;
	}
	
	FRoguelitePendingAcquireInfo PendingInfo;
	PendingInfo.ActionToAcquire = Action;
	PendingInfo.Stacks = StacksToAdd;
	
	PendingActions.Add(PendingInfo);
}

bool URogueliteSubsystem::RemoveAction(URogueliteActionData* Action, int32 StacksToRemove, bool bRemoveAll)
{
	if (!IsValid(Action))
	{
		return false;
	}

	if (!RunState.HasAction(Action))
	{
		return false;
	}

	int32 OldStacks = RunState.GetStacks(Action);
	int32 NewStacks = bRemoveAll ? 0 : FMath::Max(0, OldStacks - StacksToRemove);
	int32 ActualStacksRemoved = OldStacks - NewStacks;

	if (ActualStacksRemoved <= 0)
	{
		return false;
	}

	// 자동 효과 제거
	if (Action->bAutoApplyToRunState)
	{
		RemoveActionEffects(Action, ActualStacksRemoved);	
	}

	if (NewStacks == 0)
	{
		RunState.AcquiredActions.Remove(Action);
	}
	else
	{
		RunState.AcquiredActions[Action].Stacks = NewStacks;
	}

	// 이벤트 발생
	OnActionRemoved.Broadcast(Action, OldStacks, NewStacks);
	OnStackChanged.Broadcast(Action, OldStacks, NewStacks);

	return true;
}

bool URogueliteSubsystem::HasAction(URogueliteActionData* Action) const
{
	return RunState.HasAction(Action);
}

int32 URogueliteSubsystem::GetActionStacks(URogueliteActionData* Action) const
{
	return RunState.GetStacks(Action);
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAllAcquired() const
{
	TArray<URogueliteActionData*> Result;
	RunState.AcquiredActions.GetKeys(Result);
	return Result;
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAcquiredWithTag(FGameplayTag Tag) const
{
	TArray<URogueliteActionData*> Result;
	for (const auto& Pair : RunState.AcquiredActions)
	{
		if (IsValid(Pair.Key) && Pair.Key->HasTag(Tag))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

/*~ Tags ~*/

void URogueliteSubsystem::AddTagToSystem(FGameplayTag Tag)
{
	RunState.ActiveTagStacks.AddTag(Tag);
}

void URogueliteSubsystem::RemoveTagFromSystem(FGameplayTag Tag)
{
	RunState.ActiveTagStacks.RemoveTag(Tag);
}

bool URogueliteSubsystem::HasTagInSystem(FGameplayTag Tag) const
{
	return RunState.ActiveTagStacks.HasTag(Tag);
}

FGameplayTagContainer URogueliteSubsystem::GetAllTags() const
{
	return RunState.ActiveTagStacks.GetTags();
}

/*~ Numeric Data ~*/

void URogueliteSubsystem::SetRunStateValue(FGameplayTag Key, float Value)
{
	float OldValue = RunState.GetNumericValue(Key);
	if (!FMath::IsNearlyEqual(OldValue, Value))
	{
		RunState.SetNumericValue(Key, Value);
		OnRunStateValueChanged.Broadcast(Key, OldValue, Value);
	}
}

float URogueliteSubsystem::GetRunStateValue(FGameplayTag Key, float DefaultValue) const
{
	return RunState.GetNumericValue(Key, DefaultValue);
}

float URogueliteSubsystem::AddRunStateValue(FGameplayTag Key, float Delta)
{
	float OldValue = RunState.GetNumericValue(Key);
	float NewValue = OldValue + Delta;
	if (!FMath::IsNearlyEqual(OldValue, NewValue))
	{
		RunState.SetNumericValue(Key, NewValue);
		OnRunStateValueChanged.Broadcast(Key, OldValue, NewValue);
	}
	return NewValue;
}

TMap<FGameplayTag, float> URogueliteSubsystem::GetAllRunStateValues() const
{
	return RunState.NumericData;
}

/*~ Slots ~*/

bool URogueliteSubsystem::EquipActionToSlot(URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (!IsValid(Action) || !SlotTag.IsValid())
	{
		return false;
	}

	if (!RunState.HasAction(Action))
	{
		return false;
	}

	FRogueliteSlotArray& SlotData = RunState.Slots.FindOrAdd(SlotTag);
	if (SlotData.Actions.Contains(Action))
	{
		return false;
	}

	SlotData.Actions.Add(Action);
	return true;
}

void URogueliteSubsystem::UnequipActionFromSlot(URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (!IsValid(Action) || !SlotTag.IsValid())
	{
		return;
	}

	if (FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		SlotData->Actions.Remove(Action);
	}
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetSlotContents(FGameplayTag SlotTag) const
{
	if (const FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		return SlotData->Actions;
	}
	return TArray<URogueliteActionData*>();
}

int32 URogueliteSubsystem::GetSlotCount(FGameplayTag SlotTag) const
{
	if (const FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		return SlotData->Actions.Num();
	}
	return 0;
}

bool URogueliteSubsystem::IsSlotFull(FGameplayTag SlotTag, int32 MaxCount) const
{
	return GetSlotCount(SlotTag) >= MaxCount;
}

/*~ Save/Load ~*/

FRogueliteRunSaveData URogueliteSubsystem::CreateRunSaveData() const
{
	FRogueliteRunSaveData SaveData;

	for (const auto& Pair : RunState.AcquiredActions)
	{
		if (IsValid(Pair.Key))
		{
			FSoftObjectPath Path(Pair.Key);
			SaveData.AcquiredActions.Add(Path, Pair.Value.Stacks);
		}
	}

	for (const auto& SlotPair : RunState.Slots)
	{
		FRogueliteSlotSaveArray& SaveSlot = SaveData.Slots.FindOrAdd(SlotPair.Key);
		for (URogueliteActionData* Action : SlotPair.Value.Actions)
		{
			if (IsValid(Action))
			{
				SaveSlot.ActionPaths.Add(FSoftObjectPath(Action));
			}
		}
	}

	SaveData.ActiveTags = RunState.ActiveTagStacks.GetTags();
	SaveData.NumericData = RunState.NumericData;

	return SaveData;
}

void URogueliteSubsystem::RestoreRunFromSaveData(const FRogueliteRunSaveData& SaveData)
{
	RunState.Reset();
	RunState.bActive = true;

	for (const auto& Pair : SaveData.AcquiredActions)
	{
		if (UObject* Obj = Pair.Key.TryLoad())
		{
			if (URogueliteActionData* Action = Cast<URogueliteActionData>(Obj))
			{
				FRogueliteAcquiredInfo Info;
				Info.Stacks = Pair.Value;
				RunState.AcquiredActions.Add(Action, Info);
			}
		}
	}

	for (const auto& SlotPair : SaveData.Slots)
	{
		FRogueliteSlotArray& SlotData = RunState.Slots.FindOrAdd(SlotPair.Key);
		for (const FSoftObjectPath& Path : SlotPair.Value.ActionPaths)
		{
			if (UObject* Obj = Path.TryLoad())
			{
				if (URogueliteActionData* Action = Cast<URogueliteActionData>(Obj))
				{
					SlotData.Actions.Add(Action);
				}
			}
		}
	}

	RunState.ActiveTagStacks.Reset();
	RunState.ActiveTagStacks.AppendTags(SaveData.ActiveTags);
	RunState.NumericData = SaveData.NumericData;
}

/*~ Pre-Acquire Check ~*/

void URogueliteSubsystem::RegisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate)
{
	PreAcquireChecks.Add(CheckDelegate);
}

void URogueliteSubsystem::UnregisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate)
{
	PreAcquireChecks.Remove(CheckDelegate);
}

void URogueliteSubsystem::ApplyActionEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	for (const FRogueliteValueEntry& Entry : Action->Values)
	{
		for (int32 i = 0; i < Stacks; ++i)
		{
			float OldValue = RunState.GetNumericValue(Entry.Key);
			float NewValue = RunState.ApplyValue(Entry.Key, Entry.Value, Entry.ApplyMode);
			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnRunStateValueChanged.Broadcast(Entry.Key, OldValue, NewValue);
			}
		}
	}

	if (!Action->TagsToGrant.IsEmpty())
	{
		RunState.ActiveTagStacks.AppendTags(Action->TagsToGrant);
	}
}

void URogueliteSubsystem::RemoveActionEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	for (const FRogueliteValueEntry& Entry : Action->Values)
	{
		for (int32 i = 0; i < Stacks; ++i)
		{
			float OldValue = RunState.GetNumericValue(Entry.Key);
			float NewValue = OldValue;

			// Add의 역연산
			if (Entry.ApplyMode == ERogueliteApplyMode::Add)
			{
				NewValue = RunState.ApplyValue(Entry.Key, -Entry.Value, ERogueliteApplyMode::Add);
			}
				
			// Multiply, Set, Max, Min은 역연산 불가 (상태가 보존되지 않음) TODO: 재계산 시스템

			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnRunStateValueChanged.Broadcast(Entry.Key, OldValue, NewValue);
			}
		}
	}
	
	// 자동 부여된 태그 제거
	if (!Action->TagsToGrant.IsEmpty())
	{
		RunState.ActiveTagStacks.RemoveTags(Action->TagsToGrant);
	}
}
