#include "Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "DebugHelper.h"
// #include "GameFramework/GameUserSettings.h"
#include "DeathEcho/Settings/DESettings.h"
//include master sound class
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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
    SetIsFocusable(true); // previously bIsFocusable = true; -> now deprecated

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

    if(MultiplayerSessionsSubsystem)//bind the custom delegates to the functions
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

    if(SaveGraphicsButton)
    {
        SaveGraphicsButton->OnClicked.AddDynamic(this, &UMenu::SaveGraphicsButtonClicked);
    }

    if(LowQuality)
    {
        LowQuality->OnClicked.AddDynamic(this, &UMenu::GraphicsQualityLowButtonClicked);
    }

    if(MediumQuality)
    {
        MediumQuality->OnClicked.AddDynamic(this, &UMenu::GraphicsQualityMediumButtonClicked);
    }

    if(HighQuality)
    {
        HighQuality->OnClicked.AddDynamic(this, &UMenu::GraphicsQualityHighButtonClicked);
    }

    if(MadQuality)
    {
        MadQuality->OnClicked.AddDynamic(this, &UMenu::GraphicsQualityMadButtonClicked);
    }

    UDESettings * Settings = Cast<UDESettings>(UDESettings::GetGameUserSettings());

    if(Settings)
    {
        if(GlobalVolumeSlider)
        {
            float MasterVolume = Settings->GetMasterSoundVolume();
            GlobalVolumeSlider->SetValue(MasterVolume);
            
            USoundClass* MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master"));
            if (MasterSoundClass)
            {
                MasterSoundClass->Properties.Volume = MasterVolume;
            }
        }

        if(MouseSensitivitySlider)
        {
            MouseSensitivitySlider->SetValue(Settings->GetMouseSensitivity());
        }

        if(VersionText)
        {
            VersionText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Settings->GetGameVersion())));
        }

        if(ResolutionSelect)
        {
            TArray<FIntPoint> SupportedResolutions;

            UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);

            for(FIntPoint Resolution : SupportedResolutions)
            {
                ResolutionSelect->AddOption(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
            }

            FIntPoint CurrentScreenResolution = Settings->GetScreenResolution();
            FString CurrentScreenResolutionString = FString::Printf(TEXT("%dx%d"), CurrentScreenResolution.X, CurrentScreenResolution.Y);

            ResolutionSelect->SetSelectedOption(CurrentScreenResolutionString);
        }

        if(FullScreenModeSelect)
        {
            EWindowMode::Type WindowMode = Settings->GetFullscreenMode();
            int32 FullScreenMode = 0;

            switch (WindowMode)
            {
                case EWindowMode::Fullscreen:
                    FullScreenMode = 0;
                    break;
                case EWindowMode::WindowedFullscreen:
                    FullScreenMode = 1;
                    break;
                case EWindowMode::Windowed:
                    FullScreenMode = 2;
                    break;
                default:
                    FullScreenMode = 2;
                    break;
            }

            FullScreenModeSelect->SetSelectedIndex(FullScreenMode);
        }
    }

    return true;
}

void UMenu::SaveGraphicsButtonClicked()
{
    SaveGraphicsSettings();
}

void UMenu::GraphicsQualityLowButtonClicked()
{
    GraphicsQualityUpdate(0);
}

void UMenu::GraphicsQualityMediumButtonClicked()
{
    GraphicsQualityUpdate(1);
}

void UMenu::GraphicsQualityHighButtonClicked()
{
    GraphicsQualityUpdate(2);
}

void UMenu::GraphicsQualityMadButtonClicked()
{
    GraphicsQualityUpdate(3);
}


void UMenu::GraphicsQualityUpdate(int32 QualityLevel)
{
    UDESettings * Settings = Cast<UDESettings>(UDESettings::GetGameUserSettings());

    if(Settings)
    {
        DebugHelper::PrintToLog(FString::Printf(TEXT("Current Resolution: %d x %d"), Settings->GetScreenResolution().X, Settings->GetScreenResolution().Y), FColor::Green);

        Settings->SetOverallScalabilityLevel(QualityLevel);

        DebugHelper::PrintToLog(FString::Printf(TEXT("Current Overall Scalability Level: %d"), Settings->GetOverallScalabilityLevel()), FColor::Green);

        Settings->ApplySettings(false);
    }
    else
    {
        DebugHelper::PrintToLog("Game User Settings is null", FColor::Red);
    }
}

void UMenu::SaveGraphicsSettings()
{
    UDESettings * Settings = Cast<UDESettings>(UDESettings::GetGameUserSettings());

    if(Settings)
    {
        if(MouseSensitivitySlider)
        {
            UE_LOG(LogTemp, Display, TEXT("Current Mouse Sensitivity: %f"), MouseSensitivitySlider->GetValue());
            Settings->SetMouseSensitivity(MouseSensitivitySlider->GetValue());
        }

        if(GlobalVolumeSlider)
        {
            UE_LOG(LogTemp, Display, TEXT("Current Global Volume: %f"), GlobalVolumeSlider->GetValue());
            Settings->SetMasterSoundVolume(GlobalVolumeSlider->GetValue());

            USoundClass* MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master"));

            if (MasterSoundClass)
            {
                MasterSoundClass->Properties.Volume = GlobalVolumeSlider->GetValue();
            }
        }
    
        if(VersionText)
        {
            VersionText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Settings->GetGameVersion())));
        }

        if(ResolutionSelect)
        {
            FString Resolution = ResolutionSelect->GetSelectedOption();

            TArray<FString> ResolutionArray;
            Resolution.ParseIntoArray(ResolutionArray, TEXT("x"), true);

            if(ResolutionArray.Num() == 2)
            {
                int32 Width = FCString::Atoi(*ResolutionArray[0]);
                int32 Height = FCString::Atoi(*ResolutionArray[1]);

                if(Width > 0 && Height > 0)//if the Atoi string conversion fails one of them will be 0 
                {
                    Settings->SetScreenResolution(FIntPoint(Width, Height));
                }
            }
        }

        if(FullScreenModeSelect)
        {
            int32 WindowMode = FullScreenModeSelect->GetSelectedIndex();
            EWindowMode::Type FullScreenMode = EWindowMode::ConvertIntToWindowMode(WindowMode);

            Settings->SetFullscreenMode(FullScreenMode);
        }

        Settings->ApplySettings(false);
    }
}

/**
 * Called when the host button is clicked.
 * Prints a debug message to the log and creates a session using the MultiplayerSessionsSubsystem.
 */
void UMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);
    JoinButton->SetIsEnabled(false);
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
    // JoinButton->SetIsEnabled(false);
    if(bIsJoining)
    {
        JoinCanceled();
        return;
    }

    HostButton->SetIsEnabled(false);
    JoinText->SetText(FText::FromString("Cancel"));

    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10);//The size of the search results shouldn't really matter.  Epic's documentation for BuildUniqueId is Used to keep different builds from seeing each other during searches
    }

    bIsJoining = true;
}

void UMenu::JoinCanceled()
{
    bIsJoining = false;

    DebugHelper::PrintToLog("Canceled join", FColor::Red);

    HostButton->SetIsEnabled(true);
    JoinButton->SetIsEnabled(true);
    JoinText->SetText(FText::FromString("Join"));
}

/**
 * Tears down the menu by removing it from the parent widget and resetting the input mode.
 * This function is called when the menu is being closed.
 */
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

        MultiplayerSessionsSubsystem->StartSession();
    }
    else
    {
        DebugHelper::PrintToLog("Session Creation Failed", FColor::Red);

        HostButton->SetIsEnabled(true);
        JoinButton->SetIsEnabled(true);
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
        if(!bIsJoining) return;

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
        // JoinButton->SetIsEnabled(true);
        HostButton->SetIsEnabled(true);
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
        // JoinButton->SetIsEnabled(true);
        HostButton->SetIsEnabled(true);
        JoinText->SetText(FText::FromString("Join"));
    }

    bIsJoining = false;
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