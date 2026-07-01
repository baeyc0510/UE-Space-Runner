// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CombatStatData.generated.h"

class URogueliteActionData;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class RUNNER_API UCombatStatData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Stat)
	TObjectPtr<URogueliteActionData> MinMaxSettingAction;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = Stat)
	TMap<FGameplayTag,float> Stats;
};