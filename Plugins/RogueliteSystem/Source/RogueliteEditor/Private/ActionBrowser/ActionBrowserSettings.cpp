#include "ActionBrowser/ActionBrowserSettings.h"

const FRogueliteQuery& UActionBrowserSettings::GetQuery() const
{
	return Query.Query;
}

FRogueliteRunState UActionBrowserSettings::ToRunState() const
{
	if (Query.bUseSimulatedState)
	{
		return Query.SimulatedState.ToRunState();
	}
	return FRogueliteRunState();
}

void UActionBrowserSettings::ResetToDefault()
{
	Query = FActionBrowserQuery();
}
