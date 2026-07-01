#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteSubsystem.generated.h"

class URogueliteActionData;
class URogueliteActionDatabase;
class URoguelitePoolPreset;
class URogueliteQueryFilter;
class IRogueliteEffectHandler;

/*~ Delegates ~*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRogueliteRunStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRogueliteRunEndedSignature, bool, bCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRogueliteQueryCompleteSignature, const FRogueliteQuery&, Query, const TArray<URogueliteActionData*>&, Results);
// 획득 전 체크 델리게이트 (false 반환 시 획득 차단)
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FRoguelitePreAcquireCheckSignature, URogueliteActionData*, Action, const FRogueliteRunState&, RunState);

/**
 * 로그라이트 시스템 핵심 서브시스템.
 * ActionDB 관리, RunState 관리, 쿼리 실행을 담당.
 */
UCLASS(BlueprintType)
class ROGUELITECORE_API URogueliteSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/*~ Static Access ~*/

	// 서브시스템 인스턴스 획득
	UFUNCTION(BlueprintCallable, Category = "Roguelite", meta = (WorldContext = "WorldContextObject"))
	static URogueliteSubsystem* Get(const UObject* WorldContextObject);

	/*~ ActionDB ~*/

	// Database 직접 접근
	UFUNCTION(BlueprintPure, Category = "Roguelite|DB")
	URogueliteActionDatabase* GetDatabase() const;

	// 액션 등록 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	void RegisterAction(URogueliteActionData* Action);

	// 액션 해제 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	void UnregisterAction(URogueliteActionData* Action);

	// PrimaryAssetType으로부터 액션 로드 및 등록 - 동기 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	void LoadAndRegisterActions(FPrimaryAssetType AssetType);

	// PrimaryAssetType으로부터 액션 로드 및 등록 - 비동기 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	void LoadAndRegisterActionsAsync(FPrimaryAssetType AssetType, FRogueliteActionsLoadedSignature OnProgress);

	// 모든 등록된 액션 조회 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	TArray<URogueliteActionData*> GetAllActions() const;

	// 특정 태그를 가진 액션 조회 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	TArray<URogueliteActionData*> GetActionsByTag(FGameplayTag Tag) const;

	// 태그 컨테이너로 액션 조회 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB")
	TArray<URogueliteActionData*> GetActionsByTags(const FGameplayTagContainer& Tags, bool bRequireAll = false) const;

	/*~ Run Management ~*/

	// 런 시작
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run")
	void StartRun();

	// 런 종료
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run")
	void EndRun(bool bCompleted = false);

	// 런 활성 여부
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run")
	bool IsRunActive() const;

	// // RunState 직접 접근
	// UFUNCTION(BlueprintCallable, Category = "Roguelite|Run")
	// FRogueliteRunState& GetRunState() {return RunState;}
	//
	// UFUNCTION(BlueprintCallable, Category = "Roguelite|Run")
	// const FRogueliteRunState& GetRunStateConst() const {return RunState;}

	/*~ Query ~*/

	// 쿼리 실행 (Database에 위임, 현재 RunState 적용)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query")
	TArray<URogueliteActionData*> ExecuteQuery(const FRogueliteQuery& InQuery);

	// 프리셋으로 간편 쿼리 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query")
	TArray<URogueliteActionData*> QuerySimple(URoguelitePoolPreset* Preset, int32 Count = 3);

	// 태그로 간편 쿼리 (Database에 위임)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query")
	TArray<URogueliteActionData*> QueryByTag(FGameplayTag PoolTag, int32 Count = 3);

	/*~ Action Management ~*/

	// 액션 획득
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	bool AcquireAction(URogueliteActionData* Action, int32 StacksToAdd = 1);

	// 액션 획득 시도 (실패 사유 반환)
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	bool TryAcquireAction(URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd = 1);

	// Start Run이후 획득될 Action 추가
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	void AddPendingAction(URogueliteActionData* Action, int32 StacksToAdd = 1);
	
	// 액션 제거
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	bool RemoveAction(URogueliteActionData* Action, int32 StacksToRemove = 1, bool bRemoveAll = false);

	// Action 효과 적용
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	void ApplyActionEffects(URogueliteActionData* Action, int32 Stacks);

	// Aciton 효과 제거
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	void RemoveActionEffects(URogueliteActionData* Action, int32 Stacks);
	
	// 액션 보유 여부
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	bool HasAction(URogueliteActionData* Action) const;

	// 현재 스택 수
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	int32 GetActionStacks(URogueliteActionData* Action) const;

	// 모든 획득 액션
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	TArray<URogueliteActionData*> GetAllAcquired() const;

	// 태그로 획득 액션 필터
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action")
	TArray<URogueliteActionData*> GetAcquiredWithTag(FGameplayTag Tag) const;

	/*~ Tags ~*/

	// 태그 추가
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags")
	void AddTagToSystem(FGameplayTag Tag);

	// 태그 제거
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags")
	void RemoveTagFromSystem(FGameplayTag Tag);

	// 태그 보유 여부
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags")
	bool HasTagInSystem(FGameplayTag Tag) const;

	// 모든 활성 태그
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags")
	FGameplayTagContainer GetAllTags() const;

	/*~ Numeric Data ~*/

	// 수치 설정
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric")
	void SetRunStateValue(FGameplayTag Key, float Value);

	// 수치 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric")
	float GetRunStateValue(FGameplayTag Key, float DefaultValue = 0.f) const;

	// 수치 더하기
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric")
	float AddRunStateValue(FGameplayTag Key, float Delta);

	// 모든 수치 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric")
	TMap<FGameplayTag, float> GetAllRunStateValues() const;

	/*~ Slots ~*/

	// 슬롯에 장착
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots")
	bool EquipActionToSlot(URogueliteActionData* Action, FGameplayTag SlotTag);

	// 슬롯에서 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots")
	void UnequipActionFromSlot(URogueliteActionData* Action, FGameplayTag SlotTag);

	// 슬롯 내용 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots")
	TArray<URogueliteActionData*> GetSlotContents(FGameplayTag SlotTag) const;

	// 슬롯 사용 개수
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots")
	int32 GetSlotCount(FGameplayTag SlotTag) const;

	// 슬롯 풀 여부
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots")
	bool IsSlotFull(FGameplayTag SlotTag, int32 MaxCount) const;

	/*~ Save/Load ~*/

	// 세이브 데이터 생성
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Save")
	FRogueliteRunSaveData CreateRunSaveData() const;

	// 세이브 데이터로 복원
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Save")
	void RestoreRunFromSaveData(const FRogueliteRunSaveData& SaveData);

	/*~ Pre-Acquire Check ~*/

	// 획득 전 체크 등록
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Handlers")
	void RegisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate);

	// 획득 전 체크 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Handlers")
	void UnregisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate);

public:
	/*~ Delegates ~*/

	// 런 시작 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteRunStartedSignature OnRunStarted;

	// 런 종료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteRunEndedSignature OnRunEnded;

	// 액션 획득 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteActionChangedSignature OnActionAcquired;

	// 액션 제거 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteActionChangedSignature OnActionRemoved;

	// 스택 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteActionChangedSignature OnStackChanged;

	// 쿼리 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteQueryCompleteSignature OnQueryComplete;

	// 수치 데이터 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Roguelite|Events")
	FRogueliteValueChangedSignature OnRunStateValueChanged;

private:
	/*~ ActionDB ~*/

	// Action 데이터베이스
	UPROPERTY()
	TObjectPtr<URogueliteActionDatabase> Database;

	// 등록 대기 액션
	UPROPERTY()
	TArray<FRoguelitePendingAcquireInfo> PendingActions;

	/*~ RunState ~*/

	// 현재 런 상태
	UPROPERTY()
	FRogueliteRunState RunState;

	/*~ Pre-Acquire Checks ~*/

	// 획득 전 체크 목록
	TArray<FRoguelitePreAcquireCheckSignature> PreAcquireChecks;
};
