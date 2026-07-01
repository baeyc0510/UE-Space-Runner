#include "RogueliteEditor.h"
#include "ActionBrowser/SActionBrowserTab.h"
#include "ToolMenus.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FRogueliteEditorModule"

static const FName ActionBrowserTabId("RogueliteActionBrowser");

void FRogueliteEditorModule::StartupModule()
{
	RegisterTabSpawners();
	RegisterMenuExtensions();
}

void FRogueliteEditorModule::ShutdownModule()
{
	UnregisterMenuExtensions();
	UnregisterTabSpawners();
}

void FRogueliteEditorModule::RegisterMenuExtensions()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		if (IsValid(Menu))
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("Roguelite");
			Section.Label = LOCTEXT("RogueliteMenuLabel", "Roguelite");

			Section.AddMenuEntry(
				"ActionBrowser",
				LOCTEXT("ActionBrowserMenuEntry", "Action Browser"),
				LOCTEXT("ActionBrowserMenuTooltip", "로그라이트 액션 브라우저를 엽니다."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(ActionBrowserTabId);
				}))
			);
		}
	}));
}

void FRogueliteEditorModule::UnregisterMenuExtensions()
{
	UToolMenus::UnregisterOwner(this);
}

void FRogueliteEditorModule::RegisterTabSpawners()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ActionBrowserTabId,
		FOnSpawnTab::CreateRaw(this, &FRogueliteEditorModule::SpawnActionBrowserTab))
		.SetDisplayName(LOCTEXT("ActionBrowserTabTitle", "Action Browser"))
		.SetTooltipText(LOCTEXT("ActionBrowserTabTooltip", "로그라이트 액션 데이터를 조회하고 쿼리 확률을 시뮬레이션합니다."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FRogueliteEditorModule::UnregisterTabSpawners()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ActionBrowserTabId);
}

TSharedRef<SDockTab> FRogueliteEditorModule::SpawnActionBrowserTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SActionBrowserTab)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRogueliteEditorModule, RogueliteEditor)