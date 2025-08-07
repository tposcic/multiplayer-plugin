// Fill out your copyright notice in the Description page of Project Settings.


#include "OptionsMenu.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Combustion_Vertical/Settings/BaseSettings.h"
#include "Components/TextBlock.h"
#include "DebugHelper.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundClass.h"

UBaseSettings * UOptionsMenu::GetSettings()
{
	if (Settings == nullptr)
	{
		Settings = Cast<UBaseSettings>(UBaseSettings::GetGameUserSettings());
	}

	return Settings;
}

bool UOptionsMenu::Initialize()
{
	if(!Super::Initialize()) return false;

	if(SaveGraphicsButton)
	{
		SaveGraphicsButton->OnClicked.AddDynamic(this, &UOptionsMenu::SaveGraphicsSettings);
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

        if (DetailsSelect)
        {
            switch (GetSettings()->GetOverallScalabilityLevel())
            {
                case 0:
                    DetailsSelect->SetSelectedOption(FString::Printf(TEXT("Minimal")));
                    break;
                case 1:
                    DetailsSelect->SetSelectedOption(FString::Printf(TEXT("Steam Deck")));
                    break;
                case 2:
                    DetailsSelect->SetSelectedOption(FString::Printf(TEXT("High")));
                    break;
                case 3:
                    DetailsSelect->SetSelectedOption(FString::Printf(TEXT("Mad")));
                    break;
                default:
                    break;
            }
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

void UOptionsMenu::SaveGraphicsSettings()
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

            if (USoundClass* MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master")))
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

        if (DetailsSelect)
        {
            GraphicsQualityUpdate(DetailsSelect->GetSelectedIndex());
        }

        GetSettings()->ApplySettings(false);
    }
}

void UOptionsMenu::GraphicsQualityUpdate(int32 QualityLevel)
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
