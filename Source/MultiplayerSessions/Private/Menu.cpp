// Copyright: Toni Poscic

#include "Menu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/StackBox.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "DebugHelper.h"
#include "Combustion_Vertical/Settings/BaseSettings.h"
#include "Sound/SoundClass.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ListViewEntryWidget.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, TSoftObjectPtr<UWorld> LobbyLevel)
{
    PathToLobby = LobbyLevel.GetLongPackageName().Append("?listen");
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    SetupWidget();
    SetupMultiplayerSubsystem();
}

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
    NumPublicConnections = NumberOfPublicConnections;
    MatchType = TypeOfMatch;

    SetupWidget();
    SetupMultiplayerSubsystem();
}

UBaseSettings * UMenu::GetSettings()
{
    if (Settings == nullptr)
    {
        Settings = Cast<UBaseSettings>(UBaseSettings::GetGameUserSettings());
    }

    return Settings;
}

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
    
    if(GetSettings())
    {
        if(GlobalVolumeSlider)
        {
            float MasterVolume = GetSettings()->GetMasterSoundVolume();
            GlobalVolumeSlider->SetValue(MasterVolume);
            
            USoundClass* MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master"));
            if (MasterSoundClass)
            {
                MasterSoundClass->Properties.Volume = MasterVolume;
            }
        }

        if(MouseSensitivitySlider)
        {
            MouseSensitivitySlider->SetValue(GetSettings()->GetMouseSensitivity());
        }

        if(VersionText)
        {
            VersionText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GetSettings()->GetGameVersion())));
        }

        if(ResolutionSelect)
        {
            TArray<FIntPoint> SupportedResolutions;

            UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions);

            for(FIntPoint Resolution : SupportedResolutions)
            {
                ResolutionSelect->AddOption(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
            }

            FIntPoint CurrentScreenResolution = GetSettings()->GetScreenResolution();
            FString CurrentScreenResolutionString = FString::Printf(TEXT("%dx%d"), CurrentScreenResolution.X, CurrentScreenResolution.Y);

            ResolutionSelect->SetSelectedOption(CurrentScreenResolutionString);
        }

        if(FullScreenModeSelect)
        {
            EWindowMode::Type WindowMode = GetSettings()->GetFullscreenMode();
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
    if(GetSettings())
    {
        DebugHelper::PrintToLog(FString::Printf(TEXT("Current Resolution: %d x %d"), GetSettings()->GetScreenResolution().X, GetSettings()->GetScreenResolution().Y), FColor::Green);

        GetSettings()->SetOverallScalabilityLevel(QualityLevel);

        DebugHelper::PrintToLog(FString::Printf(TEXT("Current Overall Scalability Level: %d"), GetSettings()->GetOverallScalabilityLevel()), FColor::Green);

        GetSettings()->ApplySettings(false);
    }
    else
    {
        DebugHelper::PrintToLog("Game User Settings is null", FColor::Red);
    }
}

void UMenu::SaveGraphicsSettings()
{
    if(GetSettings())
    {
        if(MouseSensitivitySlider)
        {
            GetSettings()->SetMouseSensitivity(MouseSensitivitySlider->GetValue());
        }

        if(GlobalVolumeSlider)
        {
            GetSettings()->SetMasterSoundVolume(GlobalVolumeSlider->GetValue());

            USoundClass* MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master"));

            if (MasterSoundClass)
            {
                MasterSoundClass->Properties.Volume = GlobalVolumeSlider->GetValue();
            }
        }
    
        if(VersionText)
        {
            VersionText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GetSettings()->GetGameVersion())));
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
                    GetSettings()->SetScreenResolution(FIntPoint(Width, Height));
                }
            }
        }

        if(FullScreenModeSelect)
        {
            int32 WindowMode = FullScreenModeSelect->GetSelectedIndex();
            EWindowMode::Type FullScreenMode = EWindowMode::ConvertIntToWindowMode(WindowMode);

            GetSettings()->SetFullscreenMode(FullScreenMode);
        }

        GetSettings()->ApplySettings(false);
    }
}

void UMenu::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);
    JoinButton->SetIsEnabled(false);

    if(MapSelect)
    {
        MatchType = MapSelect->GetSelectedOption();
    }

    if(MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();//in case destroy a session if its alive
        MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
    }
}

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
        MultiplayerSessionsSubsystem->DestroySession();//in case destroy a session if its alive
        MultiplayerSessionsSubsystem->FindSessions(10);//The size of the search results shouldn't really matter.  Epic's documentation for BuildUniqueId is Used to keep different builds from seeing each other during searches
    }

    bIsJoining = true;
}

void UMenu::JoinCanceled()
{
    bIsJoining = false;

    if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->GetSessionInterface())
    {
        MultiplayerSessionsSubsystem->GetSessionInterface()->CancelFindSessions();
    }

    HostButton->SetIsEnabled(true);
    JoinButton->SetIsEnabled(true);
    JoinText->SetText(FText::FromString("Search"));
}

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

void UMenu::NativeDestruct()
{
	MenuTearDown();
 
	Super::NativeDestruct();
}

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

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful)
{
    if(MultiplayerSessionsSubsystem == nullptr) return;

    if(ServerList)
    {
        ServerList->ClearChildren();
    }

	for (auto Result : SessionResults)
	{
        if(!bIsJoining) return;

        // MultiplayerSessionsSubsystem->JoinSession(Result);

        FString Id = Result.GetSessionIdStr();
        FString UserName = Result.Session.OwningUserName;
        FString SettingsValue;
        FString GameType;

        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        Result.Session.SessionSettings.Get(FName("GameType"), GameType);

        DebugHelper::PrintToLog(FString::Printf(TEXT("Session %s - %s | Map: %s"), *Id, *UserName, *SettingsValue), FColor::Green);

        if(ServerList && GameType == FString("DeathEcho"))
        {
            //create a new widget 
            if(ListEntryWidget)
            {
                UWorld* World = GetWorld();
                if (World)
                {
                    UListViewEntryWidget * NewWidget = CreateWidget<UListViewEntryWidget>(World, ListEntryWidget);
                    if (NewWidget)
                    {
                        
                        //from NewWidget get a reference to the ServerTitle Text Block widget and change the server title
                        UTextBlock * ServerTitle = Cast<UTextBlock>(NewWidget->GetWidgetFromName("ServerTitle"));

                        if(ServerTitle)
                        {
                            ServerTitle->SetText(FText::FromString(FString::Printf(TEXT("Server: %s | Map: %s"),*UserName, *SettingsValue)));
                        }

                        NewWidget->Session = Result;

                        UButton * ServerJoinButton = Cast<UButton>(NewWidget->GetWidgetFromName("JoinButton"));

                        if(ServerJoinButton)
                        {
                            ServerJoinButton->SetToolTipText(FText::FromString(Id));
                        }

                        //add the new widget to ServerList StackBox
                        ServerList->AddChild(NewWidget);
                    }
                }
            }
        }
	}

    if(!bWasSuccessful)
    {
        // JoinButton->SetIsEnabled(true);
        HostButton->SetIsEnabled(true);
        JoinText->SetText(FText::FromString("Search"));
    }
}

void UMenu::JoinSession(FOnlineSessionSearchResult Session)
{
    FString Id = Session.GetSessionIdStr();
    FString UserName = Session.Session.OwningUserName;
    FString SettingsValue;

    Session.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

    DebugHelper::PrintToLog(FString::Printf(TEXT("Session %s - %s | Map: %s"), *Id, *UserName, *SettingsValue), FColor::Green);

    MultiplayerSessionsSubsystem->JoinSession(Session);//add the join session complete delegate
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
        JoinText->SetText(FText::FromString("Search"));
    }

    bIsJoining = false;
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
    bWasSuccessful ? DebugHelper::PrintToLog("Session Destroyed Successfully", FColor::Green) : DebugHelper::PrintToLog("Session Destroy Failed", FColor::Red);
}

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