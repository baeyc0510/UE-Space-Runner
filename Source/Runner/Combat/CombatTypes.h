// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "CombatTypes.generated.h"

/*~ GameplayTags - Data (메타 태그) ~*/

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Critical);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Cooldown);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_BaseDamage);

/*~ GameplayTags - Stat (고유 스탯) ~*/

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_MaxHealth);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_CurrentHealth);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_Defense);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_DamageBonus);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_CriticalChance);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_CriticalBonus);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_LifeSteal);

/*~ Combat Context ~*/

/**
 * 데미지 적용 컨텍스트
 */
USTRUCT(BlueprintType)
struct FDamageContext
{
	GENERATED_BODY()

	/*~ 공격자 스탯 (BaseDamage, CritMult, ArmorPen 등) ~*/
	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, float> SourceValues;

	/*~ 방어자 스탯 (Defense, MaxHealth 등) ~*/
	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, float> TargetValues;

	/*~ 메타 태그 목록 ~*/
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer Tags;

	/*~ 데미지 유발자 (캐릭터) ~*/
	UPROPERTY(BlueprintReadWrite)
	AActor* Instigator = nullptr;

	/*~ 데미지 소스 (캐릭터 or 화살 등) ~*/
	UPROPERTY(BlueprintReadWrite)
	AActor* Source = nullptr;

	/*~ 데미지 타겟 ~*/
	UPROPERTY(BlueprintReadWrite)
	AActor* Target = nullptr;
};

/** 
 * 데미지 처리 결과
 */
USTRUCT(BlueprintType)
struct FDamageResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float Damage;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer Tags;
};