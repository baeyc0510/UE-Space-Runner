#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageHandler.generated.h"

struct FDamageContext;
struct FDamageResult;

/**
 * 데미지 공식 적용 및 데미지 이벤트 처리
 * - FDamageContext를 받아 데미지 공식 적용
 * - 모든 스탯은 Context에서 조회 (공격자/방어자 스탯 모두 포함)
 * - BP 상속하여 OnDamageApplied에서 UI 처리 가능
 */
UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class RUNNER_API UDamageHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageHandler();

	/*~ Damage Processing ~*/

	// 데미지 수신 처리 - 공식 적용 후 결과 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void OnDamageReceived(const FDamageContext& InDamageContext, UPARAM(ref) FDamageResult& OutDamageResult);

	// 데미지 적용 완료 이벤트 (UI 표시 등)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void OnDamageApplied(const FDamageResult& InDamageResult);

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

	/*~ Damage Calculation ~*/

	// 데미지 공식 계산: FinalDamage = BaseDamage * (1 + DamageBonus) * CritMultiplier - Defense
	virtual float CalculateDamage(const FDamageContext& InDamageContext) const;
};
