#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "ActionBrowser/ActionBrowserTypes.h"

/*~ Action 테이블 행 위젯 ~*/
class SActionRowWidget : public SMultiColumnTableRow<FActionRowDataPtr>
{
public:
	SLATE_BEGIN_ARGS(SActionRowWidget)
		: _MaxProbability(1.f)
	{}
		SLATE_ARGUMENT(FActionRowDataPtr, RowData)
		SLATE_ARGUMENT(float, MaxProbability)
	SLATE_END_ARGS()

	// 위젯 생성
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	// 컬럼별 위젯 생성
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	// 상태 아이콘 생성
	TSharedRef<SWidget> CreateStatusIcon();

	// 이름 위젯 생성
	TSharedRef<SWidget> CreateNameWidget();

	// 폴더 위젯 생성
	TSharedRef<SWidget> CreateFolderWidget();

	// 보유 수 위젯 생성
	TSharedRef<SWidget> CreateOwnedWidget();

	// 가중치 위젯 생성
	TSharedRef<SWidget> CreateWeightWidget();

	// 확률 위젯 생성
	TSharedRef<SWidget> CreateProbabilityWidget();

	// 확률 그래프 생성
	TSharedRef<SWidget> CreateProbabilityGraph();

	// 확률에 따른 색상 계산
	FLinearColor GetProbabilityColor() const;

private:
	// 행 데이터
	FActionRowDataPtr RowData;

	// 최대 확률 (그래프 정규화용)
	float MaxProbability = 1.f;
};
