// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "AbilitySystemComponent.h"
#include "Runner/Combat/CombatTypes.h"
#include "Runner/Combat/HealthComponent.h"

AEnemyBase::AEnemyBase()
{
	// 기본 HealthComp 부착
	CreateHealthComponent<UHealthComponent>();
	
	// Enemy 스탯 태그 초기화 (체력만 보유)
	CombatContextStatTags.AddTag(TAG_Stat_MaxHealth);
	CombatContextStatTags.AddTag(TAG_Stat_CurrentHealth);

	// 기본 스탯 값 설정
	EnemyStats.Add(TAG_Stat_MaxHealth, 100.0f);
	EnemyStats.Add(TAG_Stat_CurrentHealth, 100.0f);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// HealthComponent 초기화 및 이벤트 바인딩
	if (UHealthComponent* HC = GetHealthComponent())
	{
		const float MaxHealth = EnemyStats.FindRef(TAG_Stat_MaxHealth);
		HC->InitializeHealth(MaxHealth);
		HC->OnHealthChanged.AddDynamic(this, &AEnemyBase::HandleHealthChanged);
	}
}

/*~ ICombatInterface ~*/

float AEnemyBase::GetCombatStat(FGameplayTag StatTag) const
{
	return EnemyStats.FindRef(StatTag);
}

/*~ Event Handlers ~*/

void AEnemyBase::HandleHealthChanged(float CurrentHealth, float MaxHealth, float Delta)
{
	EnemyStats.Add(TAG_Stat_CurrentHealth, CurrentHealth);
	EnemyStats.Add(TAG_Stat_MaxHealth, MaxHealth);
}