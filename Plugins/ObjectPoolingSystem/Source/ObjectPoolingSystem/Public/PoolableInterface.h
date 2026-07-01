// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 오브젝트 풀에서 관리되는 액터가 구현해야 하는 인터페이스
 */
class OBJECTPOOLINGSYSTEM_API IPoolableInterface
{
	GENERATED_BODY()

public:
	/*~ Pool Lifecycle ~*/

	// 풀에 최초 등록될 때 1회 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnRegisteredToPool();

	// 풀에서 꺼내어 활성화될 때 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnPoolableActivate();

	// 풀로 반환되어 비활성화될 때 호출
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnPoolableDeactivate();
};
