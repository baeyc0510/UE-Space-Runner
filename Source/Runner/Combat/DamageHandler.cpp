// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageHandler.h"
#include "CombatTypes.h"

UDamageHandler::UDamageHandler()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDamageHandler::BeginPlay()
{
	Super::BeginPlay();
}

void UDamageHandler::OnDamageReceived(const FDamageContext& InDamageContext, FDamageResult& OutDamageResult)
{
	// 데미지 계산
	OutDamageResult.Damage = CalculateDamage(InDamageContext);

	// 컨텍스트 태그 복사 (크리티컬 등)
	OutDamageResult.Tags = InDamageContext.Tags;
}

void UDamageHandler::OnDamageApplied_Implementation(const FDamageResult& InDamageResult)
{
	// BP에서 오버라이드하여 UI 처리
}

float UDamageHandler::CalculateDamage(const FDamageContext& InDamageContext) const
{
	/*~ Source (공격자) ~*/

	// 기본 데미지 (무기/펫에서 설정)
	const float BaseDamage = InDamageContext.SourceValues.FindRef(TAG_Data_BaseDamage);

	// 데미지 보너스% (캐릭터 스탯)
	const float DamageBonus = InDamageContext.SourceValues.FindRef(TAG_Stat_DamageBonus);

	// 크리티컬 배율 (TAG_Data_Critical 태그가 있을 때만 적용)
	float CritMultiplier = 1.0f;
	if (InDamageContext.Tags.HasTag(TAG_Data_Critical))
	{
		const float* ContextCritBonus = InDamageContext.SourceValues.Find(TAG_Stat_CriticalBonus);
		CritMultiplier = ContextCritBonus ? (1 + *ContextCritBonus) : 1.0f;
	}

	/*~ Target (방어자) ~*/

	// 방어력 (플레이어만 보유)
	const float Defense = InDamageContext.TargetValues.FindRef(TAG_Stat_Defense);

	/*~ 최종 데미지 계산 ~*/

	// FinalDamage = BaseDamage * (1 + DamageBonus) * CritMultiplier - Defense
	float FinalDamage = BaseDamage * (1.0f + DamageBonus) * CritMultiplier - Defense;

	// 최소 1 데미지 보장
	return FMath::Max(FinalDamage, 1.0f);
}
