# MultiplayerPlugin 
Unreal Engine 5 Plugin that provides a subsystem to handle online connectivity and steamworks

## Features
- Subsystem that handles online connectivity
- Subsystem that handles menu integration with the online subsystem

## Requirements

- Unreal Engine: 5.3
- Is generally tailored for the Steam platform

## Setup

### 1. Project Creation
Create a new project and pull the plugin from the repository into the Plugins folder for your project. In the Unreal Editor, go to Edit -> Plugins and search for `Online Subsystem Steam` and `MultiplayerSessions` and enable them.

### 2. Project Configuration 
Open the `DefaultEngine.ini` file and copy the following:

  ```ini
[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver") # The net driver to use for the game

[OnlineSubsystem]
DefaultPlatformService=Steam # The platform service to use by default

[OnlineSubsystemSteam]
bEnabled=true # Whether the subsystem is enabled or not
SteamDevAppId=480 # This would be the AppID of your game, here set to Spacewar (480) for testing
bInitServerOnClient=true # Whether the server should be initialized on the client or not

Achievement_0_Id=ACH_WIN_ONE_GAME ## You add in the achievements you want to use
Achievement_1_Id=ACH_WIN_100_GAMES
Achievement_2_Id=ACH_TRAVEL_FAR_ACCUM
Achievement_3_Id=ACH_TRAVEL_FAR_SINGLE
Achievement_4_Id=NEW_ACHIEVEMENT_NAME_0_4

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection" # The class to use for the net connection

  ```
  Then in `DefaultGame.ini` add the following:
  ```ini
  [/Script/Engine.GameSession]
  MaxPlayers=100
  ```

### 3. Regenerate Project Files
Regenerate and rebuild your project files.

### 4. Connect the Menu
Set a default level and in that level open the level blueprint. On `EventBeginPlay`, add a `CreateWidget` node. In the class parameter select `WBP_Menu`. From that node add another node and search for `Menu Setup`. Here you can set Number Of Public Connections, Type Of Match, and select the Lobby Level from a dropdown.

### 5. Build and Test
After building the game if you want to check for Steam connectivity you should either package the project and run it or run it in the editor by selecting "Standalone Game" from the dropdown menu. Playing it directly from the editor will not work and the client will be registered as a LAN connection.

## Notes
You can test the plugin by running the game in the editor and selecting `Play As Client` and `Play As Server` from the editor. You can also package the game and test it on a local network or on the Steam platform. In order to test hosting and joining you would need multiple steam accounts or a friend to help you test.

If using the default Steam Game Id (480) you need to have the same Download Region set as all the other players hosting or connecting. This can be set in the Steam client by going to `Steam -> Settings -> Downloads -> Download Region`.