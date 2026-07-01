// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatInterface.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"

class UDamageHandler;
class UHealthComponent;

/**
 * 전투 가능한 캐릭터 기본 클래스 (Player/Enemy 공통)
 * - 데미지 수신/처리 (DamageHandler)
 * - 체력 관리 (HealthComponent)
 * - 사망 처리
 */
UCLASS()
class RUNNER_API ACombatCharacter : public ACharacter, public ICombatInterface
{
	GENERATED_BODY()

public:
	ACombatCharacter();

	/*~ ICombatInterface ~*/
	virtual FGameplayTagContainer GetStatTags() const override;
	virtual float GetCombatStat(FGameplayTag StatTag) const override;
	virtual void ApplyDamage_Implementation(const FDamageContext& InDamageContext) override;
	virtual void OnDamageDealt(const FDamageResult& InDamageResult) override;
	virtual void Die() override;
	virtual bool IsDead() override;
	
	/*~ Component Getters ~*/

	UFUNCTION(BlueprintPure, Category = "Combat")
	UHealthComponent* GetHealthComponent() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UDamageHandler* GetDamageHandler() const;

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

	/*~ ACombatCharacter Interface ~*/
	template<typename HealthCompType>
	HealthCompType* CreateHealthComponent()
	{
		HealthCompType* Comp = CreateDefaultSubobject<HealthCompType>(TEXT("HealthComponent"));
		HealthComponent = Comp;
		return Comp;
	}
	
	// 데미지 처리
	virtual void OnDamageReceived(const FDamageContext& InDamageContext);
	virtual void OnDamageApplied(const FDamageResult& InDamageResult);
	
	// C++ 사망 처리
	virtual void OnDie();

	// BP 사망 이벤트
	UFUNCTION(BlueprintNativeEvent, meta = (DisplayName = "OnDie"))
	void K2_OnDie();

	// HealthComponent 사망 이벤트 핸들러
	UFUNCTION()
	void HandleHealthComponentDeath();

protected:
	/*~ Combat Stats ~*/

	// 이 캐릭터가 제공하는 스탯 태그 목록
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer CombatContextStatTags;

	/*~ State ~*/

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsDead = false;
	
	/*~ Cached Components ~*/
	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "DamangeHandler"))
	TObjectPtr<UDamageHandler> DamageHandler = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "HealthComponent"))
	TObjectPtr<UHealthComponent> HealthComponent = nullptr;
};
