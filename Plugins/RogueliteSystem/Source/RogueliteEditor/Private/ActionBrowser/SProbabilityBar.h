#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

/*~ 확률 시각화 막대 그래프 ~*/
class SProbabilityBar : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SProbabilityBar)
		: _Probability(0.f)
		, _BarColor(FLinearColor::Green)
	{}
		SLATE_ATTRIBUTE(float, Probability)
		SLATE_ATTRIBUTE(FLinearColor, BarColor)
	SLATE_END_ARGS()

	// 위젯 생성
	void Construct(const FArguments& InArgs);

	// 확률 값 설정
	void SetProbability(float InProbability);

	// 색상 설정
	void SetBarColor(FLinearColor InColor);

	/*~ SWidget Interface ~*/
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	// 확률 값 (0.0 ~ 1.0)
	TAttribute<float> Probability;

	// 막대 색상
	TAttribute<FLinearColor> BarColor;
};
