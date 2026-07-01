#include "RogueliteQueryFilter.h"
#include "RogueliteActionData.h"

/*~ URogueliteQueryFilter ~*/

bool URogueliteQueryFilter::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	return true;
}

/*~ URogueliteFilter_IsAcquired ~*/

bool URogueliteFilter_IsAcquired::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	return RunState.HasAction(Action);
}

/*~ URogueliteFilter_NotAcquired ~*/

bool URogueliteFilter_NotAcquired::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	return !RunState.HasAction(Action);
}

/*~ URogueliteFilter_NotMaxStacked ~*/

bool URogueliteFilter_NotMaxStacked::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (!IsValid(Action))
	{
		return false;
	}

	int32 CurrentStacks = RunState.GetStacks(Action);
	return !Action->IsMaxStacked(CurrentStacks);
}

/*~ URogueliteFilter_HasTags ~*/

bool URogueliteFilter_HasTags::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (!IsValid(Action))
	{
		return false;
	}

	if (RequiredTags.IsEmpty())
	{
		return true;
	}

	if (bRequireAll)
	{
		return Action->HasAllTags(RequiredTags);
	}
	else
	{
		return Action->HasAnyTags(RequiredTags);
	}
}

/*~ URogueliteFilter_ValueCompare ~*/

bool URogueliteFilter_ValueCompare::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (!IsValid(Action) || !Key.IsValid())
	{
		return false;
	}

	float Value;
	if (bUseRunStateValue)
	{
		Value = RunState.GetNumericValue(Key);
	}
	else
	{
		Value = Action->GetValue(Key);
	}

	switch (Operator)
	{
	case ERogueliteCompareOp::Equal:
		return FMath::IsNearlyEqual(Value, CompareValue);
	case ERogueliteCompareOp::NotEqual:
		return !FMath::IsNearlyEqual(Value, CompareValue);
	case ERogueliteCompareOp::Greater:
		return Value > CompareValue;
	case ERogueliteCompareOp::GreaterOrEqual:
		return Value >= CompareValue;
	case ERogueliteCompareOp::Less:
		return Value < CompareValue;
	case ERogueliteCompareOp::LessOrEqual:
		return Value <= CompareValue;
	}

	return false;
}

/*~ URogueliteFilter_And ~*/

bool URogueliteFilter_And::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	for (URogueliteQueryFilter* Filter : SubFilters)
	{
		if (IsValid(Filter) && !Filter->PassesFilter(Action, RunState))
		{
			return false;
		}
	}
	return true;
}

/*~ URogueliteFilter_Or ~*/

bool URogueliteFilter_Or::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (SubFilters.Num() == 0)
	{
		return true;
	}

	for (URogueliteQueryFilter* Filter : SubFilters)
	{
		if (IsValid(Filter) && Filter->PassesFilter(Action, RunState))
		{
			return true;
		}
	}
	return false;
}

/*~ URogueliteFilter_Not ~*/

bool URogueliteFilter_Not::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (!IsValid(SubFilter))
	{
		return true;
	}

	return !SubFilter->PassesFilter(Action, RunState);
}

/*~ URogueliteFilter_ExcludeNewWithTag ~*/

bool URogueliteFilter_ExcludeNewWithTag::PassesFilter_Implementation(URogueliteActionData* Action, const FRogueliteRunState& RunState) const
{
	if (!IsValid(Action))
	{
		return false;
	}

	// 이미 보유 중이면 통과 (업그레이드 허용)
	if (RunState.HasAction(Action))
	{
		return true;
	}

	// 미보유 + 제외 태그 보유 시 제외
	if (!ExcludeTags.IsEmpty() && Action->HasAnyTags(ExcludeTags))
	{
		return false;
	}

	return true;
}
