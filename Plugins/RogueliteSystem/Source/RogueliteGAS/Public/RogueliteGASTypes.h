#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "RogueliteGASTypes.generated.h"

/*~ Enums ~*/

/**
 * 스택 변경 시 Effect/Ability 처리 방식
 */
UENUM(BlueprintType)
enum class ERogueliteStackScalingMode : uint8
{
	// Effect/Ability Level = 1, 스택 수는 SetByCaller로 전달
	SetByCaller,

	// Effect/Ability Level = 스택 수
	Level
};

/*~ Structs ~*/

/**
 * 트리거 조건 정의
 * TriggerEventTag가 비어있으면 패시브 (획득 즉시 적용)
 */
USTRUCT(BlueprintType)
struct ROGUELITEGAS_API FRogueliteTriggerCondition
{
	GENERATED_BODY()

	// 반응할 GameplayEvent 태그 (비어있으면 패시브)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TriggerEventTag;

	// 발동 확률 (1.0 = 100%)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TriggerChance = 1.0f;

	// 쿨다운 (초, 0 = 없음)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float Cooldown = 0.0f;

	// true: 정확히 일치하는 태그만, false: 하위 태그도 포함
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bExactMatch = true;

	// Payload의 EventMagnitude를 전달받을 SetByCaller 태그 (비어있으면 전달 안함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag EventMagnitudeSetByCallerTag;

	// 트리거 필요 여부
	bool HasTrigger() const { return TriggerEventTag.IsValid(); }
};

/**
 * 트리거 발동 상태 추적
 */
USTRUCT(BlueprintType)
struct ROGUELITEGAS_API FRogueliteTriggerState
{
	GENERATED_BODY()

	// 마지막 트리거 발동 시간
	UPROPERTY(BlueprintReadOnly)
	float LastTriggerTime = -9999.0f;

	// 쿨다운 확인
	bool IsOnCooldown(float CurrentTime, float Cooldown) const
	{
		if (Cooldown <= 0.0f)
		{
			return false;
		}
		return (CurrentTime - LastTriggerTime) < Cooldown;
	}
};

/**
 * Action별 GAS 핸들 관리
 */
USTRUCT(BlueprintType)
struct ROGUELITEGAS_API FRogueliteGASHandles
{
	GENERATED_BODY()

	// 부여된 Ability 핸들들
	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;

	// 적용된 패시브 Effect 핸들들
	UPROPERTY(BlueprintReadOnly)
	TArray<FActiveGameplayEffectHandle> AppliedEffects;

	// 현재 적용된 스택 수
	UPROPERTY(BlueprintReadOnly)
	int32 AppliedStacks = 0;

	// 트리거 상태
	UPROPERTY(BlueprintReadOnly)
	FRogueliteTriggerState TriggerState;

	// 핸들 유효성 검증
	bool HasValidHandles() const
	{
		return GrantedAbilities.Num() > 0 || AppliedEffects.Num() > 0;
	}

	// 핸들 초기화
	void Reset()
	{
		GrantedAbilities.Empty();
		AppliedEffects.Empty();
		AppliedStacks = 0;
		TriggerState = FRogueliteTriggerState();
	}
};