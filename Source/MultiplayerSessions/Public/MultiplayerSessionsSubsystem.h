// Copyright: Toni Poscic

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//
// DECLARATION OF CUSTOM DELEGATES FOR THE MENU CLASS TO BIND CALLBACKS TO
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult> & SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionsSubsystem();

	// FORCEINLINE IOnlineSessionPtr GetSessionInterface() const { return SessionInterface; }
	const IOnlineSessionPtr GetSessionInterface();

	/**
	 * Creates a new session with the specified number of public connections and match type.
	 *
	 * @param NumPublicConnections The number of public connections for the session.
	 * @param MatchType The match type for the session.
	 */
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	/**
	 * Finds online sessions.
	 *
	 * @param MaxSearchResults The maximum number of search results to return.
	 */
	void FindSessions(int32 MaxSearchResults);

	/**
	 * Joins the specified online session.
	 *
	 * @param SearchResult The search result of the session to join.
	 */
	void JoinSession(const FOnlineSessionSearchResult & SearchResult);

	/**
	 * Destroy session via name NAME_GameSession, and broadcast the destroy event
	 */
	void DestroySession();
	/**
	 * Starts an online session.
	 * If the session interface is not valid, an error message is printed and the function returns.
	 * Adds a delegate for the start session complete event.
	 * If starting the session fails, an error message is printed, the delegate is cleared, and a broadcast is sent indicating that the session was not started successfully.
	 */
	void StartSession();

	/**
	 * Custom delegates for the menu class to bind callbacks to
	 */
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	int32 DesiredNumberOfPublicConnections{};//this will initialize it to an empty string
	FString DesiredMatchType{};

protected:
	// Internal callbacks for the delegates added to the Online Session Interface delegate list
	// This will be called inside the MultiplayerSessionsSubsystem.cpp file
	/**
	 * Callback function called when the session creation is complete.
	 *
	 * @param SessionName The name of the session that was created.
	 * @param bWasSuccessful Indicates whether the session creation was successful or not.
	 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	/**
	 * Callback function called when finding sessions is complete.
	 *
	 * @param bWasSuccessful - Indicates whether the session search was successful or not.
	 */
	void OnFindSessionsComplete(bool bWasSuccessful);
	/**
	 * Callback function called when the join session operation is complete.
	 *
	 * @param SessionName The name of the session that was joined.
	 * @param Result The result of the join session operation.
	 */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	/**
	 * Callback function called when starting a session is complete.
	 *
	 * @param SessionName The name of the session that was started.
	 * @param bWasSuccessful True if the session was started successfully, false otherwise. 
	 */
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;//@TODO: Fix possible issue with storing IOnlineSessionPtr
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;//these are the settings used when we last created a session
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	// To add to the online session interface delegate functions
	// we will bind our MultiplayerSessionsSubsystem functions to the delegate functions
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
