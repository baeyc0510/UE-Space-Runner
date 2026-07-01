// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPoolDeveloperSettings.h"

const UObjectPoolDeveloperSettings* UObjectPoolDeveloperSettings::Get()
{
	return GetDefault<UObjectPoolDeveloperSettings>();
}
