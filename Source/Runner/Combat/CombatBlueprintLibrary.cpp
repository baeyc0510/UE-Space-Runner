// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatBlueprintLibrary.h"
#include "CombatInterface.h"
#include "CombatTypes.h"

FDamageContext UCombatBlueprintLibrary::CreateDamageContext(
	AActor* SourceActor,
	AActor* TargetActor,
	AActor* Instigator)
{
	FDamageContext Context;
	Context.Instigator = Instigator ? Instigator : SourceActor;
	Context.Source = SourceActor;
	Context.Target = TargetActor;

	// Instigator 스탯 자동 수집
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Context.Instigator))
	{
		const FGameplayTagContainer SourceTags = CombatInterface->GetStatTags();
		for (const FGameplayTag& Tag : SourceTags)
		{
			const float Value = CombatInterface->GetCombatStat(Tag);
			Context.SourceValues.Add(Tag, Value);
		}
	}

	// Target 스탯 자동 수집
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Context.Target))
	{
		const FGameplayTagContainer TargetTags = CombatInterface->GetStatTags();
		for (const FGameplayTag& Tag : TargetTags)
		{
			const float Value = CombatInterface->GetCombatStat(Tag);
			Context.TargetValues.Add(Tag, Value);
		}
	}

	// 크리티컬 판정
	const float CriticalChance = Context.SourceValues.FindRef(TAG_Stat_CriticalChance);
	if (CriticalChance > 0.0f && FMath::FRand() < CriticalChance)
	{
		Context.Tags.AddTag(TAG_Data_Critical);
	}

	return Context;
}

void UCombatBlueprintLibrary::SetDamageContextSourceValue(FDamageContext& OutDamageContext, const FGameplayTag&  ValueTag, float Value)
{
	if (!ValueTag.IsValid())
	{
		return;
	}
	OutDamageContext.SourceValues.Add(ValueTag, Value);
}

void UCombatBlueprintLibrary::SetDamageContextTargetValue(FDamageContext& OutDamageContext, const FGameplayTag&  ValueTag, float Value)
{
	if (!ValueTag.IsValid())
	{
		return;
	}
	OutDamageContext.TargetValues.Add(ValueTag, Value);
}