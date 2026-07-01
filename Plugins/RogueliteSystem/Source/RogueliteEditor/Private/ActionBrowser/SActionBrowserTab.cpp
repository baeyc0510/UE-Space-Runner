#include "ActionBrowser/SActionBrowserTab.h"
#include "ActionBrowser/ActionBrowserSettings.h"
#include "ActionBrowser/SActionRowWidget.h"
#include "RogueliteActionDatabase.h"
#include "RogueliteActionData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PropertyEditorModule.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#define LOCTEXT_NAMESPACE "SActionBrowserTab"

/*~ 컬럼 ID 정의 ~*/
namespace ActionBrowserColumns
{
	const FName Status("Status");
	const FName Name("Name");
	const FName Folder("Folder");
	const FName Owned("Owned");
	const FName Weight("Weight");
	const FName Probability("Probability");
	const FName Graph("Graph");
}

void SActionBrowserTab::Construct(const FArguments& InArgs)
{
	// 설정 객체 생성 (TStrongObjectPtr가 자동으로 GC 방지)
	Settings.Reset(NewObject<UActionBrowserSettings>());

	// 데이터베이스 생성 (TStrongObjectPtr가 자동으로 GC 방지)
	Database.Reset(NewObject<URogueliteActionDatabase>());

	// 초기 정렬 설정
	CurrentSortColumn = ActionBrowserColumns::Probability;
	CurrentSortMode = EColumnSortMode::Descending;

	// UI 구성
	ChildSlot
	[
		SNew(SVerticalBox)

		// 메인 콘텐츠 영역
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// 왼쪽: Query 설정 패널
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				CreateQuerySettingsPanel()
			]

			// 중앙: Action 리스트
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				CreateActionListPanel()
			]

			// 오른쪽: Action 상세
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				CreateActionDetailsPanel()
			]
		]

		// 상태 바
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateStatusBar()
		]
	];

	// 에셋 스캔
	ScanAndRegisterAssets();
}

SActionBrowserTab::~SActionBrowserTab()
{
	// TStrongObjectPtr가 소멸 시 자동으로 GC 참조 해제
	// 수동 해제 불필요
}

TSharedRef<SWidget> SActionBrowserTab::CreateQuerySettingsPanel()
{
	// DetailsView 생성
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;

	QueryDetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	QueryDetailsView->SetObject(Settings.Get());
	QueryDetailsView->OnFinishedChangingProperties().AddSP(this, &SActionBrowserTab::OnQueryPropertyChanged);

	return SNew(SVerticalBox)

		// 헤더
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QuerySettingsHeader", "Query Settings"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// 버튼 행
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshButton", "Refresh"))
				.HAlign(HAlign_Center)
				.OnClicked(this, &SActionBrowserTab::OnRefreshClicked)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ResetButton", "Reset"))
				.HAlign(HAlign_Center)
				.OnClicked(this, &SActionBrowserTab::OnResetClicked)
			]
		]

		// DetailsView
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			QueryDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> SActionBrowserTab::CreateActionListPanel()
{
	return SNew(SVerticalBox)

		// 헤더 + 검색
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ActionListHeader", "Actions"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(SearchBox, SSearchBox)
				.HintText(LOCTEXT("SearchHint", "Search..."))
				.OnTextChanged(this, &SActionBrowserTab::OnSearchTextChanged)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked(bShowFilteredOnly ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SActionBrowserTab::OnFilterToggleChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("FilteredOnlyCheckbox", "Passed Only"))
				]
			]
		]

		// 리스트 뷰
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(ActionListView, SListView<FActionRowDataPtr>)
			.ListItemsSource(&FilteredRowData)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SActionBrowserTab::OnGenerateRow)
			.OnSelectionChanged(this, &SActionBrowserTab::OnActionSelected)
			.OnMouseButtonDoubleClick(this, &SActionBrowserTab::OnActionDoubleClicked)
			.HeaderRow
			(
				SNew(SHeaderRow)

				+ SHeaderRow::Column(ActionBrowserColumns::Status)
				.DefaultLabel(FText::GetEmpty())
				.FixedWidth(30.f)

				+ SHeaderRow::Column(ActionBrowserColumns::Name)
				.DefaultLabel(LOCTEXT("NameColumn", "Name"))
				.FillWidth(1.f)
				.SortMode(this, &SActionBrowserTab::GetColumnSortMode, ActionBrowserColumns::Name)
				.OnSort(this, &SActionBrowserTab::OnColumnSortModeChanged)

				+ SHeaderRow::Column(ActionBrowserColumns::Folder)
				.DefaultLabel(LOCTEXT("FolderColumn", "Folder"))
				.FillWidth(0.7f)
				.SortMode(this, &SActionBrowserTab::GetColumnSortMode, ActionBrowserColumns::Folder)
				.OnSort(this, &SActionBrowserTab::OnColumnSortModeChanged)

				+ SHeaderRow::Column(ActionBrowserColumns::Owned)
				.DefaultLabel(LOCTEXT("OwnedColumn", "Owned"))
				.FixedWidth(60.f)
				.SortMode(this, &SActionBrowserTab::GetColumnSortMode, ActionBrowserColumns::Owned)
				.OnSort(this, &SActionBrowserTab::OnColumnSortModeChanged)

				+ SHeaderRow::Column(ActionBrowserColumns::Weight)
				.DefaultLabel(LOCTEXT("WeightColumn", "Weight"))
				.FixedWidth(70.f)
				.SortMode(this, &SActionBrowserTab::GetColumnSortMode, ActionBrowserColumns::Weight)
				.OnSort(this, &SActionBrowserTab::OnColumnSortModeChanged)

				+ SHeaderRow::Column(ActionBrowserColumns::Probability)
				.DefaultLabel(LOCTEXT("ProbabilityColumn", "Prob %"))
				.FixedWidth(70.f)
				.SortMode(this, &SActionBrowserTab::GetColumnSortMode, ActionBrowserColumns::Probability)
				.OnSort(this, &SActionBrowserTab::OnColumnSortModeChanged)

				+ SHeaderRow::Column(ActionBrowserColumns::Graph)
				.DefaultLabel(LOCTEXT("GraphColumn", "Graph"))
				.FillWidth(0.5f)
			)
		];
}

TSharedRef<SWidget> SActionBrowserTab::CreateActionDetailsPanel()
{
	// DetailsView 생성
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;

	ActionDetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	
	return SNew(SVerticalBox)

		// 헤더
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ActionDetailsHeader", "Action Details"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// DetailsView
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			ActionDetailsView.ToSharedRef()
		];
}

TSharedRef<SWidget> SActionBrowserTab::CreateStatusBar()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(4.f)
		[
			SAssignNew(StatusText, STextBlock)
			.Text(LOCTEXT("StatusInitial", "Ready"))
		];
}

void SActionBrowserTab::ScanAndRegisterAssets()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// URogueliteActionData 및 자식 클래스 에셋 검색
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(URogueliteActionData::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	Database->UnregisterAllActions();

	for (const FAssetData& AssetData : AssetDataList)
	{
		if (URogueliteActionData* Action = Cast<URogueliteActionData>(AssetData.GetAsset()))
		{
			Database->RegisterAction(Action);
		}
	}

	RefreshActionList();
}

void SActionBrowserTab::RefreshActionList()
{
	ApplyQuery();
	ApplyFilter();
	ApplySorting();
	UpdateStatusBar();

	if (ActionListView.IsValid())
	{
		ActionListView->RequestListRefresh();
	}
}

void SActionBrowserTab::ApplyQuery()
{
	AllRowData.Empty();
	MaxProbability = 0.f;

	if (!Database || !Settings)
	{
		return;
	}

	// 쿼리 및 런 상태 생성
	const FRogueliteQuery& Query = Settings->GetQuery();
	FRogueliteRunState RunState = Settings->ToRunState();

	// 확률 계산
	TArray<FRogueliteActionProbability> Probabilities = Database->CalculateProbabilities(Query, &RunState);

	// 행 데이터 생성
	for (const FRogueliteActionProbability& ProbInfo : Probabilities)
	{
		if (!IsValid(ProbInfo.Action))
		{
			continue;
		}

		FActionRowDataPtr RowData = MakeShared<FActionRowData>();
		RowData->Action = ProbInfo.Action;
		RowData->Probability = ProbInfo.Probability;
		RowData->Weight = ProbInfo.Weight;
		RowData->bPassedFilter = ProbInfo.bPassedFilter;
		RowData->MaxStacks = ProbInfo.Action->MaxStacks;
		RowData->OwnedStacks = RunState.GetStacks(ProbInfo.Action);

		// 폴더 경로 추출 (/Game/Data/Actions/Weapons/DA_Weapon -> Weapons)
		FString PackagePath = ProbInfo.Action->GetOutermost()->GetName();
		FString FolderPath = FPaths::GetPath(PackagePath);
		RowData->Folder = FPaths::GetBaseFilename(FolderPath);

		AllRowData.Add(RowData);

		if (ProbInfo.bPassedFilter && ProbInfo.Probability > MaxProbability)
		{
			MaxProbability = ProbInfo.Probability;
		}
	}
}

void SActionBrowserTab::ApplyFilter()
{
	FilteredRowData.Empty();

	for (const FActionRowDataPtr& RowData : AllRowData)
	{
		if (!RowData.IsValid() || !RowData->Action.IsValid())
		{
			continue;
		}

		// 필터 통과만 표시 체크
		if (bShowFilteredOnly && !RowData->bPassedFilter)
		{
			continue;
		}

		// 검색어 필터
		if (!CurrentSearchText.IsEmpty())
		{
			FString DisplayName = RowData->Action->DisplayName.ToString();
			if (!DisplayName.Contains(CurrentSearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}
		}

		FilteredRowData.Add(RowData);
	}
}

void SActionBrowserTab::ApplySorting()
{
	if (CurrentSortColumn.IsNone())
	{
		return;
	}

	FilteredRowData.Sort([this](const FActionRowDataPtr& A, const FActionRowDataPtr& B) -> bool
	{
		if (!A.IsValid() || !B.IsValid())
		{
			return false;
		}

		bool bResult = false;

		if (CurrentSortColumn == ActionBrowserColumns::Name)
		{
			FString NameA = A->Action.IsValid() ? A->Action->DisplayName.ToString() : TEXT("");
			FString NameB = B->Action.IsValid() ? B->Action->DisplayName.ToString() : TEXT("");
			bResult = NameA < NameB;
		}
		else if (CurrentSortColumn == ActionBrowserColumns::Folder)
		{
			bResult = A->Folder < B->Folder;
		}
		else if (CurrentSortColumn == ActionBrowserColumns::Owned)
		{
			bResult = A->OwnedStacks < B->OwnedStacks;
		}
		else if (CurrentSortColumn == ActionBrowserColumns::Weight)
		{
			bResult = A->Weight < B->Weight;
		}
		else if (CurrentSortColumn == ActionBrowserColumns::Probability)
		{
			bResult = A->Probability < B->Probability;
		}

		return CurrentSortMode == EColumnSortMode::Ascending ? bResult : !bResult;
	});
}

void SActionBrowserTab::UpdateStatusBar()
{
	if (!StatusText.IsValid())
	{
		return;
	}

	int32 TotalCount = AllRowData.Num();
	int32 FilteredCount = 0;
	float WeightSum = 0.f;

	for (const FActionRowDataPtr& RowData : AllRowData)
	{
		if (RowData.IsValid() && RowData->bPassedFilter)
		{
			FilteredCount++;
			WeightSum += RowData->Weight;
		}
	}

	FText StatusString = FText::Format(
		LOCTEXT("StatusFormat", "Total: {0} | Passed: {1} | Weight Sum: {2} | Displayed: {3}"),
		FText::AsNumber(TotalCount),
		FText::AsNumber(FilteredCount),
		FText::AsNumber(WeightSum, &FNumberFormattingOptions::DefaultWithGrouping()),
		FText::AsNumber(FilteredRowData.Num())
	);

	StatusText->SetText(StatusString);
}

void SActionBrowserTab::OnQueryPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	RefreshActionList();
}

void SActionBrowserTab::OnActionSelected(FActionRowDataPtr Item, ESelectInfo::Type SelectInfo)
{
	if (Item.IsValid() && Item->Action.IsValid())
	{
		SelectedAction = Item->Action;
		if (ActionDetailsView.IsValid())
		{
			ActionDetailsView->SetObject(Item->Action.Get());
		}
	}
	else
	{
		SelectedAction.Reset();
		if (ActionDetailsView.IsValid())
		{
			ActionDetailsView->SetObject(nullptr);
		}
	}
}

void SActionBrowserTab::OnActionDoubleClicked(FActionRowDataPtr Item)
{
	if (Item.IsValid() && Item->Action.IsValid())
	{
		// 콘텐츠 브라우저에서 에셋 선택
		TArray<FAssetData> Assets;
		Assets.Add(FAssetData(Item->Action.Get()));

		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
	}
}

void SActionBrowserTab::OnSearchTextChanged(const FText& NewText)
{
	CurrentSearchText = NewText.ToString();
	ApplyFilter();
	ApplySorting();

	if (ActionListView.IsValid())
	{
		ActionListView->RequestListRefresh();
	}

	UpdateStatusBar();
}

void SActionBrowserTab::OnFilterToggleChanged(ECheckBoxState NewState)
{
	bShowFilteredOnly = (NewState == ECheckBoxState::Checked);
	ApplyFilter();
	ApplySorting();

	if (ActionListView.IsValid())
	{
		ActionListView->RequestListRefresh();
	}

	UpdateStatusBar();
}

TSharedRef<ITableRow> SActionBrowserTab::OnGenerateRow(FActionRowDataPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SActionRowWidget, OwnerTable)
		.RowData(Item)
		.MaxProbability(MaxProbability);
}

EColumnSortMode::Type SActionBrowserTab::GetColumnSortMode(FName ColumnId) const
{
	if (ColumnId == CurrentSortColumn)
	{
		return CurrentSortMode;
	}
	return EColumnSortMode::None;
}

void SActionBrowserTab::OnColumnSortModeChanged(EColumnSortPriority::Type SortPriority, const FName& ColumnId, EColumnSortMode::Type SortMode)
{
	CurrentSortColumn = ColumnId;
	CurrentSortMode = SortMode;

	ApplySorting();

	if (ActionListView.IsValid())
	{
		ActionListView->RequestListRefresh();
	}
}

FReply SActionBrowserTab::OnRefreshClicked()
{
	ScanAndRegisterAssets();
	return FReply::Handled();
}

FReply SActionBrowserTab::OnResetClicked()
{
	if (Settings)
	{
		Settings->ResetToDefault();
		if (QueryDetailsView.IsValid())
		{
			QueryDetailsView->ForceRefresh();
		}
		RefreshActionList();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
