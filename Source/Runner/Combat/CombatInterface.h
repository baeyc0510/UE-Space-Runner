// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

struct FDamageResult;
struct FDamageContext;

UINTERFACE(BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RUNNER_API ICombatInterface
{
	GENERATED_BODY()
	
public:
	/*~ Stat Query ~*/

	// 해당 캐릭터가 제공하는 스탯 태그 목록
	virtual FGameplayTagContainer GetStatTags() const = 0;

	// 특정 스탯 값 조회
	virtual float GetCombatStat(FGameplayTag StatTag) const = 0;

	/*~ Damage ~*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void ApplyDamage(const FDamageContext& InDamageContext);

	// 데미지를 가한 후 Instigator에게 호출됨
	virtual void OnDamageDealt(const FDamageResult& InDamageResult) = 0;

	/*~ Death ~*/
	virtual void Die() = 0;
	virtual bool IsDead() = 0;
};