#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RogueliteSettings.generated.h"

/**
 * 로그라이트 시스템 프로젝트 설정.
 * Project Settings > Plugins > Roguelite System 에서 접근.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Roguelite System"))
class ROGUELITECORE_API URogueliteSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URogueliteSettings();

	/*~ UDeveloperSettings Interface ~*/
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName() const override { return TEXT("Roguelite System"); }

	/*~ Static Access ~*/

	// 설정 인스턴스 획득
	static const URogueliteSettings* Get();

public:
	/*~ Auto Registration (추후 구현) ~*/

	// 게임 시작 시 자동 등록할 폴더 경로
	// UPROPERTY(Config, EditAnywhere, Category = "Auto Registration")
	// TArray<FDirectoryPath> AutoRegisterPaths;

	/*~ Debug ~*/

	// 디버그 로깅 활성화
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	bool bEnableDebugLogging = false;
};
