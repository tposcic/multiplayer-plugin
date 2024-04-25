#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Online/OnlineSessionNames.h"
#include "DebugHelper.h"

/**
 * Constructor for the UMultiplayerSessionsSubsystem class.
 * Initializes the delegates for session creation, finding sessions, joining sessions, destroying sessions, and starting sessions.
 * Retrieves the online subsystem and session interface.
 */
UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
    FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
    JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
    DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
    StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
    IOnlineSubsystem * Subsystem = IOnlineSubsystem::Get();//Get the Online Subsystem
    
    if(Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();//Get the Session Interface
    }
}

/**
 * Creates a new session with the specified number of public connections and match type.
 *
 * @param NumPublicConnections The number of public connections for the session.
 * @param MatchType The match type for the session.
 */
void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
    if(!SessionInterface.IsValid()) return;

    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);

    if(ExistingSession)
    {
        bCreateSessionOnDestroy = true;
        LastNumPublicConnections = NumPublicConnections;
        LastMatchType = MatchType;
        DestroySession();
    }

    //////////////////////////////////////////////////////////////////////////
    // SESSION SETUP
    //////////////////////////////////////////////////////////////////////////

    LastSessionSettings = MakeShareable(new FOnlineSessionSettings());//create a new session settings object

	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // if we are using the steam subsystem it is not a lan match, but if we are using the null subsystem it is a lan match
	LastSessionSettings->NumPublicConnections = NumPublicConnections; // set the number of public connections to the value passed in as a parameter
	LastSessionSettings->bAllowJoinInProgress = true; // allow players to join the session even if it is already in progress
	LastSessionSettings->bAllowJoinViaPresence = true; // allow players to join the session via presence
	LastSessionSettings->bShouldAdvertise = true; // allow the session to be advertised
	LastSessionSettings->bUsesPresence = true; // use presence to advertise the session
	LastSessionSettings->bUseLobbiesIfAvailable = true; // use lobbies if available
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);//set the map name
    LastSessionSettings->BuildUniqueId = 1;//multiple users can have the same session name

    //////////////////////////////////////////////////////////////////////////
    // CREATE SESSION
    //////////////////////////////////////////////////////////////////////////

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);//store the delegate in an FDelegateHandle so we can later remove it from the delegate list

    const ULocalPlayer * LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//get the first local player

	if(!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))//create the session using the session settings object
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);//clear the delegate

        MultiplayerOnCreateSessionComplete.Broadcast(false);//broadcast that the session was not created successfully
    }
}

/**
 * Finds online sessions.
 *
 * @param MaxSearchResults The maximum number of search results to return.
 */
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if(!SessionInterface.IsValid())//if the session interface is not valid
	{
        DebugHelper::PrintToLog("Online Session Interface is not valid!", FColor::Red);

		return;
	}

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);//add the find sessions complete delegate

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());//create a new session search object wrapped in a shared pointer

	LastSessionSearch->MaxSearchResults = MaxSearchResults; // set the maximum search results
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // check if the subsystem is null, if it is set the query to lan, if it is not set the query to not lan
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);//set the query settings

	const ULocalPlayer * LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//get the first local player

	if(!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))//add the find sessions complete delegate
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);//clear the delegate

        MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);//broadcast that the session was not found successfully
    }
}

/**
 * Joins the specified online session.
 *
 * @param SearchResult The search result of the session to join.
 */
void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult & SearchResult)
{
    if(!SessionInterface.IsValid()) {
        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);//broadcast that the session was not joined successfully

        DebugHelper::PrintToLog("Online Session Interface is not valid!", FColor::Red);

        return;
    }

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);//add the join session complete delegate

    const ULocalPlayer * LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//get the first local player

    if(!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))//join the session
    {
        DebugHelper::PrintToLog("Failed to join session!", FColor::Red);

        //ovdje samo javljamo da je doslo do greske
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);//clear the delegate

        MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);//broadcast that the session was not joined successfully
    }   
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
    if(!SessionInterface.IsValid())
    {
        MultiplayerOnDestroySessionComplete.Broadcast(false);//broadcast that the session was not destroyed successfully

        return;
    }

    DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

    if(!SessionInterface->DestroySession(NAME_GameSession))
    {
        DebugHelper::PrintToLog("Failed to destroy session!", FColor::Red);

        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

        MultiplayerOnDestroySessionComplete.Broadcast(false);//broadcast that the session was not destroyed successfully
    }
}

/**
 * Starts an online session.
 * If the session interface is not valid, an error message is printed and the function returns.
 * Adds a delegate for the start session complete event.
 * If starting the session fails, an error message is printed, the delegate is cleared, and a broadcast is sent indicating that the session was not started successfully.
 */
void UMultiplayerSessionsSubsystem::StartSession()
{
    if(!SessionInterface.IsValid())
    {        
        DebugHelper::PrintToLog("Online Session Interface is not valid!", FColor::Red);

        return;
    };

    StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);//add the start session complete delegate
    
    if(!SessionInterface->StartSession(NAME_GameSession))//start the session
    {
        DebugHelper::PrintToLog("Failed to start session!", FColor::Red);

        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);//clear the delegate

        MultiplayerOnStartSessionComplete.Broadcast(false);//broadcast that the session was not started successfully
    }
}


/**
 * Callback function called when the session creation is complete.
 *
 * @param SessionName The name of the session that was created.
 * @param bWasSuccessful Indicates whether the session creation was successful or not.
 */
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(bWasSuccessful)//if the session was created successfully
	{
        DebugHelper::PrintToLog(FString::Printf(TEXT("Created session %s!"), *SessionName.ToString()), FColor::Green);

        if(SessionInterface)
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);//clear the delegate
        }

        MultiplayerOnCreateSessionComplete.Broadcast(true);//broadcast that the session was created successfully
	}
	else//if the session was not created successfully
	{
        DebugHelper::PrintToLog("Session Creation Failed!", FColor::Red);

        MultiplayerOnCreateSessionComplete.Broadcast(false);//broadcast that the session was not created successfully
	}
}

/**
 * Callback function called when finding sessions is complete.
 *
 * @param bWasSuccessful - Indicates whether the session search was successful or not.
 */
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);//clear the delegate

        if(bWasSuccessful)
        {
            if(LastSessionSearch->SearchResults.Num() <= 0)
            {
                MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);//broadcast that the session was not found successfully

                return;
            }

            TArray<FOnlineSessionSearchResult> SessionResults = LastSessionSearch->SearchResults;//get the search results -> SearchResults is a TArray of FOnlineSessionSearchResult

            MultiplayerOnFindSessionsComplete.Broadcast(SessionResults, true);//broadcast that the session was found successfully
        }
        else
        {
            MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);//broadcast that the session was not found successfully
        }
    }
}

/**
 * Callback function called when the join session operation is complete.
 *
 * @param SessionName The name of the session that was joined.
 * @param Result The result of the join session operation.
 */
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);//clear the delegate since we are done with it
    }
    
    MultiplayerOnJoinSessionComplete.Broadcast(Result);//broadcast that the session was joined successfully
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);//clear the delegate
    }

    if(bWasSuccessful && bCreateSessionOnDestroy)
    {
        bCreateSessionOnDestroy = false;
        CreateSession(LastNumPublicConnections, LastMatchType);
    }

    MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);//broadcast that the session was destroyed successfully

}

/**
 * Callback function called when starting a session is complete.
 *
 * @param SessionName The name of the session that was started.
 * @param bWasSuccessful True if the session was started successfully, false otherwise.
 */
void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if(SessionInterface)
    {
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);//clear the delegate
    }

    MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);//broadcast that the session was started successfully
}
