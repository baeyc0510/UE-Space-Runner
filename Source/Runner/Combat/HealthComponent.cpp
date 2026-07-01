// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// 기본값으로 초기화
	InitializeHealth(DefaultMaxHealth);
}

/*~ Health Getters ~*/

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
}

bool UHealthComponent::IsDead() const
{
	return bIsDead;
}

/*~ Health Setters ~*/

void UHealthComponent::SetCurrentHealth(float NewHealth)
{
	if (bIsDead)
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	const float Delta = CurrentHealth - OldHealth;

	// 변경 처리
	if (!FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		HandleHealthChanged(OldHealth, CurrentHealth, Delta);
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bAdjustCurrentHealth)
{
	const float OldMaxHealth = MaxHealth;
	MaxHealth = FMath::Max(NewMaxHealth, 0.0f);

	// 최대 체력 증가 시 현재 체력도 비례 조정
	if (bAdjustCurrentHealth && OldMaxHealth > 0.0f)
	{
		const float HealthRatio = CurrentHealth / OldMaxHealth;
		SetCurrentHealth(MaxHealth * HealthRatio);
	}
	else
	{
		// 현재 체력이 새 최대치를 초과하면 조정
		if (CurrentHealth > MaxHealth)
		{
			SetCurrentHealth(MaxHealth);
		}
	}
}

void UHealthComponent::ModifyHealth(float Delta)
{
	SetCurrentHealth(CurrentHealth + Delta);
}

void UHealthComponent::InitializeHealth(float InMaxHealth)
{
	MaxHealth = FMath::Max(InMaxHealth, 0.0f);
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount > 0.0f)
	{
		ModifyHealth(Amount);
	}
}

void UHealthComponent::TakeDamage(float Damage)
{
	if (Damage > 0.0f)
	{
		ModifyHealth(-Damage);
	}
}

/*~ Internal ~*/

void UHealthComponent::HandleHealthChanged(float OldHealth, float NewHealth, float Delta)
{
	// 델리게이트 브로드캐스트
	OnHealthChanged.Broadcast(NewHealth, MaxHealth, Delta);

	// 사망 체크
	if (NewHealth <= 0.0f && OldHealth > 0.0f && !bIsDead)
	{
		HandleDeath();
	}
}

void UHealthComponent::HandleDeath()
{
	bIsDead = true;
	OnDeath.Broadcast();
}


