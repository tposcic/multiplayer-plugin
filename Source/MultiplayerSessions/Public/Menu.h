// Copyright: Toni Poscic

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UBaseSettings;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), TSoftObjectPtr<UWorld> LobbyLevel = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SaveGraphicsSettings();

	/**
	 * Sets up the menu with the specified parameters.
	 *
	 * @param NumberOfPublicConnections The number of public connections for the lobby.
	 * @param TypeOfMatch The type of match for the lobby.
	 * @param LobbyPath The path to the lobby.
	 */
	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

	UBaseSettings * GetSettings();
protected:
	/**
	 * Initializes the menu.
	 * This function is called when the menu is being initialized.
	 * It sets up the button click events for the Host and Join buttons.
	 *
	 * @return True if the initialization was successful, false otherwise.
	 */
	virtual bool Initialize() override;
	/**
	 * Called when the menu is being destroyed.
	 * Performs any necessary cleanup before the menu is destroyed.
	 */
	virtual void NativeDestruct() override;

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
	class UButton * MadQuality;

	UPROPERTY(meta = (BindWidget))
	class UButton * SaveGraphicsButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock * JoinText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock * VersionText;

	UPROPERTY(meta = (BindWidget))
	class USlider * MouseSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	class USlider * GlobalVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString * ResolutionSelect;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString * MapSelect;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString * FullScreenModeSelect;

	UPROPERTY(meta = (BindWidget))
	class UStackBox * ServerList;

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

	UFUNCTION()
	void GraphicsQualityLowButtonClicked();

	UFUNCTION()
	void GraphicsQualityMediumButtonClicked();

	UFUNCTION()
	void GraphicsQualityHighButtonClicked();

	UFUNCTION()
	void GraphicsQualityMadButtonClicked();

	UFUNCTION()
	void SaveGraphicsButtonClicked();

	void GraphicsQualityUpdate(int32 QualityLevel);
	
	/**
	 * Tears down the menu by removing it from the parent widget and resetting the input mode.
	 * This function is called when the menu is being closed.
	 */
	void MenuTearDown();

	/**
	 * Sets up the widget for the menu.
	 * Adds the widget to the viewport, sets its visibility to visible, and makes it focusable.
	 * Sets the input mode to UI only, locks the mouse to the viewport, and shows the mouse cursor.
	 */
	void SetupWidget();
	/**
	 * Sets up the multiplayer subsystem for the menu.
	 * This function retrieves the game instance and assigns the multiplayer sessions subsystem to the class member variable.
	 * It also binds custom delegates to their respective functions.
	 */
	void SetupMultiplayerSubsystem();

	//add multiplayer sessions subsystem
	class UMultiplayerSessionsSubsystem * MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	int32 NumPublicConnections{1};

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	FString MatchType{TEXT("D_ShootingRange")};

	FString PathToLobby{TEXT("")};

	bool bIsJoining = false;

	UPROPERTY()
	UBaseSettings * Settings;
	
	void GetTopPlayers();
};
