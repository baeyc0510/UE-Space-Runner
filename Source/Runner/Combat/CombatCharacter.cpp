// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatCharacter.h"
#include "CombatTypes.h"
#include "DamageHandler.h"
#include "HealthComponent.h"

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	// 컴포넌트는 자식 클래스에서 생성
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 컴포넌트 캐싱
	if (!IsValid(DamageHandler))
	{
		DamageHandler = FindComponentByClass<UDamageHandler>();	
	}
	if (!IsValid(HealthComponent))
	{
		HealthComponent = FindComponentByClass<UHealthComponent>();		
	}

	// HealthComponent 사망 이벤트 바인딩
	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeath.AddDynamic(this, &ACombatCharacter::HandleHealthComponentDeath);
	}
}

/*~ Component Getters ~*/

UHealthComponent* ACombatCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

UDamageHandler* ACombatCharacter::GetDamageHandler() const
{
	return DamageHandler;
}

/*~ ICombatInterface ~*/

FGameplayTagContainer ACombatCharacter::GetStatTags() const
{
	return CombatContextStatTags;
}

float ACombatCharacter::GetCombatStat(FGameplayTag StatTag) const
{
	// 기본 구현: 0 반환 (자식 클래스에서 override)
	return 0.0f;
}

void ACombatCharacter::ApplyDamage_Implementation(const FDamageContext& InDamageContext)
{
	OnDamageReceived(InDamageContext);
}

void ACombatCharacter::OnDamageReceived(const FDamageContext& InDamageContext)
{
	if (bIsDead)
	{
		return;
	}

	if (!IsValid(DamageHandler) || !IsValid(HealthComponent))
	{
		return;
	}

	// 데미지 계산
	FDamageResult DamageResult;
	DamageHandler->OnDamageReceived(InDamageContext, DamageResult);
	
	// 데미지 적용 이벤트 (피격자)
	OnDamageApplied(DamageResult);
	
	// 체력 감소
	HealthComponent->TakeDamage(DamageResult.Damage);
	
	// 데미지 가함 이벤트 (공격자)
	if (AActor* InstigatorActor = InDamageContext.Instigator)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(InstigatorActor))
		{
			CombatInterface->OnDamageDealt(DamageResult);
		}
	}
}

void ACombatCharacter::OnDamageApplied(const FDamageResult& InDamageResult)
{
	if (IsValid(DamageHandler))
	{
		DamageHandler->OnDamageApplied(InDamageResult);
	}
}

void ACombatCharacter::OnDamageDealt(const FDamageResult& InDamageResult)
{
	// 기본 구현: 아무 동작 없음 (자식 클래스에서 override)
}

void ACombatCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	OnDie();
}

bool ACombatCharacter::IsDead()
{
	return bIsDead;
}

/*~ ACombatCharacter Interface ~*/

void ACombatCharacter::OnDie()
{
	K2_OnDie();
}

void ACombatCharacter::K2_OnDie_Implementation()
{
	// BP에서 오버라이드
}

void ACombatCharacter::HandleHealthComponentDeath()
{
	Die();
}
