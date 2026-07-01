#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteBlueprintLibrary.generated.h"

class URogueliteActionData;
class URoguelitePoolPreset;
class URogueliteSubsystem;

/**
 * 로그라이트 시스템 BP 함수 라이브러리.
 * Subsystem API의 간편 접근용.
 */
UCLASS()
class ROGUELITECORE_API URogueliteBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*~ Subsystem Access ~*/

	// 서브시스템 획득
	UFUNCTION(BlueprintCallable, Category = "Roguelite", meta = (WorldContext = "WorldContextObject"))
	static URogueliteSubsystem* GetSubsystem(const UObject* WorldContextObject);

	/*~ DB ~*/

	// 액션 등록
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB", meta = (WorldContext = "WorldContextObject"))
	static void RegisterAction(const UObject* WorldContextObject, URogueliteActionData* Action);

	// 액션 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB", meta = (WorldContext = "WorldContextObject"))
	static void UnregisterAction(const UObject* WorldContextObject, URogueliteActionData* Action);

	// 모든 등록된 액션 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> GetAllRegisteredActions(const UObject* WorldContextObject);

	// 태그로 액션 조회
	UFUNCTION(BlueprintCallable, Category = "Roguelite|DB", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> GetActionsByTag(const UObject* WorldContextObject, FGameplayTag Tag);

	/*~ Run ~*/

	// 런 시작
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run", meta = (WorldContext = "WorldContextObject"))
	static void StartRun(const UObject* WorldContextObject);

	// 런 종료
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run", meta = (WorldContext = "WorldContextObject"))
	static void EndRun(const UObject* WorldContextObject, bool bCompleted = false);

	// 런 활성 여부
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Run", meta = (WorldContext = "WorldContextObject"))
	static bool IsRunActive(const UObject* WorldContextObject);

	// // RunState 직접 접근
	// UFUNCTION(BlueprintCallable, Category = "Roguelite|Run", meta = (WorldContext = "WorldContextObject"))
	// static FRogueliteRunState GetRunState(const UObject* WorldContextObject);

	/*~ Query ~*/

	// 프리셋으로 간편 쿼리
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> QuerySimple(const UObject* WorldContextObject, URoguelitePoolPreset* Preset, int32 Count = 3);

	// 태그로 간편 쿼리
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> QueryByTag(const UObject* WorldContextObject, FGameplayTag PoolTag, int32 Count = 3);

	// 쿼리 실행
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> ExecuteQuery(const UObject* WorldContextObject, const FRogueliteQuery& QueryStruct);

	/*~ Action ~*/

	// 액션 획득
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static bool AcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToAdd = 1);

	// 액션 획득 시도
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static bool TryAcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd = 1);

	// 액션 제거
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static bool RemoveAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToRemove = 1, bool bRemoveAll = false);

	// 스택 수 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static int32 GetActionStacks(const UObject* WorldContextObject, URogueliteActionData* Action);

	// 액션 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static bool HasAction(const UObject* WorldContextObject, URogueliteActionData* Action);

	// 모든 획득 액션
	UFUNCTION(BlueprintPure, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> GetAllAcquired(const UObject* WorldContextObject);

	// 태그로 획득 액션 필터
	UFUNCTION(BlueprintPure, Category = "Roguelite|Action", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> GetAcquiredWithTag(const UObject* WorldContextObject, FGameplayTag Tag);

	/*~ Tags ~*/

	// 태그 추가
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags", meta = (WorldContext = "WorldContextObject"))
	static void AddTagToSystem(const UObject* WorldContextObject, FGameplayTag Tag);

	// 태그 제거
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Tags", meta = (WorldContext = "WorldContextObject"))
	static void RemoveTagFromSystem(const UObject* WorldContextObject, FGameplayTag Tag);

	// 태그 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|Tags", meta = (WorldContext = "WorldContextObject"))
	static bool HasTagInSystem(const UObject* WorldContextObject, FGameplayTag Tag);

	// 모든 활성 태그
	UFUNCTION(BlueprintPure, Category = "Roguelite|Tags", meta = (WorldContext = "WorldContextObject"))
	static FGameplayTagContainer GetAllTags(const UObject* WorldContextObject);

	/*~ Numeric ~*/

	// 수치 설정
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric", meta = (WorldContext = "WorldContextObject"))
	static void SetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Value);

	// 수치 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Numeric", meta = (WorldContext = "WorldContextObject"))
	static float GetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float DefaultValue = 0.f);

	// 수치 더하기
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Numeric", meta = (WorldContext = "WorldContextObject"))
	static float AddRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Delta);

	// 모든 수치 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Numeric", meta = (WorldContext = "WorldContextObject"))
	static TMap<FGameplayTag, float> GetAllRunStateValues(const UObject* WorldContextObject);

	/*~ Slots ~*/

	// 슬롯에 장착
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots", meta = (WorldContext = "WorldContextObject"))
	static bool EquipActionToSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag);

	// 슬롯에서 해제
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Slots", meta = (WorldContext = "WorldContextObject"))
	static void UnequipActionFromSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag);

	// 슬롯 내용 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|Slots", meta = (WorldContext = "WorldContextObject"))
	static TArray<URogueliteActionData*> GetSlotContents(const UObject* WorldContextObject, FGameplayTag SlotTag);

	// 슬롯 사용 개수
	UFUNCTION(BlueprintPure, Category = "Roguelite|Slots", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSlotCount(const UObject* WorldContextObject, FGameplayTag SlotTag);

	// 슬롯 풀 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|Slots", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotFull(const UObject* WorldContextObject, FGameplayTag SlotTag, int32 MaxCount);

	/*~ Save/Load ~*/

	// 세이브 데이터 생성
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Save", meta = (WorldContext = "WorldContextObject"))
	static FRogueliteRunSaveData CreateRunSaveData(const UObject* WorldContextObject);

	// 세이브 데이터로 복원
	UFUNCTION(BlueprintCallable, Category = "Roguelite|Save", meta = (WorldContext = "WorldContextObject"))
	static void RestoreRunFromSaveData(const UObject* WorldContextObject, const FRogueliteRunSaveData& SaveData);
};
