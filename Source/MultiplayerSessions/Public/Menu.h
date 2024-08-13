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
	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), TSoftObjectPtr<UWorld> LobbyLevel = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SaveGraphicsSettings();

	void MenuSetup(int32 NumberOfPublicConnections = 2, FString TypeOfMatch = FString(TEXT("DeathMatch")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// callbacks for the custom delegates on MultiplayerSessionsSubsystem
	//
	UFUNCTION()//this needs to be a UFUNCTION() so that we can bind it to the delegate
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful);//these are not dynamic delegates so we don't need UFUNCTION
	
	void JoinSession(FOnlineSessionSearchResult Session);
	
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

	UFUNCTION()
	void HostButtonClicked();

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

	void MenuTearDown();

	void SetupWidget();
	void SetupMultiplayerSubsystem();

	//add multiplayer sessions subsystem
	class UMultiplayerSessionsSubsystem * MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	int32 NumPublicConnections{1};

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	FString MatchType{TEXT("D_ShootingRange")};

	FString PathToLobby{TEXT("")};

	bool bIsJoining = false;

	void GetTopPlayers();
};
