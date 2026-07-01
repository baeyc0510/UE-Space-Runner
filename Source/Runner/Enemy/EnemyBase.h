// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Runner/Combat/CombatCharacter.h"
#include "EnemyBase.generated.h"

UCLASS()
class RUNNER_API AEnemyBase : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	/*~ ICombatInterface ~*/
	virtual float GetCombatStat(FGameplayTag StatTag) const override;

protected:
	virtual void BeginPlay() override;

	// HealthComponent 이벤트 핸들러
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta);

protected:
	// Enemy 스탯 (Tag 기반)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Stats")
	TMap<FGameplayTag, float> EnemyStats;
};