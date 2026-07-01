// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPoolBlueprintLibrary.h"
#include "Engine/World.h"

/*~ Pool Configuration ~*/

void UObjectPoolBlueprintLibrary::RegisterPoolSettings(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FPoolSettings& Settings)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		Subsystem->RegisterPoolSettings(ActorClass, Settings);
	}
}

void UObjectPoolBlueprintLibrary::PreWarmPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		Subsystem->PreWarmPool(ActorClass, Count);
	}
}

/*~ Pool Operations ~*/

AActor* UObjectPoolBlueprintLibrary::SpawnFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		return Subsystem->SpawnFromPool(ActorClass, SpawnTransform, CollisionHandling);
	}
	return nullptr;
}

bool UObjectPoolBlueprintLibrary::ReturnToPool(const UObject* WorldContextObject, AActor* Actor)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		return Subsystem->ReturnToPool(Actor);
	}
	return false;
}

/*~ Pool Release ~*/

void UObjectPoolBlueprintLibrary::ReleasePool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		Subsystem->ReleasePool(ActorClass);
	}
}

void UObjectPoolBlueprintLibrary::ReleaseAllPools(const UObject* WorldContextObject)
{
	if (UObjectPoolSubsystem* Subsystem = GetPoolSubsystem(WorldContextObject))
	{
		Subsystem->ReleaseAllPools();
	}
}

/*~ Internal ~*/

UObjectPoolSubsystem* UObjectPoolBlueprintLibrary::GetPoolSubsystem(const UObject* WorldContextObject)
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

	return World->GetSubsystem<UObjectPoolSubsystem>();
}
