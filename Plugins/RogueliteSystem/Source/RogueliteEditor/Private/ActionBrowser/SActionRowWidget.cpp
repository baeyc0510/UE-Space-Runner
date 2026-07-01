#include "ActionBrowser/SActionRowWidget.h"
#include "ActionBrowser/SActionBrowserTab.h"
#include "ActionBrowser/SProbabilityBar.h"
#include "RogueliteActionData.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SActionRowWidget"

void SActionRowWidget::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	RowData = InArgs._RowData;
	MaxProbability = InArgs._MaxProbability;

	SMultiColumnTableRow<FActionRowDataPtr>::Construct(
		FSuperRowType::FArguments(),
		InOwnerTableView
	);
}

TSharedRef<SWidget> SActionRowWidget::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == ActionBrowserColumns::Status)
	{
		return CreateStatusIcon();
	}
	else if (ColumnName == ActionBrowserColumns::Name)
	{
		return CreateNameWidget();
	}
	else if (ColumnName == ActionBrowserColumns::Folder)
	{
		return CreateFolderWidget();
	}
	else if (ColumnName == ActionBrowserColumns::Owned)
	{
		return CreateOwnedWidget();
	}
	else if (ColumnName == ActionBrowserColumns::Weight)
	{
		return CreateWeightWidget();
	}
	else if (ColumnName == ActionBrowserColumns::Probability)
	{
		return CreateProbabilityWidget();
	}
	else if (ColumnName == ActionBrowserColumns::Graph)
	{
		return CreateProbabilityGraph();
	}

	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> SActionRowWidget::CreateStatusIcon()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	// 필터 통과 여부에 따른 아이콘
	const FSlateBrush* IconBrush = RowData->bPassedFilter
		? FAppStyle::GetBrush("Icons.Check")
		: FAppStyle::GetBrush("Icons.X");

	FLinearColor IconColor = RowData->bPassedFilter
		? FLinearColor::Green
		: FLinearColor(0.5f, 0.5f, 0.5f);

	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.WidthOverride(16.f)
		.HeightOverride(16.f)
		[
			SNew(SImage)
			.Image(IconBrush)
			.ColorAndOpacity(IconColor)
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateNameWidget()
{
	if (!RowData.IsValid() || !RowData->Action.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FLinearColor TextColor = RowData->bPassedFilter
		? FLinearColor::White
		: FLinearColor(0.5f, 0.5f, 0.5f);

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(STextBlock)
			.Text(RowData->Action->DisplayName)
			.ColorAndOpacity(TextColor)
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateFolderWidget()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FLinearColor TextColor = RowData->bPassedFilter
		? FLinearColor(0.7f, 0.7f, 0.7f)
		: FLinearColor(0.4f, 0.4f, 0.4f);

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(RowData->Folder))
			.ColorAndOpacity(TextColor)
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateOwnedWidget()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FText OwnedText;
	if (RowData->MaxStacks > 0)
	{
		OwnedText = FText::Format(
			LOCTEXT("OwnedFormat", "{0}/{1}"),
			FText::AsNumber(RowData->OwnedStacks),
			FText::AsNumber(RowData->MaxStacks)
		);
	}
	else
	{
		OwnedText = FText::AsNumber(RowData->OwnedStacks);
	}

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(OwnedText)
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateWeightWidget()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(STextBlock)
			.Text(FText::AsNumber(RowData->Weight, &FNumberFormattingOptions::DefaultWithGrouping()))
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateProbabilityWidget()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	float ProbPercent = RowData->Probability * 100.f;
	FText ProbText = FText::Format(LOCTEXT("ProbFormat", "{0}%"), FText::AsNumber(ProbPercent, &FNumberFormattingOptions().SetMaximumFractionalDigits(2)));

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(STextBlock)
			.Text(ProbText)
			.ColorAndOpacity(GetProbabilityColor())
		];
}

TSharedRef<SWidget> SActionRowWidget::CreateProbabilityGraph()
{
	if (!RowData.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	// 정규화된 확률
	float NormalizedProb = MaxProbability > 0.f ? RowData->Probability / MaxProbability : 0.f;

	return SNew(SBox)
		.VAlign(VAlign_Center)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(SProbabilityBar)
			.Probability(NormalizedProb)
			.BarColor(GetProbabilityColor())
		];
}

FLinearColor SActionRowWidget::GetProbabilityColor() const
{
	if (!RowData.IsValid() || !RowData->bPassedFilter)
	{
		return FLinearColor(0.3f, 0.3f, 0.3f);
	}

	// 정규화된 확률
	float NormalizedProb = MaxProbability > 0.f ? RowData->Probability / MaxProbability : 0.f;

	// 녹색 → 노랑 → 빨강 그라데이션
	if (NormalizedProb > 0.5f)
	{
		float T = (NormalizedProb - 0.5f) * 2.f;
		// 노랑(#FFC107) → 녹색(#4CAF50)
		return FMath::Lerp(
			FLinearColor(1.f, 0.76f, 0.03f, 1.f),
			FLinearColor(0.3f, 0.69f, 0.31f, 1.f),
			T
		);
	}
	else
	{
		float T = NormalizedProb * 2.f;
		// 빨강(#F44336) → 노랑(#FFC107)
		return FMath::Lerp(
			FLinearColor(0.96f, 0.26f, 0.21f, 1.f),
			FLinearColor(1.f, 0.76f, 0.03f, 1.f),
			T
		);
	}
}

#undef LOCTEXT_NAMESPACE
