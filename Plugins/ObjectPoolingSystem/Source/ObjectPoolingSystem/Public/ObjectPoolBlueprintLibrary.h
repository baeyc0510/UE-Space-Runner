// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ObjectPoolSubsystem.h"
#include "ObjectPoolBlueprintLibrary.generated.h"

/**
 * 오브젝트 풀 시스템 블루프린트 함수 라이브러리
 */
UCLASS()
class OBJECTPOOLINGSYSTEM_API UObjectPoolBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*~ Pool Configuration ~*/

	// 풀 설정 등록 (자식 클래스에도 적용됨)
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject"))
	static void RegisterPoolSettings(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FPoolSettings& Settings);

	// 미리 스폰하여 풀 워밍업
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject"))
	static void PreWarmPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, int32 Count);

	/*~ Pool Operations ~*/

	// 풀에서 액터 획득
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "ActorClass"))
	static AActor* SpawnFromPool(
		const UObject* WorldContextObject,
		TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	// 풀로 액터 반환
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "Actor"))
	static bool ReturnToPool(const UObject* WorldContextObject, AActor* Actor);

	/*~ Pool Release ~*/

	// 특정 클래스 풀 해제
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject"))
	static void ReleasePool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);

	// 전체 풀 해제
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (WorldContext = "WorldContextObject"))
	static void ReleaseAllPools(const UObject* WorldContextObject);

private:
	// 서브시스템 가져오기
	static UObjectPoolSubsystem* GetPoolSubsystem(const UObject* WorldContextObject);
};
