#include "ActionBrowser/ActionBrowserTypes.h"
#include "RogueliteActionData.h"

FRogueliteRunState FSimulatedRunState::ToRunState() const
{
	FRogueliteRunState Result;
	Result.bActive = true;

	// 획득 액션 변환
	for (const FSimulatedAcquiredAction& SimAction : AcquiredActions)
	{
		if (IsValid(SimAction.Action))
		{
			FRogueliteAcquiredInfo Info;
			// MaxStacks가 0이면 무제한, 아니면 클램프
			const int32 MaxStacks = SimAction.Action->MaxStacks;
			Info.Stacks = (MaxStacks > 0) ? FMath::Clamp(SimAction.Stacks, 1, MaxStacks) : SimAction.Stacks;
			Info.AcquiredTime = 0.f;
			Result.AcquiredActions.Add(SimAction.Action, Info);
			Result.ActiveTagStacks.AppendTags(SimAction.Action->TagsToGrant);
		}
	}

	// 활성 태그 변환
	Result.ActiveTagStacks.AppendTags(ActiveTags);

	return Result;
}
