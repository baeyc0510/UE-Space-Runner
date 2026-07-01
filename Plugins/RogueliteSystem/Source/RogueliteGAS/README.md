# RogueliteGAS 모듈

RogueliteCore와 Unreal Gameplay Ability System(GAS)을 연결하는 브릿지 모듈.

## 아키텍처

```
┌─────────────────────┐     이벤트 구독      ┌──────────────────────────────┐
│  RogueliteSubsystem │◄───────────────────►│ RogueliteAbilityHandler      │
│                     │  OnActionAcquired   │ Component                    │
│  - Action 관리       │  OnActionRemoved    │                              │
│  - 스택 관리         │  OnStackChanged     │  - Ability 부여/해제          │
└─────────────────────┘  OnRunEnded         │  - Effect 적용/제거           │
                                            │  - 트리거 이벤트 구독          │
                                            └──────────────┬───────────────┘
                                                           │
                                                           ▼
                                            ┌──────────────────────────────┐
                                            │  AbilitySystemComponent      │
                                            │                              │
                                            │  - GameplayAbility           │
                                            │  - GameplayEffect            │
                                            │  - GameplayEvent             │
                                            └──────────────────────────────┘
```

## 핵심 개념

### 패시브 vs 트리거

| 구분 | 패시브 | 트리거 |
|------|--------|--------|
| 조건 | TriggerEventTag 없음 | TriggerEventTag 있음 |
| Ability | 획득 시 부여 | 획득 시 부여, 이벤트 시 Activate |
| Effect | 획득 시 적용 | 이벤트 발생 시 적용 |
| 예시 | 최대 체력 +10% | 적 처치 시 폭발 |

### 스택 스케일링

ActionData의 스택 수에 따라 Effect/Ability 강도가 달라지는 방식:

| 모드 | Effect Level | Ability Level | 스택 전달 |
|------|--------------|---------------|-----------|
| SetByCaller | 1 | 1 | SetByCaller Magnitude |
| Level | 스택 수 | 스택 수 | Level 값 자체 |

## 유즈케이스

### 1. 패시브 스탯 증가

**시나리오**: "철갑" 아이템 - 획득 시 방어력 +5, 스택당 +5

1. URogueliteGASActionData 생성
2. Effects에 방어력 증가 GE 추가
3. StackScalingMode = SetByCaller
4. StacksSetByCallerTag 설정
5. GE의 Modifier에서 해당 태그로 Magnitude 참조

**흐름**:
```
플레이어가 "철갑" 획득
    → Subsystem.OnActionAcquired 발생
    → HandlerComponent가 GE 적용 (SetByCaller로 스택 수 전달)
    → ASC에 방어력 +5 적용

"철갑" 추가 획득 (스택 2)
    → Subsystem.OnStackChanged 발생
    → HandlerComponent가 GE 재적용
    → ASC에 방어력 +10 적용
```

### 2. 처치 시 효과 발동

**시나리오**: "연쇄 번개" - 적 처치 시 25% 확률로 주변에 번개

1. URogueliteGASActionData 생성
2. Abilities에 번개 GA 추가
3. TriggerCondition.TriggerEventTag = "Event.Enemy.Killed"
4. TriggerCondition.TriggerChance = 0.25

**흐름**:
```
적 처치 시 GameplayEvent 발생 (Event.Enemy.Killed)
    → HandlerComponent의 이벤트 콜백 호출
    → 확률 체크 (25%)
    → 성공 시 번개 GA Activate
```

### 3. 피해 기반 트리거

**시나리오**: "흡혈" - 피해를 줄 때 피해량의 5% 회복

1. URogueliteGASActionData 생성
2. Effects에 회복 GE 추가
3. TriggerCondition.TriggerEventTag = "Event.Damage.Dealt"
4. TriggerCondition.EventMagnitudeSetByCallerTag = "Data.HealAmount"
5. GE에서 해당 태그로 회복량 계산 (EventMagnitude * 0.05)

**흐름**:
```
플레이어가 100 피해 가함
    → GameplayEvent 발생 (EventMagnitude = 100)
    → HandlerComponent가 GE 적용
    → SetByCaller로 100 전달
    → GE 내부에서 100 * 0.05 = 5 회복
```

### 4. 쿨다운 있는 트리거

**시나리오**: "수호 천사" - 피격 시 보호막 생성 (10초 쿨다운)

1. URogueliteGASActionData 생성
2. Effects에 보호막 GE 추가
3. TriggerCondition.TriggerEventTag = "Event.Damage.Taken"
4. TriggerCondition.Cooldown = 10.0

## 컴포넌트 설정

### 필터링

특정 태그를 가진 Action만 처리하도록 필터 설정 가능:

- **TargetActionTags**: 처리할 Action의 태그
- **bRequireAllTags**: true면 모든 태그 필요, false면 하나만 있어도 됨

**활용 예시**:
- 플레이어 전용 Handler: `TargetActionTags = "Action.Player"`
- 무기 전용 Handler: `TargetActionTags = "Action.Weapon"`

### ASC 연결

1. **자동**: Owner Actor에서 IAbilitySystemInterface 또는 컴포넌트 검색
2. **수동**: `SetAbilitySystemComponent()` 호출

## 델리게이트

### OnTriggerAction

트리거 발동 시 브로드캐스트. UI 피드백, 사운드 재생 등에 활용.

파라미터:
- Action: 발동된 ActionData
- EventTag: 트리거 이벤트 태그
- Payload: 이벤트 데이터 (Instigator, Target, Magnitude 등)

## 주의사항

1. **GE Duration**: 패시브 Effect는 Infinite Duration 권장
2. **SetByCaller 태그**: GE와 ActionData 간 태그 일치 필수
3. **이벤트 태그 계층**: bExactMatch=false 시 하위 태그도 트리거됨
