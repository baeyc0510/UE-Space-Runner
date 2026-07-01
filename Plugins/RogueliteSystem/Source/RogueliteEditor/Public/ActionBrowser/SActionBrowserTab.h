#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/SHeaderRow.h"
#include "UObject/StrongObjectPtr.h"
#include "ActionBrowserTypes.h"
#include "ActionBrowserSettings.h"
#include "RogueliteActionDatabase.h"

class URogueliteActionData;
class IDetailsView;
class SSearchBox;

/*~ 컬럼 ID ~*/
namespace ActionBrowserColumns
{
	extern const FName Status;
	extern const FName Name;
	extern const FName Folder;
	extern const FName Owned;
	extern const FName Weight;
	extern const FName Probability;
	extern const FName Graph;
}

/*~ Action Browser 메인 탭 위젯 ~*/
class ROGUELITEEDITOR_API SActionBrowserTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SActionBrowserTab) {}
	SLATE_END_ARGS()

	// 위젯 생성
	void Construct(const FArguments& InArgs);

	// 소멸자
	virtual ~SActionBrowserTab();

private:
	/*~ UI 생성 ~*/

	// Query 설정 패널 생성
	TSharedRef<SWidget> CreateQuerySettingsPanel();

	// Action 리스트 패널 생성
	TSharedRef<SWidget> CreateActionListPanel();

	// Action 상세 패널 생성
	TSharedRef<SWidget> CreateActionDetailsPanel();

	// 상태 바 생성
	TSharedRef<SWidget> CreateStatusBar();

	/*~ 데이터 관리 ~*/

	// 에셋 스캔 및 등록
	void ScanAndRegisterAssets();

	// Action 리스트 갱신
	void RefreshActionList();

	// 쿼리 적용
	void ApplyQuery();

	// 필터링 적용
	void ApplyFilter();

	// 정렬 적용
	void ApplySorting();

	// 상태 바 갱신
	void UpdateStatusBar();

	/*~ 이벤트 핸들러 ~*/

	// Query 프로퍼티 변경
	void OnQueryPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
	
	// Action 선택
	void OnActionSelected(FActionRowDataPtr Item, ESelectInfo::Type SelectInfo);

	// Action 더블클릭
	void OnActionDoubleClicked(FActionRowDataPtr Item);

	// 검색어 변경
	void OnSearchTextChanged(const FText& NewText);

	// 필터 토글 변경
	void OnFilterToggleChanged(ECheckBoxState NewState);

	// 리스트 행 생성
	TSharedRef<ITableRow> OnGenerateRow(FActionRowDataPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	// 컬럼 정렬 모드 조회
	EColumnSortMode::Type GetColumnSortMode(FName ColumnId) const;

	// 컬럼 정렬 변경
	void OnColumnSortModeChanged(EColumnSortPriority::Type SortPriority, const FName& ColumnId, EColumnSortMode::Type SortMode);

	// 리프레시 버튼
	FReply OnRefreshClicked();

	// 리셋 버튼
	FReply OnResetClicked();

private:
	/*~ Data ~*/

	// 설정 객체 (TStrongObjectPtr로 GC 자동 관리)
	TStrongObjectPtr<UActionBrowserSettings> Settings;

	// Action 데이터베이스 (TStrongObjectPtr로 GC 자동 관리)
	TStrongObjectPtr<URogueliteActionDatabase> Database;

	// 전체 Action 행 데이터
	TArray<FActionRowDataPtr> AllRowData;

	// 필터링된 Action 행 데이터
	TArray<FActionRowDataPtr> FilteredRowData;

	// 현재 검색어
	FString CurrentSearchText;

	// 필터 통과만 표시
	bool bShowFilteredOnly = false;

	// 현재 정렬 컬럼
	FName CurrentSortColumn;
	EColumnSortMode::Type CurrentSortMode = EColumnSortMode::Descending;

	// 최대 확률 (그래프 정규화용)
	float MaxProbability = 0.f;

	/*~ Widgets ~*/

	// Query 설정 DetailsView
	TSharedPtr<IDetailsView> QueryDetailsView;

	// Action 상세 DetailsView
	TSharedPtr<IDetailsView> ActionDetailsView;

	// Action 리스트 뷰
	TSharedPtr<SListView<FActionRowDataPtr>> ActionListView;

	// 검색 박스
	TSharedPtr<SSearchBox> SearchBox;

	// 상태 텍스트
	TSharedPtr<STextBlock> StatusText;

	// 현재 선택된 Action
	TWeakObjectPtr<URogueliteActionData> SelectedAction;
};
