// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerHealthComponent.h"
#include "Runner/Combat/CombatTypes.h"
#include "RogueliteBlueprintLibrary.h"
#include "RogueliteSubsystem.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
	// 기본 태그 설정
	MaxHealthTag = TAG_Stat_MaxHealth;
	CurrentHealthTag = TAG_Stat_CurrentHealth;
}

void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// RunState에서 동기화
	SyncFromRunState();

	// RunState 값 변경 이벤트 바인딩
	if (URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this))
	{
		RogueliteSubsystem->OnRunStateValueChanged.AddDynamic(this, &UPlayerHealthComponent::HandleRunStateValueChanged);
	}
}

void UPlayerHealthComponent::HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue)
{
	if (Key == MaxHealthTag || Key == CurrentHealthTag)
	{
		SyncFromRunState();
	}
}

/*~ UHealthComponent Interface ~*/

void UPlayerHealthComponent::SetCurrentHealth(float NewHealth)
{
	// 부모 호출
	Super::SetCurrentHealth(NewHealth);

	// RunState 동기화
	SyncToRunState();
}

void UPlayerHealthComponent::SetMaxHealth(float NewMaxHealth, bool bAdjustCurrentHealth)
{
	// 부모 호출
	Super::SetMaxHealth(NewMaxHealth, bAdjustCurrentHealth);

	// RunState 동기화
	SyncToRunState();
}

void UPlayerHealthComponent::InitializeHealth(float InMaxHealth)
{
	// 부모 호출
	Super::InitializeHealth(InMaxHealth);

	// RunState 동기화
	SyncToRunState();
}

/*~ RunState Sync ~*/

void UPlayerHealthComponent::SyncFromRunState()
{
	// RunState에서 값 가져오기
	const float RunStateMaxHealth = URogueliteBlueprintLibrary::GetRunStateValue(GetOwner(), MaxHealthTag, DefaultMaxHealth);
	const float RunStateCurrentHealth = URogueliteBlueprintLibrary::GetRunStateValue(GetOwner(), CurrentHealthTag, RunStateMaxHealth);

	// 내부 함수로 설정 (SyncToRunState 호출 방지)
	SetHealthInternal(RunStateMaxHealth, RunStateCurrentHealth);
}

void UPlayerHealthComponent::SyncToRunState()
{
	// RunState에 값 저장
	URogueliteBlueprintLibrary::SetRunStateValue(GetOwner(), MaxHealthTag, MaxHealth);
	URogueliteBlueprintLibrary::SetRunStateValue(GetOwner(), CurrentHealthTag, CurrentHealth);
}

void UPlayerHealthComponent::SetHealthInternal(float NewMaxHealth, float NewCurrentHealth)
{
	UHealthComponent::SetMaxHealth(NewMaxHealth, false);
	UHealthComponent::SetCurrentHealth(NewCurrentHealth);
}