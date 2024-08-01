#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"//ovo nebi trebali importati ovdje
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("ArenaDeathmatch")), TSoftObjectPtr<UWorld> LobbyLevel = nullptr);

	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("ArenaDeathmatch")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// callbacks for the custom delegates on MultiplayerSessionsSubsystem
	//
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful);//these are not dynamic delegates so we don't need UFUNCTION
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);//these are not dynamic delegates so we don't need UFUNCTION
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton * HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton * JoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton * LowQuality;

	UPROPERTY(meta = (BindWidget))
	class UButton * MediumQuality;

	UPROPERTY(meta = (BindWidget))
	class UButton * HighQuality;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock * JoinText;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void GraphicsQualityLowButtonClicked();
	UFUNCTION()
	void GraphicsQualityMediumButtonClicked();
	UFUNCTION()
	void GraphicsQualityHighButtonClicked();

	void GraphicsQualityUpdate(int32 QualityLevel);

	void MenuTearDown();

	void SetupWidget();
	void SetupMultiplayerSubsystem();

	//add multiplayer sessions subsystem
	class UMultiplayerSessionsSubsystem * MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	FString MatchType{TEXT("ArenaDeathmatch")};

	FString PathToLobby{TEXT("")};
};
