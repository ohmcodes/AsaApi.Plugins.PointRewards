
DECLARE_HOOK(APrimalDinoCharacter_Die, bool, APrimalDinoCharacter*, float, FDamageEvent*, AController*, AActor*);

bool Hook_APrimalDinoCharacter_Die(APrimalDinoCharacter* _this, float KillingDamage, FDamageEvent* DamageEvent,
    AController* Killer, AActor* DamageCauser)
{

    if (Killer && !Killer->IsLocalController() && Killer->IsA(AShooterPlayerController::GetPrivateStaticClass()) &&
        _this->TargetingTeamField() != Killer->TargetingTeamField())
    {
        const FString eos_id = AsaApi::GetApiUtils().GetEOSIDFromController(static_cast<AShooterPlayerController*>(Killer));

        AShooterPlayerController* killer_shooter_pc = static_cast<AShooterPlayerController*>(Killer);

        FString killername = killer_shooter_pc->GetPlayerCharacter()->PlayerNameField();

        int points_given = 0;

        const bool dino_rewards_enabled = PointRewards::config["DinoRewards"]["Enabled"];
        if (dino_rewards_enabled)
        {
            const bool unclaim_points = PointRewards::config["DinoRewards"].value("GiveUnclaimPoints", false);
            if (!unclaim_points && _this->OwningPlayerIDField() == 0 && _this->TargetingTeamField() >= 50000)
            {
                //Log::GetLog()->info("Killing unclaimed dino");
            }
            else
            {
                FString bp = AsaApi::GetApiUtils().GetBlueprint(_this);

                const auto dino_entry = GetDinoConfig(bp.ToString());
                if (!dino_entry.empty())
                {
                    UPrimalCharacterStatusComponent* char_comp = _this->MyCharacterStatusComponentField();
                    if (char_comp)
                    {
                        const int dino_level = char_comp->BaseCharacterLevelField() + char_comp->ExtraCharacterLevelField();

                        auto award = dino_entry["Award"];
                        for (const auto& award_entry : award)
                        {
                            const int min_level = award_entry["MinLevel"];
                            const int max_level = award_entry["MaxLevel"];

                            if (dino_level >= min_level && dino_level <= max_level)
                            {
                                const bool negative_points = PointRewards::config["PlayerRewards"].value("NegativePoints", false);

                                const int points = award_entry["Points"];
                                points_given = points;
                                if (points >= 0)
                                {
                                    AddSubPoints(eos_id, points, true, false);
                                }
                                else if (negative_points || GetPoints(eos_id) - abs(points) >= 0)
                                {
                                    AddSubPoints(eos_id, points, false, false);

                                    AShooterPlayerController* killer_controller = AsaApi::GetApiUtils().FindPlayerFromEOSID(eos_id);
                                    //AShooterPlayerController* killer_controlle = static_cast<AShooterPlayerController*>(Killer)
                                    if (killer_controller)
                                    {
                                        const std::string message_type = PointRewards::config["PlayerRewards"].value("MessageType", "notification");

                                        if (message_type == "notification")
                                        {
                                            const float display_scale = PointRewards::config["PlayerRewards"].value("NotificationScale", 1.3f);
                                            const float display_time = PointRewards::config["PlayerRewards"].value("NotificationDisplayTime", 10.0f);



                                            AsaApi::GetApiUtils().SendNotification(
                                                killer_controller,
                                                FColorList::Red,
                                                display_scale,
                                                display_time,
                                                nullptr,
                                                PointRewards::config["Messages"].value("LostPointsMSG", "You have lost {} points").c_str(), abs(points));
                                        }
                                        else if (message_type == "chat")
                                        {
                                            AsaApi::GetApiUtils().SendChatMessage(
                                                killer_controller,
                                                PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
                                                PointRewards::config["Messages"].value("LostPointsMSG", "You have lost {} points").c_str(),
                                                abs(points));
                                        }
                                    }
                                }

                                std::string msg = fmt::format(PointRewards::config["Messages"].value("DCDinoKilledMSG", "Player Killed a {}. Points given {}").c_str(), _this->DescriptiveNameField().ToString(), points_given);

                                SendMessageToDiscord(msg);

                                break;
                            }
                        }
                    }
                }
            }
        }
        if (_this->TargetingTeamField() < 50000)
        {
            AddOrUpdatePlayerStats(eos_id, killer_shooter_pc->GetLinkedPlayerID(), killername, StatsType::DinoKill);
        }
        else
        {
            AddOrUpdatePlayerStats(eos_id, killer_shooter_pc->GetLinkedPlayerID(), killername, StatsType::TamedKill);
        }
    }

    return APrimalDinoCharacter_Die_original(_this, KillingDamage, DamageEvent, Killer, DamageCauser);
}


DECLARE_HOOK(AShooterCharacter_Die, bool, AShooterCharacter*, float, FDamageEvent*, AController*, AActor*);

bool Hook_AShooterCharacter_Die(AShooterCharacter* shooter_character, float KillingDamage, FDamageEvent* DamageEvent, AController* Killer, AActor* DamageCauser)
{
	FString playername = shooter_character->PlayerNameField();

	Log::GetLog()->info("Player: {}, Dies!", playername.ToString());

    if (Killer && !Killer->IsLocalController() && Killer->IsA(AShooterPlayerController::GetPrivateStaticClass()) &&
        shooter_character->TargetingTeamField() != Killer->TargetingTeamField() && shooter_character->GetPlayerData())
    {
        AShooterPlayerController* victim_shooter_pc = AsaApi::GetApiUtils().FindControllerFromCharacter(static_cast<AShooterCharacter*>(shooter_character));

        AShooterPlayerController* killer_shooter_pc = static_cast<AShooterPlayerController*>(Killer);
        
        FString killername = killer_shooter_pc->GetPlayerCharacter()->PlayerNameField();

        const FString victim_eos_id = AsaApi::GetApiUtils().GetEOSIDFromController(victim_shooter_pc);

        const FString killer_eos_id = AsaApi::GetApiUtils().GetEOSIDFromController(Killer);

        const bool player_rewards_enabled = PointRewards::config["PlayerRewards"]["Enabled"];

        if (!player_rewards_enabled) return AShooterCharacter_Die_original(shooter_character, KillingDamage, DamageEvent, Killer, DamageCauser);;

        const std::string message_type = PointRewards::config["PlayerRewards"].value("MessageType", "notification");

        const int receive_points = PointRewards::config["PlayerRewards"]["ReceivePoints"];

        const int additional_points_percent = PointRewards::config["PlayerRewards"]["AdditionalPointsPercent"];
        const int lose_points_percent = PointRewards::config["PlayerRewards"]["LosePointsPercent"];
        const int victim_points = GetPoints(victim_eos_id);
        const int final_points = receive_points + victim_points * additional_points_percent / 100;
        const int lose_points = final_points * lose_points_percent / 100;

        if (receive_points < 0)
        {
            const bool negative_points = PointRewards::config["PlayerRewards"].value("NegativePoints", false);
            if (negative_points || GetPoints(killer_eos_id) - abs(receive_points) >= 0)
            {
                //deduct points
                AddSubPoints(killer_eos_id, abs(receive_points), false, false);

                AShooterPlayerController* killer_controller = AsaApi::GetApiUtils().FindPlayerFromEOSID(killer_eos_id);
                if (killer_controller)
                {
                    if (message_type == "notification")
                    {
                        AsaApi::GetApiUtils().SendNotification(killer_controller, FColorList::Orange, 1.3f, 15.0f, nullptr, PointRewards::config["Messages"].value("LostPointsMSG", "You have lost {} points").c_str(), abs(receive_points));
                    }
                    else if (message_type == "chat")
                    {
                        AsaApi::GetApiUtils().SendChatMessage(
                            killer_controller,
                            PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
                            PointRewards::config["Messages"].value("LostPointsMSG", "You have lost {} points").c_str(),
                            abs(receive_points));
                    }
                }
            }
        }
        else
        {
            if (victim_points > lose_points && AddSubPoints(killer_eos_id, final_points, true, false) &&
                AddSubPoints(victim_eos_id, lose_points, false, false))
            {
                AShooterPlayerController* victim_controller = AsaApi::GetApiUtils().FindPlayerFromEOSID(victim_eos_id);
                AShooterPlayerController* killer_controller = AsaApi::GetApiUtils().FindPlayerFromEOSID(killer_eos_id);

                AsaApi::GetApiUtils().SendChatMessage(
                    victim_controller,
                    PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
                    PointRewards::config["Messages"].value("LostPointsMSG", "You have lost {} points").c_str(),
                    abs(lose_points));

                AsaApi::GetApiUtils().SendChatMessage(
                    killer_controller,
                    PointRewards::config["Messages"].value("Sender", "Point Rewards").c_str(),
                    PointRewards::config["Messages"].value("GainPointsMSG", "You have gain {} points").c_str(),
                    abs(final_points));

                std::string msg = fmt::format(PointRewards::config["Messages"].value("DiscordMSG", "Player {} gain {} Points and Player {} has been killed and lose {} points").c_str(), killer_controller->GetCharacterName().ToString(), final_points, victim_controller->GetCharacterName().ToString(), lose_points);

                SendMessageToDiscord(msg);
            }
        }

        
        AddOrUpdatePlayerStats(killer_eos_id, killer_shooter_pc->GetLinkedPlayerID(), killername,StatsType::PlayerKill);

        AddOrUpdatePlayerStats(victim_eos_id, victim_shooter_pc->GetLinkedPlayerID(), playername, StatsType::PlayerDeath);

        std::string msg = fmt::format(PointRewards::config["Messages"].value("DiscordMSG", "Player {} ({}) has been killed by {} ({})").c_str(), playername.ToString(), lose_points, killername.ToString(), receive_points);

        SendMessageToDiscord(msg);
    }

	return AShooterCharacter_Die_original(shooter_character, KillingDamage, DamageEvent, Killer, DamageCauser);
}

void SetHooks(bool addHooks = true)
{
	if (addHooks)
	{
        AsaApi::GetHooks().SetHook("APrimalDinoCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_APrimalDinoCharacter_Die, &APrimalDinoCharacter_Die_original);

		AsaApi::GetHooks().SetHook("AShooterCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_AShooterCharacter_Die, &AShooterCharacter_Die_original);
	}
	else
	{
        AsaApi::GetHooks().DisableHook("APrimalDinoCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_APrimalDinoCharacter_Die);

		AsaApi::GetHooks().DisableHook("AShooterCharacter.Die(float,FDamageEvent&,AController*,AActor*)", &Hook_AShooterCharacter_Die);
	}
}