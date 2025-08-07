// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerMenu.generated.h"

class USpinBox;
class UMultiplayerSessionsSubsystem;
class UButton;
class UTextBlock;
class UComboBoxString;
class UStackBox;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
 * Needs to be called in blueprints
 * @param NumberOfPublicConnections 
 * @param TypeOfMatch 
 * @param LobbyLevel 
 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void MultiplayerMenuSetup(int32 NumberOfPublicConnections = 1, FString TypeOfMatch = FString(TEXT("DeathMatch")), TSoftObjectPtr<UWorld> LobbyLevel = nullptr);
	
	/**
	 * Sets up the menu with the specified parameters.
	 *
	 * @param NumberOfPublicConnections The number of public connections for the lobby.
	 * @param TypeOfMatch The type of match for the lobby.
	 * @param LobbyPath The path to the lobby.
	 */
	void MultiplayerMenuSetup(int32 NumberOfPublicConnections = 1, FString TypeOfMatch = FString(TEXT("DeathMatch")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

	
protected:
	virtual bool Initialize() override;

	/**
	 * Callback function called when a session creation attempt is completed.
	 *
	 * @param bWasSuccessful - Indicates whether the session creation was successful or not.
	 */
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnCreateSession(bool bWasSuccessful);
	/**
	 * Callback function called when finding sessions is completed.
	 *
	 * @param SessionResults The array of session search results.
	 * @param bWasSuccessful Indicates whether the session search was successful or not.
	 */
	void OnFindSessions(const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful);//these are not dynamic delegates so we don't need UFUNCTION
	
	void JoinSession(FOnlineSessionSearchResult Session);
	
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);//these are not dynamic delegates so we don't need UFUNCTION
	/**
	 * Callback function called when a session is destroyed.
	 *
	 * @param bWasSuccessful - Indicates whether the session destruction was successful or not.
	 */
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnDestroySession(bool bWasSuccessful);
	/**
	 * Called when the session starts.
	 *
	 * @param bWasSuccessful - Indicates whether the session start was successful or not.
	 */
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnStartSession(bool bWasSuccessful);
private:
	UPROPERTY(meta = (BindWidget))
	UButton * HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton * JoinButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock * JoinText;

	UPROPERTY(meta = (BindWidget))
	USpinBox * PlayerNumberInput;
	
	UPROPERTY(meta = (BindWidget))
	UComboBoxString * MapSelect;

	UPROPERTY(meta = (BindWidget))
	UStackBox * ServerList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> ListEntryWidget;

	TArray<FOnlineSessionSearchResult> SearchResults; 
	
	/**
	 * Called when the host button is clicked.
	 * Prints a debug message to the log and creates a session using the MultiplayerSessionsSubsystem.
	 */
	UFUNCTION()
	void HostButtonClicked();

	/**
	 * Called when the join button is clicked.
	 * It checks if the MultiplayerSessionsSubsystem is valid and calls the FindSessions function with a timeout of 10000 milliseconds.
	 */
	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void JoinCanceled();
	
	/**
	 * Sets up the multiplayer subsystem for the menu.
	 * This function retrieves the game instance and assigns the multiplayer sessions subsystem to the class member variable.
	 * It also binds custom delegates to their respective functions.
	 */
	void SetupMultiplayerSubsystem();

	UFUNCTION()
	void PlayerNumberValueChanged(float NewValue);

	//add multiplayer sessions subsystem
	UPROPERTY()
	UMultiplayerSessionsSubsystem * MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	int32 NumPublicConnections{1};

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	FString MatchType{TEXT("D_ShootingRange")};

	FString PathToLobby{TEXT("")};

	bool bIsJoining = false;

	// UPROPERTY()
	// UBaseSettings * Settings;
	
	void GetTopPlayers();
};
