#include "RogueliteSettings.h"

URogueliteSettings::URogueliteSettings()
{
}

const URogueliteSettings* URogueliteSettings::Get()
{
	return GetDefault<URogueliteSettings>();
}
