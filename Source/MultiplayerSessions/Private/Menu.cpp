#include "Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "DebugHelper.h"

/**
 * Sets up the menu with the specified parameters. OVERLOADED
 *
 * @param NumberOfPublicConnections The number of public connections for the multiplayer session.
 * @param TypeOfMatch The type of match for the multiplayer session.
 * @param LobbyLevel The lobby level for the multiplayer session.
 */
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, TSoftObjectPtr<UWorld> LobbyLevel)
{
    PathToLobby = LobbyLevel.GetLongPackageName().Append("?listen");
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    SetupWidget();
    SetupMultiplayerSubsystem();
}

/**
 * Sets up the menu with the specified parameters.
 *
 * @param NumberOfPublicConnections The number of public connections for the lobby.
 * @param TypeOfMatch The type of match for the lobby.
 * @param LobbyPath The path to the lobby.
 */
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    SetupWidget();
    SetupMultiplayerSubsystem();
}

/**
 * Sets up the widget for the menu.
 * Adds the widget to the viewport, sets its visibility to visible, and makes it focusable.
 * Sets the input mode to UI only, locks the mouse to the viewport, and shows the mouse cursor.
 */
void UMenu::SetupWidget()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);
    // bIsFocusable = true; //deprecated

    UWorld * World = GetWorld();

    if(World)
    {
        APlayerController * PlayerController = World->GetFirstPlayerController();

        if(PlayerController)
        {
            FInputModeUIOnly InputModeData;//we set the input mode to UI only so that we don't apply input to any pawns in the world and just focus on the UI
            InputModeData.SetWidgetToFocus(TakeWidget());//we set the widget to focus to the widget we are using
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//we set the mouse lock behavior to not lock to the viewport (kao windowed mode)
            PlayerController->SetInputMode(InputModeData);//we set the input mode to the player controller
            PlayerController->SetShowMouseCursor(true);//we show the mouse cursor
        }
    }
}

/**
 * Sets up the multiplayer subsystem for the menu.
 * This function retrieves the game instance and assigns the multiplayer sessions subsystem to the class member variable.
 * It also binds custom delegates to their respective functions.
 */
void UMenu::SetupMultiplayerSubsystem()
{
    UGameInstance * GameInstance = GetGameInstance();

    if(GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    //bind the custom delegates to the functions
    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);//moze i &ThisClass
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnStartSession);
    }
}

/**
 * Initializes the menu.
 * This function is called when the menu is being initialized.
 * It sets up the button click events for the Host and Join buttons.
 *
 * @return True if the initialization was successful, false otherwise.
 */
bool UMenu::Initialize()
{
    if(!Super::Initialize()) return false;

    if(HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
    }

    if(JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
    }

    return true;
}

/**
 * Called when the host button is clicked.
 * Prints a debug message to the log and creates a session using the MultiplayerSessionsSubsystem.
 */
void UMenu::HostButtonClicked()
{
    DebugHelper::PrintToLog("Host Button Clicked", FColor::Yellow);

    HostButton->SetIsEnabled(false);
    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
    }
}

/**
 * Called when the join button is clicked.
 * It checks if the MultiplayerSessionsSubsystem is valid and calls the FindSessions function with a timeout of 10000 milliseconds.
 */
void UMenu::JoinButtonClicked()
{
    JoinButton->SetIsEnabled(false);
    JoinText->SetText(FText::FromString("Searching..."));

    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000);
    }
}


//this function is called when the server is hosted, map is travelled to and the lobby is created
void UMenu::MenuTearDown()
{
    RemoveFromParent();

    UWorld * World = GetWorld();

    if(World)
    {
        APlayerController * PlayerController = World->GetFirstPlayerController();

        if(PlayerController)
        {
            FInputModeGameOnly InputModeData;//we set the input mode to game only so that we don't apply input to the UI and just focus on the pawns in the world
            PlayerController->SetInputMode(InputModeData);//we set the input mode to the player controller
            PlayerController->SetShowMouseCursor(false);//we hide the mouse cursor
        }
    }
}

/**
 * Called when the menu is being destroyed.
 * Performs any necessary cleanup before the menu is destroyed.
 */
void UMenu::NativeDestruct()
{
	MenuTearDown();
 
	Super::NativeDestruct();
}

/**
 * Callback function called when a session creation attempt is completed.
 *
 * @param bWasSuccessful - Indicates whether the session creation was successful or not.
 */
void UMenu::OnCreateSession(bool bWasSuccessful)
{
    if(bWasSuccessful)
    {
        DebugHelper::PrintToLog("Session Created Successfully", FColor::Green);

		// UWorld * World = GetWorld();

		// if(World)
		// {
		// 	// World->ServerTravel("/Game/Maps/Arena_2?listen");//this should be the lobby level
		// 	World->ServerTravel("/Game/Maps/Lobby?listen");
		// }

        MultiplayerSessionsSubsystem->StartSession();
    }
    else
    {
        DebugHelper::PrintToLog("Session Creation Failed", FColor::Red);

        HostButton->SetIsEnabled(true);
    }
}

/**
 * Callback function called when finding sessions is completed.
 *
 * @param SessionResults The array of session search results.
 * @param bWasSuccessful Indicates whether the session search was successful or not.
 */
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful)
{
    if(MultiplayerSessionsSubsystem == nullptr) return;

	for (auto Result : SessionResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString UserName = Result.Session.OwningUserName;
		FString SettingsValue;

		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if(SettingsValue == MatchType)
		{
            DebugHelper::PrintToLog(FString::Printf(TEXT("Found session %s - %s. Joining!"), *Id, *UserName), FColor::Green);

			MultiplayerSessionsSubsystem->JoinSession(Result);//add the join session complete delegate

            return;// short circuit the function so that we don't join multiple sessions at once
		}
	}

    if(!bWasSuccessful)
    {
        JoinButton->SetIsEnabled(true);
        JoinText->SetText(FText::FromString("Join"));
    }
}

/**
 * Callback function called when joining a session is complete.
 *
 * @param Result The result of the join session operation.
 */
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem * Subsystem = IOnlineSubsystem::Get();

    if(Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

        if(SessionInterface.IsValid())
        {
            FString Address;
            SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

            APlayerController * PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

            if(PlayerController)
            {
                PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            }        
        }
    }

    if(Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
        JoinText->SetText(FText::FromString("Join"));
    }
}

/**
 * Callback function called when a session is destroyed.
 *
 * @param bWasSuccessful - Indicates whether the session destruction was successful or not.
 */
void UMenu::OnDestroySession(bool bWasSuccessful)
{
    bWasSuccessful ? DebugHelper::PrintToLog("Session Destroyed Successfully", FColor::Green) : DebugHelper::PrintToLog("Session Destroy Failed", FColor::Red);
}

/**
 * Called when the session starts.
 *
 * @param bWasSuccessful - Indicates whether the session start was successful or not.
 */
void UMenu::OnStartSession(bool bWasSuccessful)
{
    bWasSuccessful ? DebugHelper::PrintToLog("Session Started Successfully", FColor::Green) : DebugHelper::PrintToLog("Session Start Failed", FColor::Red);

    UWorld * World = GetWorld();

    if(World)
    {
        // World->ServerTravel("/Game/Maps/Arena_2?listen");//this should be the lobby level
        World->ServerTravel(PathToLobby);
    }
}