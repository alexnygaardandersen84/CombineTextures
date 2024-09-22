#pragma once
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace DebugHeader
{
	static void Print(const FString& Message, const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);

		}

	}

	static void PrintLog(const FString& Message)
	{

		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);

	}

	//ShowMsgDialog is our variable name
	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMessageAsWarning = true)
	{

		if (bShowMessageAsWarning)
		{
			FText MsgTitle = FText::FromString(TEXT("Warning!"));

			return FMessageDialog::Open(MsgType, FText::FromString(Message), MsgTitle);
		}
		else
		{

			return FMessageDialog::Open(MsgType, FText::FromString(Message));

		}
	}


	static void ShowNotifyInfo(const FString& Message)
	{

		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);

	}

}
#pragma once
