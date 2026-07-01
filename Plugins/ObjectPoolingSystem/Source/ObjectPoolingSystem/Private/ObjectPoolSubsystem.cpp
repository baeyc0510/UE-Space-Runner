// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPoolSubsystem.h"
#include "PoolableInterface.h"
#include "ObjectPoolDeveloperSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectPool, Log, All);

/*~ UWorldSubsystem Interface ~*/

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UObjectPoolSubsystem::Deinitialize()
{
	ReleaseAllPools();
	Super::Deinitialize();
}

bool UObjectPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 게임 월드에서만 생성
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

/*~ Pool Configuration ~*/

void UObjectPoolSubsystem::RegisterPoolSettings(TSubclassOf<AActor> ActorClass, const FPoolSettings& Settings)
{
	if (!IsValid(ActorClass))
	{
		return;
	}

	// 설정 등록 (자식 클래스에도 적용됨)
	RegisteredSettings.Add(ActorClass, Settings);

	// 이미 해당 클래스의 풀이 존재하면 설정 업데이트
	if (FPoolData* PoolDataPtr = Pools.Find(ActorClass))
	{
		PoolDataPtr->Settings = Settings;
		SetupAutoReleaseTimer(ActorClass, *PoolDataPtr);
	}
}

void UObjectPoolSubsystem::PreWarmPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!IsValid(ActorClass) || Count <= 0)
	{
		return;
	}

	// 인터페이스 구현 확인
	if (!ActorClass->ImplementsInterface(UPoolableInterface::StaticClass()))
	{
		UE_LOG(LogObjectPool, Warning, TEXT("PreWarm failed: [%s] does not implement IPoolableInterface"), *ActorClass->GetName());
		return;
	}

	FPoolData& PoolData = GetOrCreatePoolData(ActorClass);

	// MaxPoolSize 제한 확인
	const int32 CurrentTotal = PoolData.AvailableActors.Num() + PoolData.ActiveActors.Num();
	const int32 MaxToSpawn = (PoolData.Settings.MaxPoolSize > 0)
		? FMath::Min(Count, PoolData.Settings.MaxPoolSize - CurrentTotal)
		: Count;

	for (int32 i = 0; i < MaxToSpawn; ++i)
	{
		AActor* NewActor = SpawnAndRegisterActor(ActorClass, FTransform::Identity);
		if (IsValid(NewActor))
		{
			DeactivateActor(NewActor);
			PoolData.AvailableActors.Add(NewActor);
			PoolData.ReturnTimestamps.Add(NewActor, FPlatformTime::Seconds());
		}
	}

	// 자동 해제 타이머 설정
	SetupAutoReleaseTimer(ActorClass, PoolData);
}

/*~ Pool Operations ~*/

AActor* UObjectPoolSubsystem::SpawnFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	if (!IsValid(ActorClass))
	{
		return nullptr;
	}

	// 인터페이스 구현 확인
	if (!ActorClass->ImplementsInterface(UPoolableInterface::StaticClass()))
	{
		UE_LOG(LogObjectPool, Warning, TEXT("AcquireFromPool failed: [%s] does not implement IPoolableInterface"), *ActorClass->GetName());
		return nullptr;
	}

	FPoolData& PoolData = GetOrCreatePoolData(ActorClass);

	// 사용 가능한 액터가 있는 경우
	while (PoolData.AvailableActors.Num() > 0)
	{
		AActor* Actor = PoolData.AvailableActors.Pop();
		PoolData.ReturnTimestamps.Remove(Actor);

		// 유효성 체크 (외부에서 파괴된 경우 스킵)
		if (!IsValid(Actor))
		{
			continue;
		}

		// 활성화 시도 (충돌 처리 포함)
		if (ActivateActor(Actor, SpawnTransform, CollisionHandling))
		{
			PoolData.ActiveActors.Add(Actor);
			return Actor;
		}

		// 활성화 실패 - 다시 풀로 반환
		PoolData.AvailableActors.Add(Actor);
		PoolData.ReturnTimestamps.Add(Actor, FPlatformTime::Seconds());
		return nullptr;
	}

	// MaxPoolSize 제한 확인
	const int32 CurrentTotal = PoolData.ActiveActors.Num();
	if (PoolData.Settings.MaxPoolSize > 0 && CurrentTotal >= PoolData.Settings.MaxPoolSize)
	{
		return nullptr;
	}

	// 새로 스폰
	AActor* NewActor = SpawnAndRegisterActor(ActorClass, SpawnTransform);
	if (IsValid(NewActor))
	{
		// 활성화 시도 (충돌 처리 포함)
		if (ActivateActor(NewActor, SpawnTransform, CollisionHandling))
		{
			PoolData.ActiveActors.Add(NewActor);
			return NewActor;
		}

		// 활성화 실패 - 풀에 비활성 상태로 추가
		DeactivateActor(NewActor);
		PoolData.AvailableActors.Add(NewActor);
		PoolData.ReturnTimestamps.Add(NewActor, FPlatformTime::Seconds());
		return nullptr;
	}

	return nullptr;
}

bool UObjectPoolSubsystem::ReturnToPool(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	// IPoolableInterface 구현 확인
	if (!Actor->Implements<UPoolableInterface>())
	{
		return false;
	}

	// 액터의 클래스로 풀 찾기
	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	FPoolData* PoolDataPtr = Pools.Find(ActorClass);

	if (!PoolDataPtr)
	{
		return false;
	}

	// 이미 Available에 있는지 확인 (중복 반환 방지)
	if (PoolDataPtr->AvailableActors.Contains(Actor))
	{
		return false;
	}

	// ActiveActors에서 제거
	PoolDataPtr->ActiveActors.Remove(Actor);

	// 비활성화 처리
	DeactivateActor(Actor);

	// AvailableActors로 이동
	PoolDataPtr->AvailableActors.Add(Actor);
	PoolDataPtr->ReturnTimestamps.Add(Actor, FPlatformTime::Seconds());

	return true;
}

/*~ Pool Release ~*/

void UObjectPoolSubsystem::ReleasePool(TSubclassOf<AActor> ActorClass)
{
	if (!IsValid(ActorClass))
	{
		return;
	}

	FPoolData* PoolDataPtr = Pools.Find(ActorClass);
	if (!PoolDataPtr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 타이머 정리
	World->GetTimerManager().ClearTimer(PoolDataPtr->AutoReleaseTimerHandle);

	// Available 액터들 파괴
	for (AActor* Actor : PoolDataPtr->AvailableActors)
	{
		if (IsValid(Actor))
		{
			Actor->OnDestroyed.RemoveDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);
			Actor->Destroy();
		}
	}

	// Active 액터들 파괴
	for (AActor* Actor : PoolDataPtr->ActiveActors)
	{
		if (IsValid(Actor))
		{
			Actor->OnDestroyed.RemoveDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);
			Actor->Destroy();
		}
	}

	// 풀 데이터 제거
	Pools.Remove(ActorClass);
}

void UObjectPoolSubsystem::ReleaseAllPools()
{
	UWorld* World = GetWorld();

	for (auto& Pair : Pools)
	{
		FPoolData& PoolData = Pair.Value;

		// 타이머 정리
		if (IsValid(World))
		{
			World->GetTimerManager().ClearTimer(PoolData.AutoReleaseTimerHandle);
		}

		// Available 액터들 파괴
		for (AActor* Actor : PoolData.AvailableActors)
		{
			if (IsValid(Actor))
			{
				Actor->OnDestroyed.RemoveDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);
				Actor->Destroy();
			}
		}

		// Active 액터들 파괴
		for (AActor* Actor : PoolData.ActiveActors)
		{
			if (IsValid(Actor))
			{
				Actor->OnDestroyed.RemoveDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);
				Actor->Destroy();
			}
		}
	}

	Pools.Empty();
}

/*~ Internal Helpers ~*/

AActor* UObjectPoolSubsystem::SpawnAndRegisterActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
	if (IsValid(NewActor))
	{
		// OnDestroyed 델리게이트 바인딩
		NewActor->OnDestroyed.AddDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);

		if (NewActor->Implements<UPoolableInterface>())
		{
			IPoolableInterface::Execute_OnRegisteredToPool(NewActor);
		}
	}

	return NewActor;
}

bool UObjectPoolSubsystem::ActivateActor(AActor* Actor, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandling)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	// 설정 및 활성화
	Actor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(Actor->PrimaryActorTick.bStartWithTickEnabled);
	
	// 충돌 처리 (엔진 Actor.cpp 로직 기반)
	switch (CollisionHandling)
	{
	case ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn:
		{
			FVector AdjustedLocation = Actor->GetActorLocation();
			FRotator AdjustedRotation = Actor->GetActorRotation();
			if (World->FindTeleportSpot(Actor, AdjustedLocation, AdjustedRotation))
			{
				Actor->SetActorLocationAndRotation(AdjustedLocation, AdjustedRotation, false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
		break;

	case ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding:
		{
			FVector AdjustedLocation = Actor->GetActorLocation();
			FRotator AdjustedRotation = Actor->GetActorRotation();
			if (World->FindTeleportSpot(Actor, AdjustedLocation, AdjustedRotation))
			{
				Actor->SetActorLocationAndRotation(AdjustedLocation, AdjustedRotation, false, nullptr, ETeleportType::TeleportPhysics);
			}
			else
			{
				UE_LOG(LogObjectPool, Warning, TEXT("AcquireFromPool failed because of collision at the spawn location [%s] for [%s]"),
					*SpawnTransform.GetLocation().ToString(), *Actor->GetClass()->GetName());
				return false;
			}
		}
		break;

	case ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding:
		{
			FVector AdjustedLocation = Actor->GetActorLocation();
			FRotator AdjustedRotation = Actor->GetActorRotation();
			if (!World->FindTeleportSpot(Actor, AdjustedLocation, AdjustedRotation))
			{
				UE_LOG(LogObjectPool, Warning, TEXT("AcquireFromPool failed because of collision at the spawn location [%s] for [%s]"),
					*SpawnTransform.GetLocation().ToString(), *Actor->GetClass()->GetName());
				return false;
			}
		}
		break;

	case ESpawnActorCollisionHandlingMethod::AlwaysSpawn:
	default:
		break;
	}
	
	// 컴포넌트 활성화
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (IsValid(Component))
		{
			Component->Activate(true);
		}
	}

	// OnPoolableActivate 호출
	if (Actor->Implements<UPoolableInterface>())
	{
		IPoolableInterface::Execute_OnPoolableActivate(Actor);
	}

	return true;
}

void UObjectPoolSubsystem::DeactivateActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	if (Actor->Implements<UPoolableInterface>())
	{
		IPoolableInterface::Execute_OnPoolableDeactivate(Actor);
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
	
	// 컴포넌트 비활성화
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (IsValid(Component))
		{
			Component->Deactivate();
		}
	}
}

void UObjectPoolSubsystem::SetupAutoReleaseTimer(TSubclassOf<AActor> ActorClass, FPoolData& PoolData)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 기존 타이머 정리
	World->GetTimerManager().ClearTimer(PoolData.AutoReleaseTimerHandle);

	// 자동 해제가 비활성화된 경우 타이머 설정하지 않음
	if (!PoolData.Settings.bAutoRelease)
	{
		return;
	}

	// 타이머 설정
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UObjectPoolSubsystem::OnAutoReleaseTimerTick, ActorClass);

	World->GetTimerManager().SetTimer(
		PoolData.AutoReleaseTimerHandle,
		TimerDelegate,
		PoolData.Settings.AutoReleaseCheckInterval,
		true // 반복
	);
}

void UObjectPoolSubsystem::OnAutoReleaseTimerTick(TSubclassOf<AActor> ActorClass)
{
	FPoolData* PoolDataPtr = Pools.Find(ActorClass);
	if (!PoolDataPtr)
	{
		return;
	}

	const double CurrentTime = FPlatformTime::Seconds();
	const float IdleTime = PoolDataPtr->Settings.AutoReleaseIdleTime;
	const int32 MinSize = PoolDataPtr->Settings.MinPoolSize;

	// 뒤에서부터 순회하며 제거 (배열 인덱스 안전)
	for (int32 i = PoolDataPtr->AvailableActors.Num() - 1; i >= 0; --i)
	{
		// 최소 크기 유지
		if (PoolDataPtr->AvailableActors.Num() <= MinSize)
		{
			break;
		}

		AActor* Actor = PoolDataPtr->AvailableActors[i];
		if (!IsValid(Actor))
		{
			PoolDataPtr->AvailableActors.RemoveAt(i);
			continue;
		}

		const double* ReturnTimePtr = PoolDataPtr->ReturnTimestamps.Find(Actor);
		if (ReturnTimePtr && (CurrentTime - *ReturnTimePtr) > IdleTime)
		{
			PoolDataPtr->ReturnTimestamps.Remove(Actor);
			PoolDataPtr->AvailableActors.RemoveAt(i);
			Actor->OnDestroyed.RemoveDynamic(this, &UObjectPoolSubsystem::OnPooledActorDestroyed);
			Actor->Destroy();
		}
	}
}

FPoolData& UObjectPoolSubsystem::GetOrCreatePoolData(TSubclassOf<AActor> ActorClass)
{
	if (FPoolData* ExistingData = Pools.Find(ActorClass))
	{
		return *ExistingData;
	}

	// 새 풀 생성 시 Best Matching 설정 가져오기
	FPoolData NewPoolData;
	NewPoolData.Settings = GetBestMatchingSettings(ActorClass);

	return Pools.Add(ActorClass, MoveTemp(NewPoolData));
}

FPoolSettings UObjectPoolSubsystem::GetBestMatchingSettings(TSubclassOf<AActor> ActorClass) const
{
	if (!IsValid(ActorClass))
	{
		if (const UObjectPoolDeveloperSettings* Settings = UObjectPoolDeveloperSettings::Get())
		{
			return Settings->GlobalDefaultSettings;
		}
		return FPoolSettings();
	}

	// 1. 정확히 일치하는 등록된 설정 탐색
	if (const FPoolSettings* Found = RegisteredSettings.Find(ActorClass))
	{
		return *Found;
	}

	// 2. 가장 가까운 부모 클래스 설정 탐색
	UClass* CurrentClass = ActorClass->GetSuperClass();
	while (CurrentClass && CurrentClass != AActor::StaticClass())
	{
		if (const FPoolSettings* Found = RegisteredSettings.Find(CurrentClass))
		{
			return *Found;
		}
		CurrentClass = CurrentClass->GetSuperClass();
	}

	// 3. 등록된 설정이 없으면 전역 기본값 사용
	if (const UObjectPoolDeveloperSettings* Settings = UObjectPoolDeveloperSettings::Get())
	{
		return Settings->GlobalDefaultSettings;
	}

	return FPoolSettings();
}

void UObjectPoolSubsystem::OnPooledActorDestroyed(AActor* DestroyedActor)
{
	if (!IsValid(DestroyedActor))
	{
		return;
	}

	// 액터의 클래스로 풀 찾기
	TSubclassOf<AActor> ActorClass = DestroyedActor->GetClass();
	FPoolData* PoolDataPtr = Pools.Find(ActorClass);

	if (!PoolDataPtr)
	{
		return;
	}

	// AvailableActors에서 제거
	PoolDataPtr->AvailableActors.Remove(DestroyedActor);
	PoolDataPtr->ReturnTimestamps.Remove(DestroyedActor);

	// ActiveActors에서 제거
	PoolDataPtr->ActiveActors.Remove(DestroyedActor);

	UE_LOG(LogObjectPool, Warning, TEXT("Pooled actor [%s] was destroyed externally and removed from pool"), *DestroyedActor->GetName());
}