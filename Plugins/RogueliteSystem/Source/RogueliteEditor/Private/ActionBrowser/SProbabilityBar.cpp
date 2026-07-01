#include "ActionBrowser/SProbabilityBar.h"
#include "Rendering/DrawElements.h"

void SProbabilityBar::Construct(const FArguments& InArgs)
{
	Probability = InArgs._Probability;
	BarColor = InArgs._BarColor;
}

void SProbabilityBar::SetProbability(float InProbability)
{
	Probability = FMath::Clamp(InProbability, 0.f, 1.f);
}

void SProbabilityBar::SetBarColor(FLinearColor InColor)
{
	BarColor = InColor;
}

int32 SProbabilityBar::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
) const
{
	const bool bEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const float CurrentProbability = Probability.Get();
	const FLinearColor CurrentBarColor = BarColor.Get();

	// 배경 (어두운 회색)
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("WhiteBrush"),
		DrawEffects,
		FLinearColor(0.1f, 0.1f, 0.1f, 1.f)
	);

	// 막대 (확률에 비례)
	if (CurrentProbability > 0.f)
	{
		const float BarWidth = LocalSize.X * CurrentProbability;
		const FVector2D BarSize(BarWidth, LocalSize.Y);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(BarSize, FSlateLayoutTransform()),
			FAppStyle::GetBrush("WhiteBrush"),
			DrawEffects,
			CurrentBarColor
		);
	}

	return LayerId + 1;
}

FVector2D SProbabilityBar::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	// 기본 크기: 너비 100, 높이 16
	return FVector2D(100.f, 16.f);
}
