// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectPoolSubsystem.h"
#include "ObjectPoolDeveloperSettings.generated.h"

/**
 * 오브젝트 풀 시스템 프로젝트 설정
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Object Pool"))
class OBJECTPOOLINGSYSTEM_API UObjectPoolDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/*~ UDeveloperSettings Interface ~*/
	virtual FName GetCategoryName() const override { return FName("Game"); }
	virtual FName GetSectionName() const override { return FName("Object Pool"); }

	/*~ Settings Access ~*/

	// 싱글톤 접근
	static const UObjectPoolDeveloperSettings* Get();

	/*~ Pool Settings ~*/

	// 전역 기본 풀 설정
	UPROPERTY(Config, EditAnywhere, Category = "Default Settings")
	FPoolSettings GlobalDefaultSettings;
};
