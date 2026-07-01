// Fill out your copyright notice in the Description page of Project Settings.


#include "RunnerAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RogueliteBlueprintLibrary.h"

void URunnerAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!IsValid(CooldownGameplayEffectClass) || !CooldownSetByCallerTag.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("CooldownEffect and CooldownSetByCallerTag should be valid"));
		return;
	}
	
	float CooldownCoeff = FMath::Max(0.01f, URogueliteBlueprintLibrary::GetRunStateValue(this,CooldownCoeffStatTag,1.0f));
	float Cooldown = FMath::Max(0.f,URogueliteBlueprintLibrary::GetRunStateValue(this,CooldownTag,0.f));
	float FinalCooldown = CooldownCoeff * Cooldown;
	
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
		SpecHandle.Data->SetSetByCallerMagnitude(CooldownSetByCallerTag, FinalCooldown);
	}
	
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo,SpecHandle);
}

const FGameplayTagContainer* URunnerAbility::GetCooldownTags() const
{
	static FGameplayTagContainer CooldownTags;
	CooldownTags.Reset();
	CooldownTags.AddTag(CooldownTag);
	return &CooldownTags;
}