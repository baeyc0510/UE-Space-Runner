// Copyright Epic Games, Inc. All Rights Reserved.

#include "RunnerCharacter.h"
#include "PlayerHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "RogueliteBlueprintLibrary.h"
#include "RogueliteSubsystem.h"
#include "Runner/Combat/CombatTypes.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RogueliteActionData.h"
#include "Runner/Combat/CombatStatData.h"

DEFINE_LOG_CATEGORY(LogRunnerCharacter);

/*~ Player 전용 태그 ~*/
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Stat_MoveSpeedBonus, "Stat.MoveSpeedBonus", "이동 속도 보너스%");

//////////////////////////////////////////////////////////////////////////
// ARunnerCharacter

ARunnerCharacter::ARunnerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetupAttachment(CameraBoom);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));

	// Player용 HealthComponent (RunState 동기화)
	CreateHealthComponent<UPlayerHealthComponent>();
	
	// Player 스탯 기본값 초기화
	InitDefaultPlayerStatTags();
}

void ARunnerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// RunState 값 변경 이벤트 바인딩
	if (URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this))
	{
		RogueliteSubsystem->OnActionAcquired.AddDynamic(this,&ARunnerCharacter::HandleActionAcquired);
		RogueliteSubsystem->OnRunStateValueChanged.AddDynamic(this, &ARunnerCharacter::HandleRunStateValueChanged);
	}

	// 초기 스탯 적용
	ApplyStartupStats();
	
	// 이동 속도 적용
	UpdateMovementSpeed();
}

void ARunnerCharacter::UpdateMovementSpeed()
{
	const float MoveSpeedBonus = GetCombatStat(TAG_Stat_MoveSpeedBonus);
	const float FinalSpeed = BaseMovementSpeed * (1.0f + MoveSpeedBonus);
	GetCharacterMovement()->MaxWalkSpeed = FinalSpeed;
}

void ARunnerCharacter::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (IsValid(StatData) && IsValid(StatData->MinMaxSettingAction))
	{
		if (URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this))
		{
			RogueliteSubsystem->ApplyActionEffects(StatData->MinMaxSettingAction,1);
		}
	}
}

void ARunnerCharacter::HandleRunStateValueChanged(FGameplayTag Key, float OldValue, float NewValue)
{
	if (Key == TAG_Stat_MoveSpeedBonus)
	{
		UpdateMovementSpeed();
	}
}

void ARunnerCharacter::InitDefaultPlayerStatTags()
{
	// Init Tags
	CombatContextStatTags.AddTag(TAG_Stat_MaxHealth);
	CombatContextStatTags.AddTag(TAG_Stat_CurrentHealth);
	CombatContextStatTags.AddTag(TAG_Stat_Defense);
	CombatContextStatTags.AddTag(TAG_Stat_DamageBonus);
	CombatContextStatTags.AddTag(TAG_Stat_CriticalChance);
	CombatContextStatTags.AddTag(TAG_Stat_CriticalBonus);
	CombatContextStatTags.AddTag(TAG_Stat_LifeSteal);
	CombatContextStatTags.AddTag(TAG_Stat_MoveSpeedBonus);
}

void ARunnerCharacter::ApplyStartupStats()
{
	if (!IsValid(StatData))
	{
		return;
	}
	
	for (auto& KVP : StatData->Stats)
	{
		// 모든 런에 적용되는 기본 스탯이 적용되어 있을 수 있으므로 Set이 아닌 Add 
		URogueliteBlueprintLibrary::AddRunStateValue(this,KVP.Key,KVP.Value);
	}
}

/*~ ICombatInterface ~*/

float ARunnerCharacter::GetCombatStat(FGameplayTag StatTag) const
{
	return URogueliteBlueprintLibrary::GetRunStateValue(this, StatTag, 0.0f);
}

void ARunnerCharacter::OnDamageDealt(const FDamageResult& InDamageResult)
{
	if (!IsValid(HealthComponent))
	{
		return;
	}
	
	// 흡혈 처리
	const float LifeSteal = GetCombatStat(TAG_Stat_LifeSteal);
	if (LifeSteal > 0.0f)
	{
		const float HealAmount = InDamageResult.Damage * LifeSteal;
		HealthComponent->Heal(HealAmount);
	}
}

/*~ Input ~*/

void ARunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARunnerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ARunnerCharacter::StopMoving);
	}
	else
	{
		UE_LOG(LogRunnerCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARunnerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();
}

void ARunnerCharacter::StopMoving()
{
	MovementVector = FVector2D::ZeroVector;
}
