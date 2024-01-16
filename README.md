# AsaApi.Plugins.PointRewards
ARK:SA ArkShop Plugin point rewards

# Requirements
- ServerAPI [Download/Install](https://gameservershub.com/forums/resources/ark-survival-ascended-serverapi-crossplay-supported.683/)
- Ofcourse the ARK:SA Server
- Bot Creation
- Discord Channel Integration WebHook

# Dependencies
- [ArkShop](https://gameservershub.com/forums/resources/ark-survival-ascended-arkshop-crossplay-supported.714/)
- [Permissions](https://gameservershub.com/forums/resources/ark-survival-ascended-permissions-crossplay-supported.713/)

# Future Updates
- Permission Integration
- Dedicated Notification (Not having ArkShop notif for points)
- Leaderboard
- Stats

# Blank Config
```
{
  "Mysql": {
    "MysqlDB": "",
    "MysqlHost": "",
    "MysqlPass": "",
    "MysqlPort": 3306,
    "MysqlUser": "",
    "UseMysql": false,
    "DbPathOverride": ""
  },
  "Commands": {
    "RCONReload": "PointRewards.Reload",
    "ConsoleReload": "PointRewards.Reload"
  },
  "DinoRewards": {
    "Enabled": true,
    "Dinos": [
      {
        "Blueprint": "Blueprint'/Game/PrimalEarth/Dinos/Dodo/Dodo_Character_BP.Dodo_Character_BP'",
        "Award": [
          {
            "MinLevel": 1,
            "MaxLevel": 20,
            "Points": 1
          },
          {
            "MinLevel": 21,
            "MaxLevel": 50,
            "Points": 5
          },
          {
            "MinLevel": 51,
            "MaxLevel": 150,
            "Points": 20
          }
        ]
      },
      {
        "Blueprint": "Blueprint'/Game/PrimalEarth/Dinos/Rex/Rex_Character_BP.Rex_Character_BP'",
        "Award": [
          {
            "MinLevel": 1,
            "MaxLevel": 50,
            "Points": 10
          },
          {
            "MinLevel": 51,
            "MaxLevel": 80,
            "Points": 20
          },
          {
            "MinLevel": 81,
            "MaxLevel": 120,
            "Points": 30
          }
        ]
      }
    ]
  },
  "PlayerRewards": {
    "Enabled": true,
    "ReceivePoints": 20,
    "AdditionalPointsPercent": 3,
    "LosePointsPercent": 80
  },
  "Messages": {
    "Sender": "Point Rewards",
    "LostPoints": "You have lost {} points"
  }
}
```


# Commented Config
```
{
  "Mysql": {
    "MysqlDB": "", /* Database name */
    "MysqlHost": "", /* host or ip address */
    "MysqlPass": "", /* password */
    "MysqlPort": 3306, /* port 3306 default mostly */
    "MysqlUser": "", /* db username */
    "UseMysql": false, /* Mysql credentials is required if enabled */
    "DbPathOverride": "" /* sqlite database location */
  },
  "Commands": {
    "RCONReload": "PointRewards.Reload", /* RCON reload command */
    "ConsoleReload": "PointRewards.Reload" /* Console reload command add cheat PointRewards.Reload*/
  },
  "DinoRewards": { /* List of dinos */
    "Enabled": true,
    "Dinos": [
      {
        "Blueprint": "Blueprint'/Game/PrimalEarth/Dinos/Dodo/Dodo_Character_BP.Dodo_Character_BP'",
        "Award": [
          {
            "MinLevel": 1,
            "MaxLevel": 20,
            "Points": 1
          },
          {
            "MinLevel": 21,
            "MaxLevel": 50,
            "Points": 5
          },
          {
            "MinLevel": 51,
            "MaxLevel": 150,
            "Points": 20
          }
        ]
      },
      {
        "Blueprint": "Blueprint'/Game/PrimalEarth/Dinos/Rex/Rex_Character_BP.Rex_Character_BP'",
        "Award": [
          {
            "MinLevel": 1,
            "MaxLevel": 50,
            "Points": 10
          },
          {
            "MinLevel": 51,
            "MaxLevel": 80,
            "Points": 20
          },
          {
            "MinLevel": 81,
            "MaxLevel": 120,
            "Points": 30
          }
        ]
      }
    ]
  },
  "PlayerRewards": { /* Player kill reward settings */
    "Enabled": true,
    "ReceivePoints": 20,
    "AdditionalPointsPercent": 3,
    "LosePointsPercent": 80
  },
  "Messages": {
    "Sender": "Point Rewards",
    "LostPoints": "You have lost {} points"
  }
}
```

# RELOAD COMMANDS​
Both RCON and Console command: `PointRewards.Reload`

# Support
Feel free to join my Discord server [ArkTools](https://discord.gg/q8rPGprjEJ)

# Give Love!
[Buy me a coffee](https://www.buymeacoffee.com/ohmcodes)
