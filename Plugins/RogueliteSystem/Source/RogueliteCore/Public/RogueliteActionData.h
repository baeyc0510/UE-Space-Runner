#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RogueliteTypes.h"
#include "RogueliteActionData.generated.h"

/**
 * 로그라이트 시스템의 기본 액션 데이터.
 * 게임에서 획득 가능한 모든 것(무기, 스킬, 패시브 등)의 기반 클래스.
 */
UCLASS(BlueprintType, Blueprintable)
class ROGUELITECORE_API URogueliteActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/*~ Display ~*/

	// 표시 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	// 설명 (템플릿 지원: "{Stat.Attack}" 형식으로 Values 참조 가능)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	/*~ Classification ~*/

	// 분류 태그 (Type, Pool, Rarity 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classification")
	FGameplayTagContainer ActionTags;

	// 획득 시 Tags를 RunState.ActiveTags에 자동 부여
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classification")
	FGameplayTagContainer TagsToGrant;

	/*~ Values ~*/

	// 수치 데이터 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Values")
	TArray<FRogueliteValueEntry> Values;

	/*~ Auto Apply ~*/

	// 획득 시 Values를 RunState.NumericData에 자동 적용 (플레이어)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Values")
	bool bAutoApplyToRunState = false;

	// 획득 시 Values를 StatReceiverComponent에 자동 적용 (펫, 터렛 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Values")
	bool bAutoApplyToReceiver = false;

	/*~ Pool Settings ~*/

	// 기본 등장 가중치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool")
	float BaseWeight = 1.0f;

	// 최대 스택 수 (0 = 무제한)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pool")
	int32 MaxStacks = 1;

	/*~ Conditions ~*/

	// 필요 태그 (이 태그들 보유 시에만 등장)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer RequiredTags;

	// 차단 태그 (이 태그들 보유 시 제외)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer BlockedByTags;

public:
	/*~ UPrimaryDataAsset Interface ~*/
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	// 포맷팅된 DisplayName
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	virtual FText GetFormattedTextWithValues(const FText& TextToFormat);
	
	// 특정 키의 값 조회
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	float GetValue(FGameplayTag Key, float DefaultValue = 0.f) const;
	
	// 특정 키의 값 정수형으로 반환
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	int64 GetValueAsInt(FGameplayTag Key, int64 DefaultValue = 0) const;

	// 특정 태그 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	bool HasTag(FGameplayTag Tag) const;

	// 태그 중 하나라도 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	bool HasAnyTags(const FGameplayTagContainer& InTags) const;

	// 모든 태그 보유 여부
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	bool HasAllTags(const FGameplayTagContainer& InTags) const;

	// 최대 스택 도달 여부 확인
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	bool IsMaxStacked(int32 CurrentStacks) const;

	// 조건 충족 여부 (RequiredTags, BlockedByTags 체크)
	UFUNCTION(BlueprintPure, Category = "Roguelite|ActionData")
	bool MeetsConditions(const FGameplayTagContainer& ActiveTags) const;
};
