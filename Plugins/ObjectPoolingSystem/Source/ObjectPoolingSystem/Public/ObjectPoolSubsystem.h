// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 * 오브젝트 풀 설정 구조체
 */
USTRUCT(BlueprintType)
struct FPoolSettings
{
	GENERATED_BODY()

	// 최대 풀 크기 (0 = 무제한)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
	int32 MaxPoolSize = 0;

	// 자동 해제 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings")
	bool bAutoRelease = false;

	// 이 시간(초) 동안 미사용 시 자동 해제 대상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings", meta = (EditCondition = "bAutoRelease"))
	float AutoReleaseIdleTime = 30.0f;

	// 자동 해제 검사 주기 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings", meta = (EditCondition = "bAutoRelease"))
	float AutoReleaseCheckInterval = 10.0f;

	// 자동 해제 시 유지할 최소 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool Settings", meta = (EditCondition = "bAutoRelease"))
	int32 MinPoolSize = 0;
};

/**
 * 풀 데이터 (내부 사용)
 */
USTRUCT()
struct FPoolData
{
	GENERATED_BODY()

	// 사용 가능한(비활성) 액터들
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableActors;

	// 현재 사용 중인(활성) 액터들
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveActors;

	// 풀 설정
	FPoolSettings Settings;

	// 자동 해제용: 각 Available 액터의 반환 시간
	TMap<TObjectPtr<AActor>, double> ReturnTimestamps;

	// 자동 해제 타이머 핸들
	FTimerHandle AutoReleaseTimerHandle;
};

/**
 * 오브젝트 풀 월드 서브시스템
 * 액터를 재사용하여 스폰/디스트로이 비용을 절감
 */
UCLASS()
class OBJECTPOOLINGSYSTEM_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*~ UWorldSubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/*~ Pool Configuration ~*/

	// 풀 설정 등록 (AcquireFromPool 전에 호출하면 해당 설정 적용)
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void RegisterPoolSettings(TSubclassOf<AActor> ActorClass, const FPoolSettings& Settings);

	// 미리 스폰하여 풀 워밍업
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void PreWarmPool(TSubclassOf<AActor> ActorClass, int32 Count);

	/*~ Pool Operations ~*/

	// 풀에서 액터 획득 (없으면 스폰, MaxPoolSize 초과 또는 충돌 시 nullptr)
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (DeterminesOutputType = "ActorClass"))
	AActor* SpawnFromPool(
		TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform,
		ESpawnActorCollisionHandlingMethod CollisionHandling = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	// 풀로 액터 반환
	UFUNCTION(BlueprintCallable, Category = "Object Pool", meta = (DefaultToSelf = "Actor"))
	bool ReturnToPool(AActor* Actor);

	/*~ Pool Release ~*/

	// 특정 클래스 풀 해제
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReleasePool(TSubclassOf<AActor> ActorClass);

	// 전체 풀 해제
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReleaseAllPools();

private:
	/*~ Internal Helpers ~*/

	// 새 액터를 스폰하고 풀에 등록
	AActor* SpawnAndRegisterActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	// 액터 활성화 처리 (충돌 처리 포함, 실패 시 false 반환)
	bool ActivateActor(AActor* Actor, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandling);

	// 액터 비활성화 처리
	void DeactivateActor(AActor* Actor);

	// 자동 해제 타이머 설정
	void SetupAutoReleaseTimer(TSubclassOf<AActor> ActorClass, FPoolData& PoolData);

	// 자동 해제 타이머 콜백
	void OnAutoReleaseTimerTick(TSubclassOf<AActor> ActorClass);

	// 풀 데이터 가져오기 (없으면 생성)
	FPoolData& GetOrCreatePoolData(TSubclassOf<AActor> ActorClass);

	// 풀링된 액터가 외부에서 파괴될 때 호출
	UFUNCTION()
	void OnPooledActorDestroyed(AActor* DestroyedActor);

	// 등록된 설정 중 가장 적합한 설정 가져오기 (상속 체인 탐색)
	FPoolSettings GetBestMatchingSettings(TSubclassOf<AActor> ActorClass) const;

	/*~ Pool Storage ~*/

	// 클래스별 풀 데이터
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FPoolData> Pools;

	// RegisterPoolSettings로 등록된 설정 (자식 클래스에도 적용)
	TMap<TSubclassOf<AActor>, FPoolSettings> RegisteredSettings;

	// BeginPlay 델리게이트 핸들
	FDelegateHandle BeginPlayDelegateHandle;
};
