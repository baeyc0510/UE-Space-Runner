// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Abilities/GameplayAbility.h"
#include "RunnerAbility.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class RUNNER_API URunnerAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

public:
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag CooldownSetByCallerTag;
	
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag CooldownCoeffStatTag;
	
	UPROPERTY(BlueprintReadOnly,EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag CooldownTag;
};
