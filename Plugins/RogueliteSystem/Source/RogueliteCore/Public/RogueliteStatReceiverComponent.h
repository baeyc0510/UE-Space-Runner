#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteStatReceiverComponent.generated.h"

class URogueliteSubsystem;
class URogueliteActionData;

/**
 * 로그라이트 스탯 수신 컴포넌트.
 * 펫, 터렛, 소환수 등 플레이어 외의 Actor가 Action 효과를 받을 때 활용.
 * 특정 태그를 가진 Action만 필터링하여 적용.
 */
UCLASS(ClassGroup=(Roguelite), meta=(BlueprintSpawnableComponent, DisplayName = "RogueliteStatReceiver"))
class ROGUELITECORE_API URogueliteStatReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URogueliteStatReceiverComponent();

	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*~ Stat Access ~*/

	// 스탯 값 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Stats")
	float GetStat(FGameplayTag Key, float DefaultValue = 0.f) const;

	// 스탯 값 설정
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Stats")
	void SetStat(FGameplayTag Key, float Value);

	// 스탯 값 더하기
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Stats")
	float AddStat(FGameplayTag Key, float Delta);

	// 모든 스탯 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Stats")
	TMap<FGameplayTag, float> GetAllStats() const;

	/*~ Action Query ~*/

	// 수신한 Action 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|Actions")
	bool HasReceivedAction(URogueliteActionData* Action) const;

	// 수신한 Action의 스택 수
	UFUNCTION(BlueprintPure, Category = "Roguelite|Actions")
	int32 GetReceivedActionStacks(URogueliteActionData* Action) const;

	// 모든 수신한 Action 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Actions")
	TArray<URogueliteActionData*> GetAllReceivedActions() const;

	/*~ Manual Sync ~*/

	// 기존 획득 Action 수동 동기화 (BeginPlay 후 태그 변경 시 사용)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Sync")
	void SyncExistingActions();

	// 스탯 초기화
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Sync")
	void ResetStats();

public:
	/*~ Events ~*/

	// 로컬 스탯 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteValueChangedSignature OnStatChanged;

	// Action 수신 이벤트 (획득/스택 증가)
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteActionChangedSignature OnActionAcquired;

	// Action 손실 이벤트 (제거/스택 감소)
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteActionChangedSignature OnActionRemoved;

protected:
	// Action이 수신 대상인지 확인
	bool ShouldReceiveAction(URogueliteActionData* Action) const;

	// Action 효과 적용
	void ApplyActionEffects(URogueliteActionData* Action, int32 Stacks);

	// Action 효과 제거
	void RemoveActionEffects(URogueliteActionData* Action, int32 Stacks);

	/*~ Event Handlers ~*/

	// Subsystem Action 획득 이벤트 핸들러
	UFUNCTION()
	void HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	// Subsystem Action 제거 이벤트 핸들러
	UFUNCTION()
	void HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	// Subsystem 스택 변경 이벤트 핸들러
	UFUNCTION()
	void HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);

	// 런 종료 이벤트 핸들러
	UFUNCTION()
	void HandleRunEnded(bool bCompleted);

public:
	/*~ Configuration ~*/

	// 수신할 Action의 태그 필터 (이 태그를 가진 Action만 수신)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roguelite|Config")
	FGameplayTagContainer TargetActionTags;

	// 태그 매칭 방식: true = 모든 태그 필요, false = 하나라도 있으면 수신
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roguelite|Config")
	bool bRequireAllTags = false;

	// 초기 스탯 값 (BeginPlay 시 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roguelite|Config")
	TMap<FGameplayTag, float> DefaultStats;
	
private:
	// 로컬 스탯 저장소
	UPROPERTY()
	TMap<FGameplayTag, float> CurrentStats;

	// 수신한 Action 정보
	UPROPERTY()
	TMap<URogueliteActionData*, int32> ReceivedActions;

	// 캐싱된 Subsystem 참조
	UPROPERTY()
	TWeakObjectPtr<URogueliteSubsystem> CachedSubsystem;
};
