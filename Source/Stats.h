

void MyStatsCallback(AShooterPlayerController* pc, FString* param, int, int)
{
	Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "MyStatsCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (PointRewards::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, 1.3f, 15.0f, nullptr, PointRewards::config["Messages"].value("PermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, 1.3f, 15.0f, nullptr, PointRewards::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	GetPlayerStats(pc);

	// points deductions
	Points(pc->GetEOSId(), command.value("Cost", 0));
}


void MyLBStatsCallback(AShooterPlayerController* pc, FString* param, int, int)
{
	Log::GetLog()->warn("Function: {}", __FUNCTION__);

	// permissions check
	FString perms = GetPriorPermByEOSID(pc->GetEOSId());
	nlohmann::json command = GetCommandString(perms.ToString(), "LeaderboardCMD");

	if (command.is_null() || (!command.is_null() && command.value("Enabled", false) == false))
	{
		if (PointRewards::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->info("{} No permissions. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, 1.3f, 15.0f, nullptr, PointRewards::config["Messages"].value("PermErrorMSG", "You don't have permission to use this command.").c_str());

		return;
	}

	// points checking
	if (Points(pc->GetEOSId(), command.value("Cost", 0), true) == false)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->info("{} don't have points. Command: {}", pc->GetCharacterName().ToString(), __FUNCTION__);
		}

		AsaApi::GetApiUtils().SendNotification(pc, FColorList::Red, 1.3f, 15.0f, nullptr, PointRewards::config["Messages"].value("PointsErrorMSG", "Not enough points.").c_str());

		return;
	}

	// execute
	GetPlayerStatsWithRanks(pc);

	// points deductions
	Points(pc->GetEOSId(), command.value("Cost", 0));
}