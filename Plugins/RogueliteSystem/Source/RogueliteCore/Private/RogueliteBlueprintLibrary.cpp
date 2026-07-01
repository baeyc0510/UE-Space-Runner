#include "RogueliteBlueprintLibrary.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"

/*~ Subsystem Access ~*/

URogueliteSubsystem* URogueliteBlueprintLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	return URogueliteSubsystem::Get(WorldContextObject);
}

/*~ DB ~*/

void URogueliteBlueprintLibrary::RegisterAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RegisterAction(Action);
	}
}

void URogueliteBlueprintLibrary::UnregisterAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->UnregisterAction(Action);
	}
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::GetAllRegisteredActions(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllActions();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::GetActionsByTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetActionsByTag(Tag);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Run ~*/

void URogueliteBlueprintLibrary::StartRun(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->StartRun();
	}
}

void URogueliteBlueprintLibrary::EndRun(const UObject* WorldContextObject, bool bCompleted)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->EndRun(bCompleted);
	}
}

bool URogueliteBlueprintLibrary::IsRunActive(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->IsRunActive();
	}
	return false;
}

// FRogueliteRunState URogueliteBlueprintLibrary::GetRunState(const UObject* WorldContextObject)
// {
// 	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
// 	{
// 		return Subsystem->GetRunStateConst();
// 	}
// 	return FRogueliteRunState();
// }

/*~ Query ~*/

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::QuerySimple(const UObject* WorldContextObject, URoguelitePoolPreset* Preset, int32 Count)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->QuerySimple(Preset, Count);
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::QueryByTag(const UObject* WorldContextObject, FGameplayTag PoolTag, int32 Count)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->QueryByTag(PoolTag, Count);
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::ExecuteQuery(const UObject* WorldContextObject, const FRogueliteQuery& QueryStruct)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->ExecuteQuery(QueryStruct);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Action ~*/

bool URogueliteBlueprintLibrary::AcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToAdd)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->AcquireAction(Action, StacksToAdd);
	}
	return false;
}

bool URogueliteBlueprintLibrary::TryAcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->TryAcquireAction(Action, OutFailReason, StacksToAdd);
	}
	OutFailReason = TEXT("Subsystem not found");
	return false;
}

bool URogueliteBlueprintLibrary::RemoveAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToRemove, bool bRemoveAll)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->RemoveAction(Action, StacksToRemove, bRemoveAll);
	}
	return false;
}

int32 URogueliteBlueprintLibrary::GetActionStacks(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetActionStacks(Action);
	}
	return 0;
}

bool URogueliteBlueprintLibrary::HasAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->HasAction(Action);
	}
	return false;
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::GetAllAcquired(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllAcquired();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::GetAcquiredWithTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAcquiredWithTag(Tag);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Tags ~*/

void URogueliteBlueprintLibrary::AddTagToSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->AddTagToSystem(Tag);
	}
}

void URogueliteBlueprintLibrary::RemoveTagFromSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RemoveTagFromSystem(Tag);
	}
}

bool URogueliteBlueprintLibrary::HasTagInSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->HasTagInSystem(Tag);
	}
	return false;
}

FGameplayTagContainer URogueliteBlueprintLibrary::GetAllTags(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllTags();
	}
	return FGameplayTagContainer();
}

/*~ Numeric ~*/

void URogueliteBlueprintLibrary::SetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Value)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->SetRunStateValue(Key, Value);
	}
}

float URogueliteBlueprintLibrary::GetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float DefaultValue)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetRunStateValue(Key, DefaultValue);
	}
	return DefaultValue;
}

float URogueliteBlueprintLibrary::AddRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Delta)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->AddRunStateValue(Key, Delta);
	}
	return Delta;
}

TMap<FGameplayTag, float> URogueliteBlueprintLibrary::GetAllRunStateValues(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllRunStateValues();
	}
	return TMap<FGameplayTag, float>();
}

/*~ Slots ~*/

bool URogueliteBlueprintLibrary::EquipActionToSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->EquipActionToSlot(Action, SlotTag);
	}
	return false;
}

void URogueliteBlueprintLibrary::UnequipActionFromSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->UnequipActionFromSlot(Action, SlotTag);
	}
}

TArray<URogueliteActionData*> URogueliteBlueprintLibrary::GetSlotContents(const UObject* WorldContextObject, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetSlotContents(SlotTag);
	}
	return TArray<URogueliteActionData*>();
}

int32 URogueliteBlueprintLibrary::GetSlotCount(const UObject* WorldContextObject, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetSlotCount(SlotTag);
	}
	return 0;
}

bool URogueliteBlueprintLibrary::IsSlotFull(const UObject* WorldContextObject, FGameplayTag SlotTag, int32 MaxCount)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->IsSlotFull(SlotTag, MaxCount);
	}
	return false;
}

/*~ Save/Load ~*/

FRogueliteRunSaveData URogueliteBlueprintLibrary::CreateRunSaveData(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->CreateRunSaveData();
	}
	return FRogueliteRunSaveData();
}

void URogueliteBlueprintLibrary::RestoreRunFromSaveData(const UObject* WorldContextObject, const FRogueliteRunSaveData& SaveData)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RestoreRunFromSaveData(SaveData);
	}
}
