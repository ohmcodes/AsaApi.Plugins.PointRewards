

void AddOrRemoveCommands(bool addCmd = true)
{
	const FString MyStats = PointRewards::config["Commands"]["MyStatsCMD"].get<std::string>().c_str();
	if (!MyStats.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(MyStats, &MyStatsCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(MyStats);
		}
	}

	const FString MyLBStats = PointRewards::config["Commands"]["LeaderboardCMD"].get<std::string>().c_str();
	if (!MyLBStats.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(MyLBStats, &MyLBStatsCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(MyLBStats);
		}
	}
}