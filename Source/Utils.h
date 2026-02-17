
#include <fstream>


void SendMessageToDiscordCallback(bool success, std::string results, std::unordered_map<std::string, std::string> responseHeaders)
{
	if (!success)
	{
		Log::GetLog()->error("Failed to send Post request. {} {} {}", __FUNCTION__, success, results);
	}
	else
	{
		Log::GetLog()->info("Success. {} {} {}", __FUNCTION__, success, results);
	}
}

void SendMessageToDiscord(std::string msg)
{
	if (PointRewards::config["DiscordBot"].value("Enabled", false) == false) return;

	Log::GetLog()->warn("Function: {}", __FUNCTION__);

	
	std::string webhook = PointRewards::config["DiscordBot"].value("Webhook", "");
	std::string botImgUrl = PointRewards::config["DiscordBot"].value("BotImageURL", "");

	if (webhook == "" || webhook.empty()) return;

	FString msgFormat = L"{{\"content\":\"{}\",\"username\":\"{}\",\"avatar_url\":\"{}\"}}";

	FString msgOutput = FString::Format(*msgFormat, msg, "ArkBot", botImgUrl);

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: PointRewards/1.0",
		"Connection: keep-alive",
		"Accept: */*"
	};

	try
	{
		bool req = PointRewards::req.CreatePostRequest(webhook, SendMessageToDiscordCallback, msgOutput.ToStringUTF8(), "application/json", headers);

		if(!req)
			Log::GetLog()->error("Failed to send Post request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to send Post request. Error: {}", error.what());
	}
}

bool AddSubPoints(FString eos_id, int point, bool isAdding = true, bool isChecking = false)
{
	nlohmann::json config = PointRewards::config["PointsDBSettings"];

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return false;
	}


	std::string escaped_eos_id = PointRewards::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!PointRewards::pointsDB->read(query, results))
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return false;
	}

	if (results.size() <= 0)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return false;
	}

	int current_points = std::atoi(results[0].at(points_field).c_str());

	int new_points = 0;

	if (isChecking)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Player got {} points", current_points);
		}

		if (current_points >= point) return true;
	}
	else
	{
		if (isAdding)
		{
			new_points = current_points + point;
		}
		else
		{
			if (new_points > 0)
				new_points = current_points + point;
		}

		std::vector<std::pair<std::string, std::string>> data;

		data.push_back({ points_field, std::to_string(new_points) });

		if (PointRewards::pointsDB->update(tablename, data, condition))
		{
			if (PointRewards::config["Debug"].value("Points", false) == true)
			{
				Log::GetLog()->info("{} Points DB updated", new_points);
			}

			return true;
		}
	}

	return false;

}

int GetPoints(FString eos_id)
{
	nlohmann::json config = PointRewards::config["PointsDBSettings"];

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return 0;
	}

	std::string escaped_eos_id = PointRewards::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!PointRewards::pointsDB->read(query, results))
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return 0;
	}

	if (results.size() <= 0)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return 0;
	}

	return std::atoi(results[0].at(points_field).c_str());
}

bool Points(FString eos_id, int cost, bool check_points = false)
{
	if (cost == -1)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is -1");
		}
		return false;
	}

	if (cost == 0)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is 0");
		}

		return true;
	}

	nlohmann::json config = PointRewards::config["PointsDBSettings"];

	if (config.value("Enabled", false) == false)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Points system is disabled");
		}

		return true;
	}

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return false;
	}

	std::string escaped_eos_id = PointRewards::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!PointRewards::pointsDB->read(query, results))
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return false;
	}

	if (results.size() <= 0)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return false;
	}

	int points = std::atoi(results[0].at(points_field).c_str());

	if (check_points)
	{
		if (PointRewards::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Player got {} points", points);
		}

		if (points >= cost) return true;
	}
	else
	{
		int amount = points - cost;

		std::vector<std::pair<std::string, std::string>> data;

		data.push_back({ points_field, std::to_string(amount) });

		if (totalspent_field != "")
		{
			int total_spent = std::atoi(results[0].at(totalspent_field).c_str());
			std::string total_ts = std::to_string(total_spent + cost);

			data.push_back({totalspent_field, total_ts});
		}

		std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

		if (PointRewards::pointsDB->update(tablename, data, condition))
		{
			if (PointRewards::config["Debug"].value("Points", false) == true)
			{
				Log::GetLog()->info("{} Points DB updated", amount);
			}

			return true;
		}
	}

	return false;
}

nlohmann::json GetCommandString(const std::string permission, const std::string command)
{
	if (permission.empty()) return {};
	if (command.empty()) return {};

	nlohmann::json config_obj = PointRewards::config["PermissionGroups"];
	nlohmann::json perm_obj = config_obj[permission];
	nlohmann::json command_obj = perm_obj["Commands"];
	nlohmann::json setting_obj = command_obj[command];

	return setting_obj;
}

TArray<FString> GetPlayerPermissions(FString eos_id)
{
	TArray<FString> PlayerPerms = { "Default" };

	std::string escaped_eos_id = PointRewards::permissionsDB->escapeString(eos_id.ToString());

	std::string tablename = PointRewards::config["PermissionsDBSettings"].value("TableName", "Players");

	std::string condition = PointRewards::config["PermissionsDBSettings"].value("UniqueIDField", "EOS_Id");

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}';", tablename, condition, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;
	if (!PointRewards::permissionsDB->read(query, results))
	{
		if (PointRewards::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->warn("Error reading permissions DB");
		}

		return PlayerPerms;
	}

	if (results.size() <= 0) return PlayerPerms;

	std::string permsfield = PointRewards::config["PermissionsDBSettings"].value("PermissionGroupField","PermissionGroups");

	FString playerperms = FString(results[0].at(permsfield));

	if (PointRewards::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("current player perms {}", playerperms.ToString());
	}

	playerperms.ParseIntoArray(PlayerPerms, L",", true);

	return PlayerPerms;
}

FString GetPriorPermByEOSID(FString eos_id)
{
	TArray<FString> player_groups = GetPlayerPermissions(eos_id);

	const nlohmann::json permGroups = PointRewards::config["PermissionGroups"];

	std::string defaultGroup = "Default";
	int minPriority = INT_MAX;
	nlohmann::json result;
	FString selectedPerm = "Default";

	for (const FString& param : player_groups)
	{
		if (permGroups.contains(param.ToString()))
		{
			int priority = static_cast<int>(permGroups[param.ToString()]["Priority"]);
			if (priority < minPriority)
			{
				minPriority = priority;
				result = permGroups[param.ToString()];
				selectedPerm = param;
			}
		}
	}

	if (result.is_null() && permGroups.contains(defaultGroup))
	{
		if(!permGroups[defaultGroup].is_null())
			result = permGroups[defaultGroup];

		result = {};
	}

	if (PointRewards::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("Selected Permission {}", selectedPerm.ToString());
	}

	return selectedPerm;
} 

bool ReadPlayer(FString eosID)
{
	std::string escaped_id = PointRewards::pluginTemplateDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", PointRewards::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	PointRewards::pluginTemplateDB->read(query, results);

	return results.size() <= 0 ? false : true;
}

int GetStatAmount(FString eosID, StatsType statsType)
{
	std::string escaped_id = PointRewards::pluginTemplateDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", PointRewards::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	PointRewards::pluginTemplateDB->read(query, results);

	std::string stats_field = "Kill";

	switch (statsType)
	{
	case StatsType::PlayerKill:
		stats_field = "Kill";
		break;
	case StatsType::PlayerDeath:
		stats_field = "Death";
		break;
	case StatsType::DinoKill:
		stats_field = "DinoKill";
		break;
	case StatsType::TamedKill:
		stats_field = "TamedKill";
		break;
	};

	if (results.size() > 0)
	{
		return std::atoi(results[0].at(stats_field).c_str());
	}
	else
	{
		return 0;
	}
}

bool AddOrUpdatePlayerStats(FString eosID, int playerID, FString playerName, StatsType statsType)
{
	std::vector<std::pair<std::string, std::string>> data1 = {
		{"EosId", eosID.ToString()},
		{"PlayerId", std::to_string(playerID)},
		{"PlayerName", playerName.ToString()}
	};

	std::vector<std::pair<std::string, std::string>> data2 = {};

	int current_amount = GetStatAmount(eosID, statsType);
	int new_amount = (current_amount <= 0) ? 1 : current_amount + 1;

	switch (statsType)
	{
		case StatsType::PlayerKill:
			data2.push_back({ "Kill" ,std::to_string(new_amount) });
			break;
		case StatsType::PlayerDeath:
			data2.push_back({ "Death" ,std::to_string(new_amount) });
			break;
		case StatsType::DinoKill:
			data2.push_back({ "DinoKill" ,std::to_string(new_amount) });
			break;
		case StatsType::TamedKill:
			data2.push_back({ "TamedKill" ,std::to_string(new_amount) });
			break;
	};

	try
	{
		if (ReadPlayer(eosID))
		{
			// update stats
			std::string unique_id = "EosId";

			std::string escaped_id = PointRewards::pluginTemplateDB->escapeString(eosID.ToString());

			std::string condition = fmt::format("{}='{}'", unique_id, escaped_id);

			return PointRewards::pluginTemplateDB->update(PointRewards::config["PluginDBSettings"]["TableName"].get<std::string>(), data2, condition);
		}
		else
		{
			//create stats
			data1.insert(data1.end(), data2.begin(), data2.end());

			return PointRewards::pluginTemplateDB->create(PointRewards::config["PluginDBSettings"]["TableName"].get<std::string>(), data1);
		}
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Error in AddOrUpdatePlayerStats: {}", error.what());
		return false;
	}
}

nlohmann::basic_json<> GetDinoConfig(const std::string& blueprint)
{
	auto dinos = PointRewards::config["DinoRewards"]["Dinos"];
	for (const auto& dino_entry : dinos)
	{
		const std::string bp = dino_entry["Blueprint"];
		if (bp == blueprint)
			return dino_entry;
	}
	 
	return nlohmann::json::value_type::object();
}

PlayerRanks GetPlayerRanks(FString eosID)
{
	PlayerRanks ranks;
	std::string escaped_id = PointRewards::pluginTemplateDB->escapeString(eosID.ToString());
	std::string tableName = PointRewards::config["PluginDBSettings"]["TableName"].get<std::string>();

	// Get Kill rank
	std::string killQuery = fmt::format(
		"SELECT COUNT(*) + 1 as rank FROM {} p1 WHERE p1.Kill > (SELECT Kill FROM {} p2 WHERE p2.EosId = '{}')",
		tableName, tableName, escaped_id
	);

	// Get Death rank (lower is better, so reverse the comparison)
	std::string deathQuery = fmt::format(
		"SELECT COUNT(*) + 1 as rank FROM {} p1 WHERE p1.Death < (SELECT Death FROM {} p2 WHERE p2.EosId = '{}')",
		tableName, tableName, escaped_id
	);

	// Get DinoKill rank
	std::string dinoKillQuery = fmt::format(
		"SELECT COUNT(*) + 1 as rank FROM {} p1 WHERE p1.DinoKill > (SELECT DinoKill FROM {} p2 WHERE p2.EosId = '{}')",
		tableName, tableName, escaped_id
	);

	// Get TamedKill rank
	std::string tamedKillQuery = fmt::format(
		"SELECT COUNT(*) + 1 as rank FROM {} p1 WHERE p1.TamedKill > (SELECT TamedKill FROM {} p2 WHERE p2.EosId = '{}')",
		tableName, tableName, escaped_id
	);

	std::vector<std::map<std::string, std::string>> results;

	// Execute Kill rank query
	if (PointRewards::pluginTemplateDB->read(killQuery, results) && !results.empty())
	{
		ranks.killRank = std::atoi(results[0]["rank"].c_str());
	}

	// Execute Death rank query
	results.clear();
	if (PointRewards::pluginTemplateDB->read(deathQuery, results) && !results.empty())
	{
		ranks.deathRank = std::atoi(results[0]["rank"].c_str());
	}

	// Execute DinoKill rank query
	results.clear();
	if (PointRewards::pluginTemplateDB->read(dinoKillQuery, results) && !results.empty())
	{
		ranks.dinoKillRank = std::atoi(results[0]["rank"].c_str());
	}

	// Execute TamedKill rank query
	results.clear();
	if (PointRewards::pluginTemplateDB->read(tamedKillQuery, results) && !results.empty())
	{
		ranks.tamedKillRank = std::atoi(results[0]["rank"].c_str());
	}

	return ranks;
}

void GetPlayerStats(AShooterPlayerController* pc)
{

	AsaApi::GetApiUtils().SendChatMessage(
		pc,
		PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
		PointRewards::config["Messages"].value("MyStatsMSG", "Kills ({}), Deaths ({}), Dino Kills ({}), Tamed Kills ({})").c_str(),
		GetStatAmount(pc->GetEOSId(), StatsType::PlayerKill),
		GetStatAmount(pc->GetEOSId(), StatsType::PlayerDeath),
		GetStatAmount(pc->GetEOSId(), StatsType::DinoKill),
		GetStatAmount(pc->GetEOSId(), StatsType::TamedKill));
}

void GetPlayerStatsWithRanks(AShooterPlayerController* pc)
{
	PlayerRanks ranks = GetPlayerRanks(pc->GetEOSId());

	AsaApi::GetApiUtils().SendChatMessage(
		pc,
		PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
		PointRewards::config["Messages"].value("MyLBStatsMSG", "Rank: Player Kill ({}), Player Death ({}), Dino Kill ({}), Tamed Kill ({})").c_str(), ranks.killRank, ranks.deathRank, ranks.dinoKillRank, ranks.tamedKillRank);
}

void ReadConfig()
{
	try
	{
		const std::string config_path = AsaApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/" + PROJECT_NAME + "/config.json";
		std::ifstream file{config_path};
		if (!file.is_open())
		{
			throw std::runtime_error("Can't open config file.");
		}
		file >> PointRewards::config;

		Log::GetLog()->info("{} config file loaded.", PROJECT_NAME);

		PointRewards::isDebug = PointRewards::config["General"]["Debug"].get<bool>();

		Log::GetLog()->warn("Debug {}", PointRewards::isDebug);

	}
	catch(const std::exception& error)
	{
		Log::GetLog()->error("Config load failed. ERROR: {}", error.what());
		throw;
	}
}

void LoadDatabase()
{
	Log::GetLog()->warn("LoadDatabase");
	PointRewards::pluginTemplateDB = DatabaseFactory::createConnector(PointRewards::config["PluginDBSettings"]);

	nlohmann::ordered_json tableDefinition = {};
	if (PointRewards::config["PluginDBSettings"].value("UseMySQL", true) == true)
	{
		tableDefinition = {
			{"Id", "INT NOT NULL AUTO_INCREMENT"},
			{"EosId", "VARCHAR(50) NOT NULL"},
			{"PlayerId", "VARCHAR(50) NOT NULL"},
			{"PlayerName", "VARCHAR(50) NOT NULL"},
			{"`Kill`", "INT DEFAULT 0"},
			{"`Death`", "INT DEFAULT 0"},
			{"`DinoKill`", "INT DEFAULT 0"},
			{"`TamedKill`", "INT DEFAULT 0"},
			{"CreateAt", "DATETIME DEFAULT CURRENT_TIMESTAMP"},
			{"PRIMARY", "KEY(Id)"},
			{"UNIQUE", "INDEX EosId_UNIQUE (EosId ASC)"}
		};
	}
	else
	{
		tableDefinition = {
			{"Id","INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT"},
			{"EosId","TEXT NOT NULL UNIQUE"},
			{"PlayerId","TEXT"},
			{"PlayerName","TEXT"},
			{"Kill", "INTEGER DEFAULT 0"},
			{"Death", "INTEGER DEFAULT 0"},
			{"DinoKill", "INTEGER DEFAULT 0"},
			{"TamedKill", "INTEGER DEFAULT 0"},
			{"CreateAt","TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
		};
	}

	PointRewards::pluginTemplateDB->createTableIfNotExist(PointRewards::config["PluginDBSettings"].value("TableName", ""), tableDefinition);


	// PermissionsDB
	if (PointRewards::config["PermissionsDBSettings"].value("Enabled", true) == true)
	{
		PointRewards::permissionsDB = DatabaseFactory::createConnector(PointRewards::config["PermissionsDBSettings"]);
	}

	// PointsDB (ArkShop)
	if (PointRewards::config["PointsDBSettings"].value("Enabled", true) == true)
	{
		PointRewards::pointsDB = DatabaseFactory::createConnector(PointRewards::config["PointsDBSettings"]);
	}
	
}