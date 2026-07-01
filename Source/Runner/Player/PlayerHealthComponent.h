// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runner/Combat/HealthComponent.h"
#include "GameplayTagContainer.h"
#include "PlayerHealthComponent.generated.h"

/**
 * 플레이어 전용 체력 컴포넌트
 */
UCLASS()
class RUNNER_API UPlayerHealthComponent : public UHealthComponent
{
	GENERATED_BODY()

public:
	UPlayerHealthComponent();

	/*~ UHealthComponent Interface ~*/
	virtual void SetCurrentHealth(float NewHealth) override;
	virtual void SetMaxHealth(float NewMaxHealth, bool bAdjustCurrentHealth = true) override;
	virtual void InitializeHealth(float InMaxHealth) override;

	/*~ RunState Sync ~*/

	// RunState에서 체력 동기화
	UFUNCTION(BlueprintCallable, Category = "Health|RunState")
	void SyncFromRunState();

	// RunState로 체력 저장
	UFUNCTION(BlueprintCallable, Category = "Health|RunState")
	void SyncToRunState();

protected:
	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;

	// RunState 값 변경 이벤트 핸들러
	UFUNCTION()
	void HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue);

	// 내부 체력 설정 (RunState 동기화 없음)
	void SetHealthInternal(float NewMaxHealth, float NewCurrentHealth);

protected:
	/*~ Settings ~*/

	// MaxHealth용 RunState 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|RunState")
	FGameplayTag MaxHealthTag;

	// CurrentHealth용 RunState 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health|RunState")
	FGameplayTag CurrentHealthTag;
};
