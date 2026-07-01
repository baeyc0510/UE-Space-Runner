// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "NativeGameplayTags.h"
#include "Logging/LogMacros.h"
#include "Runner/Combat/CombatCharacter.h"
#include "RunnerCharacter.generated.h"

class URogueliteActionData;
class UCombatStatData;
/*~ Player 전용 태그 ~*/
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stat_MoveSpeedBonus);

// Player Log Channel
DECLARE_LOG_CATEGORY_EXTERN(LogRunnerCharacter, Log, All);

// Forward Declarations
struct FDamageResult;
class URunnerAttributeSet;
class UAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(config=Game)
class ARunnerCharacter : public ACombatCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ARunnerCharacter();

	/*~ IAbilitySystemInterface ~*/
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	/*~ ICombatInterface ~*/
	virtual float GetCombatStat(FGameplayTag StatTag) const override;
	virtual void OnDamageDealt(const FDamageResult& InDamageResult) override;
	
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	
	void StopMoving();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	// 이동 속도 갱신 (MoveSpeed 스탯 반영)
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void UpdateMovementSpeed();

private:
	// Action 획득 이벤트 핸들러
	UFUNCTION()
	void HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks);
	
	// RunState 값 변경 이벤트 핸들러
	UFUNCTION()
	void HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue);
	
	void InitDefaultPlayerStatTags();
	void ApplyStartupStats();
	
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE  USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE  UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Combat|Stats")
	TObjectPtr<UCombatStatData> StatData;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Combat|Stats")
	float BaseMovementSpeed = 600.f;
	
	UPROPERTY(BlueprintReadOnly)
	FVector2D MovementVector;
	
	UPROPERTY(BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;
};

