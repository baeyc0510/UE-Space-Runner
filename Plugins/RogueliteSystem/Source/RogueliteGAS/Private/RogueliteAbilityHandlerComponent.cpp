#include "RogueliteAbilityHandlerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "RogueliteGASActionData.h"
#include "RogueliteCore/Public/RogueliteActionData.h"
#include "RogueliteCore/Public/RogueliteSubsystem.h"

URogueliteAbilityHandlerComponent::URogueliteAbilityHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URogueliteAbilityHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	FindAbilitySystemComponent();
	BindToSubsystem();

	// 이미 런이 진행 중이면 기존 Action 동기화
	if (CachedSubsystem.IsValid() && CachedSubsystem->IsRunActive())
	{
		SyncExistingActions();
	}
}

void URogueliteAbilityHandlerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromAllEvents();
	ClearAllHandles();
	UnbindFromSubsystem();

	Super::EndPlay(EndPlayReason);
}

void URogueliteAbilityHandlerComponent::SetAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	if (CachedASC.Get() == InASC)
	{
		return;
	}

	// 기존 Action 목록 보존 (ClearAllHandles가 AcquiredActions를 비우므로)
	TArray<URogueliteGASActionData*> PreviousActions = AcquiredActions;

	// 기존 ASC의 구독 및 핸들 정리
	UnsubscribeFromAllEvents();
	ClearAllHandles();

	CachedASC = InASC;

	if (!IsValid(InASC))
	{
		return;
	}

	// 새 ASC에 기존 Action들 재적용
	if (CachedSubsystem.IsValid())
	{
		for (URogueliteGASActionData* Action : PreviousActions)
		{
			if (!IsValid(Action))
			{
				continue;
			}

			int32 Stacks = CachedSubsystem->GetActionStacks(Action);

			AcquiredActions.Add(Action);
			FRogueliteGASHandles& Handles = ActionHandleMap.FindOrAdd(Action);
			Handles.AppliedStacks = Stacks;

			SetupActionGAS(Action, Stacks, Handles);
		}
	}
}

UAbilitySystemComponent* URogueliteAbilityHandlerComponent::GetAbilitySystemComponent() const
{
	return CachedASC.Get();
}

void URogueliteAbilityHandlerComponent::SyncExistingActions()
{
	if (!CachedSubsystem.IsValid())
	{
		return;
	}

	TArray<URogueliteActionData*> AllAcquired = CachedSubsystem->GetAllAcquired();
	for (URogueliteActionData* Action : AllAcquired)
	{
		int32 Stacks = CachedSubsystem->GetActionStacks(Action);
		HandleActionAcquired(Action, 0, Stacks);
	}
}

void URogueliteAbilityHandlerComponent::ClearAllHandles()
{
	if (CachedASC.IsValid())
	{
		for (auto& Pair : ActionHandleMap)
		{
			ClearAbilities(Pair.Value);
			RemoveEffects(Pair.Value);
		}
	}

	ActionHandleMap.Empty();
	AcquiredActions.Empty();
}

bool URogueliteAbilityHandlerComponent::GetActionHandles(URogueliteActionData* Action, FRogueliteGASHandles& OutHandles) const
{
	URogueliteGASActionData* GASAction = Cast<URogueliteGASActionData>(Action);
	if (!GASAction)
	{
		return false;
	}

	if (const FRogueliteGASHandles* Found = ActionHandleMap.Find(GASAction))
	{
		OutHandles = *Found;
		return true;
	}
	return false;
}

void URogueliteAbilityHandlerComponent::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	// GASActionData 캐스트
	URogueliteGASActionData* GASAction = Cast<URogueliteGASActionData>(Action);
	if (!GASAction || !GASAction->HasGASElements())
	{
		return;
	}

	// 이미 처리된 Action이면 스킵 (중복 부여 방지)
	if (ActionHandleMap.Contains(GASAction))
	{
		return;
	}

	// 필터 체크
	if (!PassesFilter(Action))
	{
		return;
	}

	// ASC 체크
	if (!CachedASC.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RogueliteAbilityHandler] No valid ASC for action: %s"), *Action->GetName());
		return;
	}

	// AcquiredActions에 추가
	AcquiredActions.AddUnique(GASAction);

	FRogueliteGASHandles& Handles = ActionHandleMap.FindOrAdd(GASAction);
	Handles.AppliedStacks = NewStacks;

	SetupActionGAS(GASAction, NewStacks, Handles);
}

void URogueliteAbilityHandlerComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	URogueliteGASActionData* GASAction = Cast<URogueliteGASActionData>(Action);
	if (!GASAction)
	{
		return;
	}

	FRogueliteGASHandles* Handles = ActionHandleMap.Find(GASAction);
	if (!Handles)
	{
		return;
	}

	// GAS 핸들 정리
	if (CachedASC.IsValid())
	{
		ClearAbilities(*Handles);
		RemoveEffects(*Handles);
	}

	// 트리거 이벤트 구독 해제
	if (GASAction->HasTrigger())
	{
		UnsubscribeActionFromEvent(GASAction);
	}

	ActionHandleMap.Remove(GASAction);
	AcquiredActions.Remove(GASAction);
}

void URogueliteAbilityHandlerComponent::HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	URogueliteGASActionData* GASAction = Cast<URogueliteGASActionData>(Action);
	if (!GASAction)
	{
		return;
	}

	FRogueliteGASHandles* Handles = ActionHandleMap.Find(GASAction);
	if (!Handles)
	{
		return;
	}

	RefreshForStackChange(GASAction, NewStacks, *Handles);
}

void URogueliteAbilityHandlerComponent::HandleRunEnded(bool bCompleted)
{
	UnsubscribeFromAllEvents();
	ClearAllHandles();
}

bool URogueliteAbilityHandlerComponent::PassesFilter(URogueliteActionData* Action) const
{
	if (TargetActionTags.IsEmpty())
	{
		return true;
	}

	if (bRequireAllTags)
	{
		return Action->HasAllTags(TargetActionTags);
	}
	else
	{
		return Action->HasAnyTags(TargetActionTags);
	}
}

void URogueliteAbilityHandlerComponent::FindAbilitySystemComponent()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	// IAbilitySystemInterface를 통한 검색
	CachedASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);

	if (!CachedASC.IsValid())
	{
		// 직접 컴포넌트 검색
		CachedASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	}
}

void URogueliteAbilityHandlerComponent::BindToSubsystem()
{
	if (bIsBoundToSubsystem)
	{
		return;
	}

	CachedSubsystem = URogueliteSubsystem::Get(this);
	if (!CachedSubsystem.IsValid())
	{
		return;
	}

	CachedSubsystem->OnActionAcquired.AddDynamic(this, &URogueliteAbilityHandlerComponent::HandleActionAcquired);
	CachedSubsystem->OnActionRemoved.AddDynamic(this, &URogueliteAbilityHandlerComponent::HandleActionRemoved);
	CachedSubsystem->OnStackChanged.AddDynamic(this, &URogueliteAbilityHandlerComponent::HandleStackChanged);
	CachedSubsystem->OnRunEnded.AddDynamic(this, &URogueliteAbilityHandlerComponent::HandleRunEnded);

	bIsBoundToSubsystem = true;
}

void URogueliteAbilityHandlerComponent::UnbindFromSubsystem()
{
	if (!bIsBoundToSubsystem || !CachedSubsystem.IsValid())
	{
		bIsBoundToSubsystem = false;
		return;
	}

	CachedSubsystem->OnActionAcquired.RemoveDynamic(this, &URogueliteAbilityHandlerComponent::HandleActionAcquired);
	CachedSubsystem->OnActionRemoved.RemoveDynamic(this, &URogueliteAbilityHandlerComponent::HandleActionRemoved);
	CachedSubsystem->OnStackChanged.RemoveDynamic(this, &URogueliteAbilityHandlerComponent::HandleStackChanged);
	CachedSubsystem->OnRunEnded.RemoveDynamic(this, &URogueliteAbilityHandlerComponent::HandleRunEnded);

	bIsBoundToSubsystem = false;
}

void URogueliteAbilityHandlerComponent::SubscribeActionToEvent(URogueliteGASActionData* Action)
{
	if (!CachedASC.IsValid() || !IsValid(Action) || !Action->HasTrigger())
	{
		return;
	}

	// 이미 구독 중이면 스킵
	if (GameplayEventSubscriptions.Contains(Action))
	{
		return;
	}

	FGameplayTag EventTag = Action->GetTriggerEventTag();
	bool bExactMatch = Action->TriggerCondition.bExactMatch;
	FDelegateHandle Handle;

	// TWeakObjectPtr 사용 (GC 안전)
	TWeakObjectPtr<URogueliteGASActionData> WeakAction = Action;

	// 공통 핸들러
	auto TriggerHandler = [this, WeakAction](const FGameplayEventData* Payload)
	{
		URogueliteGASActionData* Action = WeakAction.Get();
		if (!Action || !Payload)
		{
			return;
		}

		FRogueliteGASHandles* Handles = ActionHandleMap.Find(Action);
		if (Handles)
		{
			ProcessTrigger(Action, Payload, *Handles);
		}
	};

	if (bExactMatch)
	{
		Handle = CachedASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddLambda(TriggerHandler);
	}
	else
	{
		Handle = CachedASC->AddGameplayEventTagContainerDelegate(
			FGameplayTagContainer(EventTag),
			FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda(
				[TriggerHandler](FGameplayTag, const FGameplayEventData* Payload)
				{
					TriggerHandler(Payload);
				}));
	}

	GameplayEventSubscriptions.Add(Action, Handle);
}

void URogueliteAbilityHandlerComponent::RemoveEventSubscription(URogueliteGASActionData* Action, const FDelegateHandle& Handle)
{
	if (!CachedASC.IsValid() || !IsValid(Action))
	{
		return;
	}

	FGameplayTag EventTag = Action->GetTriggerEventTag();
	bool bExactMatch = Action->TriggerCondition.bExactMatch;

	if (bExactMatch)
	{
		if (FGameplayEventMulticastDelegate* Delegate = CachedASC->GenericGameplayEventCallbacks.Find(EventTag))
		{
			Delegate->Remove(Handle);
		}
	}
	else
	{
		CachedASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(EventTag), Handle);
	}
}

void URogueliteAbilityHandlerComponent::UnsubscribeActionFromEvent(URogueliteGASActionData* Action)
{
	if (!IsValid(Action))
	{
		return;
	}

	FDelegateHandle* Handle = GameplayEventSubscriptions.Find(Action);
	if (!Handle)
	{
		return;
	}

	RemoveEventSubscription(Action, *Handle);
	GameplayEventSubscriptions.Remove(Action);
}

void URogueliteAbilityHandlerComponent::UnsubscribeFromAllEvents()
{
	for (const auto& Pair : GameplayEventSubscriptions)
	{
		RemoveEventSubscription(Pair.Key, Pair.Value);
	}

	GameplayEventSubscriptions.Empty();
}

void URogueliteAbilityHandlerComponent::GrantAbilities(URogueliteGASActionData* Action, int32 Level, FRogueliteGASHandles& OutHandles)
{
	if (!CachedASC.IsValid() || !IsValid(Action))
	{
		return;
	}

	for (TSubclassOf<UGameplayAbility> AbilityClass : Action->Abilities)
	{
		if (!IsValid(AbilityClass))
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, Level, INDEX_NONE, GetOwner());
		FGameplayAbilitySpecHandle Handle = CachedASC->GiveAbility(Spec);

		if (Handle.IsValid())
		{
			OutHandles.GrantedAbilities.Add(Handle);
		}
	}
}

void URogueliteAbilityHandlerComponent::ClearAbilities(FRogueliteGASHandles& Handles)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : Handles.GrantedAbilities)
	{
		if (Handle.IsValid())
		{
			CachedASC->ClearAbility(Handle);
		}
	}

	Handles.GrantedAbilities.Empty();
}

void URogueliteAbilityHandlerComponent::ApplyEffectsInternal(URogueliteGASActionData* Action, int32 Stacks, const FGameplayEventData* Payload, FRogueliteGASHandles& OutHandles)
{
	if (!CachedASC.IsValid() || !IsValid(Action))
	{
		return;
	}

	for (TSubclassOf<UGameplayEffect> EffectClass : Action->Effects)
	{
		if (!IsValid(EffectClass))
		{
			continue;
		}

		FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();
		Context.AddSourceObject(Action);

		// 트리거 발동 시 Payload에서 Instigator 설정
		if (Payload)
		{
			Context.AddInstigator(const_cast<AActor*>(Payload->Instigator.Get()), const_cast<AActor*>(Payload->Instigator.Get()));
		}

		int32 EffectLevel = (Action->StackScalingMode == ERogueliteStackScalingMode::Level) ? Stacks : 1;
		FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(EffectClass, EffectLevel, Context);

		if (SpecHandle.IsValid())
		{
			ApplySetByCallerValues(Action, SpecHandle, Stacks, Payload);

			FActiveGameplayEffectHandle EffectHandle = CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			if (EffectHandle.IsValid())
			{
				OutHandles.AppliedEffects.Add(EffectHandle);

				// Effect 제거 시 핸들 자동 정리 구독
				if (FOnActiveGameplayEffectRemoved_Info* RemovalDelegate = CachedASC->OnGameplayEffectRemoved_InfoDelegate(EffectHandle))
				{
					RemovalDelegate->AddUObject(this, &URogueliteAbilityHandlerComponent::HandleGameplayEffectRemoved);
				}
			}
		}
	}
}

void URogueliteAbilityHandlerComponent::RemoveEffects(FRogueliteGASHandles& Handles)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	// 순회 중 콜백에서 배열 수정 방지를 위해 복사본 사용
	TArray<FActiveGameplayEffectHandle> HandlesToRemove = MoveTemp(Handles.AppliedEffects);
	Handles.AppliedEffects.Empty();

	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		if (Handle.IsValid())
		{
			CachedASC->RemoveActiveGameplayEffect(Handle);
		}
	}
}

void URogueliteAbilityHandlerComponent::ApplySetByCallerValues(URogueliteGASActionData* Action, FGameplayEffectSpecHandle& SpecHandle, int32 Stacks, const FGameplayEventData* Payload)
{
	if (!SpecHandle.IsValid() || !IsValid(Action))
	{
		return;
	}

	// 스택 수를 설정된 SetByCaller 태그로 전달
	const FGameplayTag& StacksTag = Action->StacksSetByCallerTag;
	if (StacksTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(StacksTag, static_cast<float>(Stacks));
	}

	// ActionData의 Values를 SetByCaller로 매핑
	for (const auto& Mapping : Action->ValueToSetByCallerMap)
	{
		float Value = Action->GetValue(Mapping.Key);
		SpecHandle.Data->SetSetByCallerMagnitude(Mapping.Value, Value * Stacks);
	}

	// 트리거 발동 시 EventMagnitude를 설정된 SetByCaller 태그로 전달
	if (Payload)
	{
		const FGameplayTag& EventMagnitudeTag = Action->TriggerCondition.EventMagnitudeSetByCallerTag;
		if (EventMagnitudeTag.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(EventMagnitudeTag, Payload->EventMagnitude);
		}
	}
}

void URogueliteAbilityHandlerComponent::SetupActionGAS(URogueliteGASActionData* Action, int32 Stacks, FRogueliteGASHandles& Handles)
{
	int32 AbilityLevel = (Action->StackScalingMode == ERogueliteStackScalingMode::Level) ? Stacks : 1;
	GrantAbilities(Action, AbilityLevel, Handles);

	if (Action->HasTrigger())
	{
		// 트리거: 이벤트 구독 (Activate는 이벤트 발생 시)
		SubscribeActionToEvent(Action);
	}
	else
	{
		// 패시브: 즉시 Effect 적용
		ApplyEffectsInternal(Action, Stacks, nullptr, Handles);
	}
}

void URogueliteAbilityHandlerComponent::RefreshForStackChange(URogueliteGASActionData* Action, int32 NewStacks, FRogueliteGASHandles& Handles)
{
	if (!CachedASC.IsValid())
	{
		return;
	}

	// 패시브 Action만 Effect 재적용 (트리거는 이벤트 발생 시에만 Effect 적용)
	if (!Action->HasTrigger())
	{
		// Effect 재적용
		RemoveEffects(Handles);
		ApplyEffectsInternal(Action, NewStacks, nullptr, Handles);
	}

	// Ability 레벨 갱신 (Level 모드일 때만)
	if (Action->StackScalingMode == ERogueliteStackScalingMode::Level)
	{
		for (const FGameplayAbilitySpecHandle& Handle : Handles.GrantedAbilities)
		{
			if (FGameplayAbilitySpec* Spec = CachedASC->FindAbilitySpecFromHandle(Handle))
			{
				Spec->Level = NewStacks;
				CachedASC->MarkAbilitySpecDirty(*Spec);
			}
		}
	}

	Handles.AppliedStacks = NewStacks;
}

void URogueliteAbilityHandlerComponent::ProcessTrigger(URogueliteGASActionData* Action, const FGameplayEventData* Payload, FRogueliteGASHandles& Handles)
{
	const FRogueliteTriggerCondition& Condition = Action->TriggerCondition;

	// 쿨다운 체크
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Handles.TriggerState.IsOnCooldown(CurrentTime, Condition.Cooldown))
	{
		return;
	}

	// 확률 체크
	if (Condition.TriggerChance < 1.0f)
	{
		if (FMath::FRand() > Condition.TriggerChance)
		{
			return;
		}
	}

	// Ability Activate
	if (!CachedASC.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : Handles.GrantedAbilities)
	{
		CachedASC->TryActivateAbility(Handle);
	}

	// Effect Apply (트리거 시에는 Payload와 함께)
	ApplyEffectsInternal(Action, Handles.AppliedStacks, Payload, Handles);

	// 트리거 상태 갱신
	Handles.TriggerState.LastTriggerTime = CurrentTime;

	// 델리게이트 브로드캐스트
	OnTriggerAction.Broadcast(Action, Action->GetTriggerEventTag(), *Payload);
}

void URogueliteAbilityHandlerComponent::HandleGameplayEffectRemoved(const FGameplayEffectRemovalInfo& RemovalInfo)
{
	if (!RemovalInfo.ActiveEffect)
	{
		return;
	}

	FActiveGameplayEffectHandle RemovedHandle = RemovalInfo.ActiveEffect->Handle;

	// 모든 Action의 AppliedEffects에서 해당 핸들 제거
	for (auto& Pair : ActionHandleMap)
	{
		Pair.Value.AppliedEffects.RemoveAll([&RemovedHandle](const FActiveGameplayEffectHandle& Handle)
		{
			return Handle == RemovedHandle;
		});
	}
}
