// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 체력 변경 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth, float, DeltaHealth);
// 사망 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

/**
 * 체력 관리 컴포넌트
 */
UCLASS(Blueprintable, BlueprintType)
class RUNNER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/*~ Health Getters ~*/

	// 현재 체력 조회
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;

	// 최대 체력 조회
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	// 체력 비율 조회 (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercent() const;

	// 사망 여부
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const;

	/*~ Health Setters ~*/

	// 체력 설정
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void SetCurrentHealth(float NewHealth);

	// 최대 체력 설정
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void SetMaxHealth(float NewMaxHealth, bool bAdjustCurrentHealth = true);

	// 체력 변경 (데미지: 음수, 회복: 양수)
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ModifyHealth(float Delta);

	// 체력 초기화 (최대 체력으로 설정)
	UFUNCTION(BlueprintCallable, Category = "Health")
	virtual void InitializeHealth(float InMaxHealth);

	// 체력 회복
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// 데미지 적용
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Damage);

protected:
	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;

	/*~ Internal ~*/

	// 체력 변경 내부 처리
	virtual void HandleHealthChanged(float OldHealth, float NewHealth, float Delta);

	// 사망 처리
	virtual void HandleDeath();

public:
	/*~ Delegates ~*/

	// 체력 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChangedSignature OnHealthChanged;

	// 사망 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeathSignature OnDeath;

protected:
	/*~ Settings ~*/

	// 기본 최대 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|Settings")
	float DefaultMaxHealth = 100.0f;

	/*~ State ~*/

	// 현재 체력
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 0.0f;

	// 최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float MaxHealth = 0.0f;

	// 사망 여부
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;
};
