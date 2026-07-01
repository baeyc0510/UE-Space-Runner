#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "RogueliteGASTypes.h"
#include "RogueliteAbilityHandlerComponent.generated.h"

class UAbilitySystemComponent;
class URogueliteSubsystem;
class URogueliteActionData;
class URogueliteGASActionData;

// 트리거 발동 시 브로드캐스트되는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRogueliteOnTriggerAction,URogueliteActionData*, Action,FGameplayTag, EventTag,const FGameplayEventData&, Payload);


/**
 * Roguelite ActionData와 GAS를 연결하는 핸들러 컴포넌트.
 * - 패시브 Action: 획득 즉시 Ability 부여 / Effect 적용
 * - 트리거 Action: ASC 이벤트 발생 시 Ability Activate / Effect Apply
 */
UCLASS(ClassGroup = (Roguelite), meta = (BlueprintSpawnableComponent))
class ROGUELITEGAS_API URogueliteAbilityHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URogueliteAbilityHandlerComponent();

	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*~ URogueliteAbilityHandlerComponent Interface ~*/

	// ASC 수동 설정 (자동 검색 실패 시 사용)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|GAS")
	void SetAbilitySystemComponent(UAbilitySystemComponent* InASC);

	// 현재 ASC 반환
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	// 기존 획득 Action들 동기화 (런 중간에 컴포넌트 추가 시)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|GAS")
	void SyncExistingActions();

	// 모든 GAS 핸들 정리
	UFUNCTION(BlueprintCallable, Category = "Roguelite|GAS")
	void ClearAllHandles();

	// Action의 핸들 정보 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|GAS")
	bool GetActionHandles(URogueliteActionData* Action, FRogueliteGASHandles& OutHandles) const;

protected:
	/*~ Subsystem 이벤트 핸들러 ~*/

	UFUNCTION()
	void HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	UFUNCTION()
	void HandleRunEnded(bool bCompleted);

	/*~ 내부 헬퍼 ~*/

	// 필터 통과 여부 확인
	bool PassesFilter(URogueliteActionData* Action) const;

	// ASC 자동 검색
	void FindAbilitySystemComponent();

	// Subsystem 바인딩
	void BindToSubsystem();
	void UnbindFromSubsystem();

	// 이벤트 구독 관리
	void SubscribeActionToEvent(URogueliteGASActionData* Action);
	void UnsubscribeActionFromEvent(URogueliteGASActionData* Action);
	void UnsubscribeFromAllEvents();

	// Ability 부여/해제
	void GrantAbilities(URogueliteGASActionData* Action, int32 Level, FRogueliteGASHandles& OutHandles);
	void ClearAbilities(FRogueliteGASHandles& Handles);

	// Effect 적용/제거 (Payload가 nullptr이면 패시브 처리)
	void ApplyEffectsInternal(URogueliteGASActionData* Action, int32 Stacks, const FGameplayEventData* Payload, FRogueliteGASHandles& OutHandles);
	void RemoveEffects(FRogueliteGASHandles& Handles);

	// SetByCaller 값 적용
	void ApplySetByCallerValues(URogueliteGASActionData* Action, FGameplayEffectSpecHandle& SpecHandle, int32 Stacks, const FGameplayEventData* Payload);

	// 스택 변경 처리 (패시브 전용)
	void RefreshForStackChange(URogueliteGASActionData* Action, int32 NewStacks, FRogueliteGASHandles& Handles);

	// Action에 대한 GAS 설정 (Ability 부여, Effect 적용 또는 이벤트 구독)
	void SetupActionGAS(URogueliteGASActionData* Action, int32 Stacks, FRogueliteGASHandles& Handles);

	// 트리거 처리
	void ProcessTrigger(URogueliteGASActionData* Action, const FGameplayEventData* Payload, FRogueliteGASHandles& Handles);

	// 이벤트 구독 해제 헬퍼
	void RemoveEventSubscription(URogueliteGASActionData* Action, const FDelegateHandle& Handle);

	// Effect 제거 콜백 (핸들 자동 정리)
	void HandleGameplayEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo);

public:
	/*~ 델리게이트 ~*/

	// 트리거 발동 시 브로드캐스트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteOnTriggerAction OnTriggerAction;

	/*~ 설정 ~*/

	// 이 태그를 가진 Action만 처리 (비어있으면 모든 GASActionData 처리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roguelite|Filter")
	FGameplayTagContainer TargetActionTags;

	// 모든 태그 필요 여부 (true: 모두 필요, false: 하나만 있어도 됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roguelite|Filter")
	bool bRequireAllTags = false;

private:
	/*~ 캐시 ~*/

	// ASC 약한 참조
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// Subsystem 약한 참조
	UPROPERTY()
	TWeakObjectPtr<URogueliteSubsystem> CachedSubsystem;

	/*~ Action 관리 ~*/

	// Action : GAS 핸들 매핑
	UPROPERTY()
	TMap<URogueliteGASActionData*, FRogueliteGASHandles> ActionHandleMap;

	UPROPERTY()
	TArray<URogueliteGASActionData*> AcquiredActions;
	
	/*~ 이벤트 구독 ~*/
	// Action → 구독 핸들
	TMap<URogueliteGASActionData*, FDelegateHandle> GameplayEventSubscriptions;

	/*~ 상태 ~*/
	// Subsystem 바인딩 상태
	bool bIsBoundToSubsystem = false;
};
