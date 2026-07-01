// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatBlueprintLibrary.generated.h"

struct FGameplayTag;
struct FDamageContext;

/**
 * Combat 관련 유틸리티 함수
 */
UCLASS()
class RUNNER_API UCombatBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Source와 Target의 스탯을 자동으로 수집하여 DamageContext 생성
	 * @param SourceActor 공격자 (ICombatInterface 구현 필요)
	 * @param TargetActor 방어자 (ICombatInterface 구현 필요)
	 * @param Instigator 데미지 유발자 (nullptr이면 SourceActor 사용)
	 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	static FDamageContext CreateDamageContext(
		AActor* SourceActor,
		AActor* TargetActor,
		AActor* Instigator = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void SetDamageContextSourceValue(UPARAM(ref)FDamageContext& OutDamageContext, const FGameplayTag& ValueTag, float Value);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static void SetDamageContextTargetValue(UPARAM(ref)FDamageContext& OutDamageContext, const FGameplayTag& ValueTag, float Value);
};