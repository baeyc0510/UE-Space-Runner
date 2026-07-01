#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;
class FSpawnTabArgs;

class FRogueliteEditorModule : public IModuleInterface
{
public:
	/*~ IModuleInterface ~*/
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/*~ 메뉴/탭 등록 ~*/

	// 메뉴 확장 등록
	void RegisterMenuExtensions();

	// 메뉴 확장 해제
	void UnregisterMenuExtensions();

	// 탭 스포너 등록
	void RegisterTabSpawners();

	// 탭 스포너 해제
	void UnregisterTabSpawners();

	// Action Browser 탭 스폰
	TSharedRef<SDockTab> SpawnActionBrowserTab(const FSpawnTabArgs& Args);
};
