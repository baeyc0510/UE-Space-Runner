#include "RogueliteActionData.h"

FPrimaryAssetId URogueliteActionData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("RogueliteActionData"), GetFName());
}

FText URogueliteActionData::GetFormattedTextWithValues(const FText& TextToFormat)
{
	TArray<FText> Args;
    
	for (const FRogueliteValueEntry& ValueEntry : Values)
	{
		FText FormattedValue;
        
		// 포맷 모드에 따른 값 변환
		switch (ValueEntry.FormatMode)
		{
		case ERogueliteFormatMode::Float:
			FormattedValue = FText::AsNumber(ValueEntry.Value);
			break;
            
		case ERogueliteFormatMode::Integer:
			FormattedValue = FText::AsNumber(FMath::RoundToInt(ValueEntry.Value));
			break;
            
		case ERogueliteFormatMode::Percent:
			FormattedValue = FText::AsPercent(ValueEntry.Value); // 0.1 → "10%"
			break;
            
		default:
			FormattedValue = FText::AsNumber(ValueEntry.Value);
			break;
		}
        
		// 적용 모드에 따른 부호 추가
		switch (ValueEntry.ApplyMode)
		{
		case ERogueliteApplyMode::Add:
			if (ValueEntry.Value >= 0)
			{
				FormattedValue = FText::Format(NSLOCTEXT("Roguelite", "AddPositive", "+{0}"), FormattedValue);
			}
			// 음수는 이미 - 붙어있음
			break;
            
		case ERogueliteApplyMode::Multiply:
			FormattedValue = FText::Format(NSLOCTEXT("Roguelite", "Multiply", "x{0}"), FormattedValue);
			break;
            
			// TODO: 다른 Mode 지원
		default:
			break;
		}
        
		Args.Add(FormattedValue);
	}
    
	// Ordered Arguments로 변환
	FFormatOrderedArguments OrderedArgs;
	for (const FText& Arg : Args)
	{
		OrderedArgs.Add(Arg);
	}
    
	return FText::Format(TextToFormat, OrderedArgs);
}

float URogueliteActionData::GetValue(FGameplayTag Key, float DefaultValue) const
{
	for (const FRogueliteValueEntry& Entry : Values)
	{
		if (Entry.Key == Key)
		{
			return Entry.Value;
		}
	}
	return DefaultValue;
}

int64 URogueliteActionData::GetValueAsInt(FGameplayTag Key, int64 DefaultValue) const
{
	float Value = GetValue(Key, DefaultValue);
	return static_cast<int64>(Value);
}

bool URogueliteActionData::HasTag(FGameplayTag Tag) const
{
	return ActionTags.HasTag(Tag);
}

bool URogueliteActionData::HasAnyTags(const FGameplayTagContainer& InTags) const
{
	return ActionTags.HasAny(InTags);
}

bool URogueliteActionData::HasAllTags(const FGameplayTagContainer& InTags) const
{
	return ActionTags.HasAll(InTags);
}

bool URogueliteActionData::IsMaxStacked(int32 CurrentStacks) const
{
	if (MaxStacks <= 0)
	{
		return false;
	}
	return CurrentStacks >= MaxStacks;
}

bool URogueliteActionData::MeetsConditions(const FGameplayTagContainer& ActiveTags) const
{
	// RequiredTags가 있으면 모두 보유해야 함
	if (!RequiredTags.IsEmpty() && !ActiveTags.HasAll(RequiredTags))
	{
		return false;
	}

	// BlockedByTags 중 하나라도 있으면 제외
	if (!BlockedByTags.IsEmpty() && ActiveTags.HasAny(BlockedByTags))
	{
		return false;
	}

	return true;
}
