#include "RogueliteStatReceiverComponent.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"

URogueliteStatReceiverComponent::URogueliteStatReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/*~ UActorComponent Interface ~*/

void URogueliteStatReceiverComponent::BeginPlay()
{
	Super::BeginPlay();

	// 초기 스탯 적용
	for (const auto& Pair : DefaultStats)
	{
		CurrentStats.Add(Pair.Key, Pair.Value);
	}

	// Subsystem 이벤트 바인딩
	if (URogueliteSubsystem* Subsystem = URogueliteSubsystem::Get(this))
	{
		CachedSubsystem = Subsystem;

		Subsystem->OnActionAcquired.AddDynamic(this, &URogueliteStatReceiverComponent::HandleActionAcquired);
		Subsystem->OnActionRemoved.AddDynamic(this, &URogueliteStatReceiverComponent::HandleActionRemoved);
		Subsystem->OnStackChanged.AddDynamic(this, &URogueliteStatReceiverComponent::HandleStackChanged);
		Subsystem->OnRunEnded.AddDynamic(this, &URogueliteStatReceiverComponent::HandleRunEnded);

		// 이미 획득된 Action 동기화
		SyncExistingActions();
	}
}

void URogueliteStatReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Subsystem 이벤트 언바인딩
	if (URogueliteSubsystem* Subsystem = CachedSubsystem.Get())
	{
		Subsystem->OnActionAcquired.RemoveDynamic(this, &URogueliteStatReceiverComponent::HandleActionAcquired);
		Subsystem->OnActionRemoved.RemoveDynamic(this, &URogueliteStatReceiverComponent::HandleActionRemoved);
		Subsystem->OnStackChanged.RemoveDynamic(this, &URogueliteStatReceiverComponent::HandleStackChanged);
		Subsystem->OnRunEnded.RemoveDynamic(this, &URogueliteStatReceiverComponent::HandleRunEnded);
	}

	Super::EndPlay(EndPlayReason);
}

/*~ Stat Access ~*/

float URogueliteStatReceiverComponent::GetStat(FGameplayTag Key, float DefaultValue) const
{
	if (const float* Value = CurrentStats.Find(Key))
	{
		return *Value;
	}
	return DefaultValue;
}

void URogueliteStatReceiverComponent::SetStat(FGameplayTag Key, float Value)
{
	const float OldValue = GetStat(Key);
	CurrentStats.Add(Key, Value);

	if (!FMath::IsNearlyEqual(OldValue, Value))
	{
		OnStatChanged.Broadcast(Key, OldValue, Value);
	}
}

float URogueliteStatReceiverComponent::AddStat(FGameplayTag Key, float Delta)
{
	const float OldValue = GetStat(Key);
	const float NewValue = OldValue + Delta;
	CurrentStats.Add(Key, NewValue);

	if (!FMath::IsNearlyEqual(OldValue, NewValue))
	{
		OnStatChanged.Broadcast(Key, OldValue, NewValue);
	}

	return NewValue;
}

TMap<FGameplayTag, float> URogueliteStatReceiverComponent::GetAllStats() const
{
	return CurrentStats;
}

/*~ Action Query ~*/

bool URogueliteStatReceiverComponent::HasReceivedAction(URogueliteActionData* Action) const
{
	return IsValid(Action) && ReceivedActions.Contains(Action);
}

int32 URogueliteStatReceiverComponent::GetReceivedActionStacks(URogueliteActionData* Action) const
{
	if (const int32* Stacks = ReceivedActions.Find(Action))
	{
		return *Stacks;
	}
	return 0;
}

TArray<URogueliteActionData*> URogueliteStatReceiverComponent::GetAllReceivedActions() const
{
	TArray<URogueliteActionData*> Result;
	ReceivedActions.GetKeys(Result);
	return Result;
}

/*~ Manual Sync ~*/

void URogueliteStatReceiverComponent::SyncExistingActions()
{
	URogueliteSubsystem* Subsystem = CachedSubsystem.Get();
	if (!IsValid(Subsystem))
	{
		return;
	}

	// 이미 획득된 Action 중 대상에 해당하는 것들 적용
	for (URogueliteActionData* Action : Subsystem->GetAllAcquired())
	{
		// 이미 수신 중인 Action은 스킵
		if (ReceivedActions.Contains(Action))
		{
			continue;
		}

		if (ShouldReceiveAction(Action))
		{
			const int32 Stacks = Subsystem->GetActionStacks(Action);

			// bAutoApplyToReceiver가 true일 때만 스탯 자동 적용
			if (Action->bAutoApplyToReceiver)
			{
				ApplyActionEffects(Action, Stacks);
			}

			ReceivedActions.Add(Action, Stacks);
			OnActionAcquired.Broadcast(Action, 0, Stacks);
		}
	}
}

void URogueliteStatReceiverComponent::ResetStats()
{
	// 모든 로컬 스탯을 DefaultStats로 초기화
	for (const auto& Pair : CurrentStats)
	{
		const float OldValue = Pair.Value;
		const float NewValue = DefaultStats.Contains(Pair.Key) ? DefaultStats[Pair.Key] : 0.f;

		if (!FMath::IsNearlyEqual(OldValue, NewValue))
		{
			OnStatChanged.Broadcast(Pair.Key, OldValue, NewValue);
		}
	}

	CurrentStats.Empty();
	for (const auto& Pair : DefaultStats)
	{
		CurrentStats.Add(Pair.Key, Pair.Value);
	}

	ReceivedActions.Empty();
}

/*~ Protected ~*/

bool URogueliteStatReceiverComponent::ShouldReceiveAction(URogueliteActionData* Action) const
{
	if (!IsValid(Action))
	{
		return false;
	}

	// TargetActionTags가 비어있으면  Action 수신 X
	if (TargetActionTags.IsEmpty())
	{
		return false;
	}

	// 태그 매칭 확인
	if (bRequireAllTags)
	{
		return Action->ActionTags.HasAll(TargetActionTags);
	}
	else
	{
		return Action->ActionTags.HasAny(TargetActionTags);
	}
}

void URogueliteStatReceiverComponent::ApplyActionEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	// 스택 수만큼 Values 적용
	for (int32 i = 0; i < Stacks; ++i)
	{
		for (const FRogueliteValueEntry& Entry : Action->Values)
		{
			const float OldValue = GetStat(Entry.Key);
			float NewValue = OldValue;

			switch (Entry.ApplyMode)
			{
			case ERogueliteApplyMode::Add:
				NewValue = OldValue + Entry.Value;
				break;
			case ERogueliteApplyMode::Multiply:
				NewValue = OldValue * Entry.Value;
				break;
			case ERogueliteApplyMode::Set:
				NewValue = Entry.Value;
				break;
			case ERogueliteApplyMode::Max:
				NewValue = FMath::Max(OldValue, Entry.Value);
				break;
			case ERogueliteApplyMode::Min:
				NewValue = FMath::Min(OldValue, Entry.Value);
				break;
			}

			CurrentStats.Add(Entry.Key, NewValue);

			if (!FMath::IsNearlyEqual(OldValue, NewValue))
			{
				OnStatChanged.Broadcast(Entry.Key, OldValue, NewValue);
			}
		}
	}
}

void URogueliteStatReceiverComponent::RemoveActionEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	// 스택 수만큼 Values 역적용 (Add만 역산 가능)
	for (int32 i = 0; i < Stacks; ++i)
	{
		for (const FRogueliteValueEntry& Entry : Action->Values)
		{
			// 현재 Add 모드만 역산 가능, TODO: 재계산 시스템 구현
			if (Entry.ApplyMode == ERogueliteApplyMode::Add)
			{
				const float OldValue = GetStat(Entry.Key);
				const float NewValue = OldValue - Entry.Value;
				CurrentStats.Add(Entry.Key, NewValue);

				if (!FMath::IsNearlyEqual(OldValue, NewValue))
				{
					OnStatChanged.Broadcast(Entry.Key, OldValue, NewValue);
				}
			}
		}
	}
}

/*~ Event Handlers ~*/

void URogueliteStatReceiverComponent::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!ShouldReceiveAction(Action))
	{
		return;
	}

	// bAutoApplyToReceiver가 true일 때만 스탯 자동 적용
	if (Action->bAutoApplyToReceiver)
	{
		const int32 StacksToApply = NewStacks - OldStacks;
		ApplyActionEffects(Action, StacksToApply);
	}

	ReceivedActions.Add(Action, NewStacks);
	OnActionAcquired.Broadcast(Action, OldStacks, NewStacks);
}

void URogueliteStatReceiverComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!ReceivedActions.Contains(Action))
	{
		return;
	}

	// bAutoApplyToReceiver가 true일 때만 스탯 자동 제거
	if (Action->bAutoApplyToReceiver)
	{
		const int32 StacksToRemove = OldStacks - NewStacks;
		RemoveActionEffects(Action, StacksToRemove);
	}

	if (NewStacks <= 0)
	{
		ReceivedActions.Remove(Action);
	}
	else
	{
		ReceivedActions.Add(Action, NewStacks);
	}

	OnActionRemoved.Broadcast(Action, OldStacks, NewStacks);
}

void URogueliteStatReceiverComponent::HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!ShouldReceiveAction(Action))
	{
		return;
	}

	// 이미 수신 중인 Action의 스택 변경
	if (ReceivedActions.Contains(Action))
	{
		// bAutoApplyToReceiver가 true일 때만 스탯 자동 적용/제거
		if (Action->bAutoApplyToReceiver)
		{
			if (NewStacks > OldStacks)
			{
				const int32 StacksToApply = NewStacks - OldStacks;
				ApplyActionEffects(Action, StacksToApply);
			}
			else if (NewStacks < OldStacks)
			{
				const int32 StacksToRemove = OldStacks - NewStacks;
				RemoveActionEffects(Action, StacksToRemove);
			}
		}

		ReceivedActions.Add(Action, NewStacks);

		// 스택 증가/감소에 따라 다른 이벤트 발생
		if (NewStacks > OldStacks)
		{
			OnActionAcquired.Broadcast(Action, OldStacks, NewStacks);
		}
		else if (NewStacks < OldStacks)
		{
			OnActionRemoved.Broadcast(Action, OldStacks, NewStacks);
		}
	}
}

void URogueliteStatReceiverComponent::HandleRunEnded(bool bCompleted)
{
	// 런 종료 시 스탯 초기화
	ResetStats();
}
