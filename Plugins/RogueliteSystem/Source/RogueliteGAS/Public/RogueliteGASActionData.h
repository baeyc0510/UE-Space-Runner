#pragma once

#include "CoreMinimal.h"
#include "RogueliteActionData.h"
#include "RogueliteGASTypes.h"
#include "RogueliteGASActionData.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * GAS 확장 ActionData
 * Ability/Effect를 정의하고 트리거 조건을 설정
 */
UCLASS(BlueprintType, Blueprintable)
class ROGUELITEGAS_API URogueliteGASActionData : public URogueliteActionData
{
	GENERATED_BODY()

public:
	/*~ GAS 요소 ~*/

	// 부여할 Ability 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	// 적용할 Effect 클래스들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> Effects;

	/*~ 트리거 조건 ~*/

	// 트리거 조건 (TriggerEventTag가 비어있으면 패시브)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Trigger")
	FRogueliteTriggerCondition TriggerCondition;

	/*~ 스케일링 ~*/

	// Values 태그를 SetByCaller 태그로 매핑
	// Key: ActionData의 Value 태그, Value: Effect의 SetByCaller 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Scaling")
	TMap<FGameplayTag, FGameplayTag> ValueToSetByCallerMap;

	// 스택 변경 시 Effect 처리 방식
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Scaling")
	ERogueliteStackScalingMode StackScalingMode = ERogueliteStackScalingMode::SetByCaller;

	// 스택 수를 전달받을 SetByCaller 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Scaling")
	FGameplayTag StacksSetByCallerTag;

public:
	// 패시브 여부 확인
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	bool HasTrigger() const { return TriggerCondition.HasTrigger(); }

	// 트리거 이벤트 태그 반환
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	FGameplayTag GetTriggerEventTag() const { return TriggerCondition.TriggerEventTag; }

	// GAS 요소 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	bool HasGASElements() const { return Abilities.Num() > 0 || Effects.Num() > 0; }
};
