// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"//ovo nebi trebali importati ovdje
#include "FindSessionsCallbackProxy.h"
#include "ListViewEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UListViewEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnlineSessionSearchResult Session;

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton * JoinButton;

	UFUNCTION()
	void JoinGame();
};
