#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActionBrowserTypes.h"
#include "ActionBrowserSettings.generated.h"

/*~ Action Browser 설정 객체 (IDetailsView 바인딩용) ~*/
UCLASS()
class ROGUELITEEDITOR_API UActionBrowserSettings : public UObject
{
	GENERATED_BODY()

public:
	// 쿼리 설정
	UPROPERTY(EditAnywhere, Category = "Action Browser")
	FActionBrowserQuery Query;

public:
	// FRogueliteQuery 참조 반환
	const FRogueliteQuery& GetQuery() const;

	// FRogueliteRunState로 변환 (시뮬레이션 상태)
	FRogueliteRunState ToRunState() const;

	// 설정 초기화
	void ResetToDefault();
};
