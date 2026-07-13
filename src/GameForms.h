#pragma once
#include <RE/T/TESGlobal.h>
#include <RE/B/BGSPerk.h>

enum SPECIALFormIDs
{
	StrengthID = 706,
	PerceptionID,
	EnduranceID,
	CharismaID,
	IntelligenceID,
	AgilityID,
	LuckID,
	ExperienceID
};

#ifndef VanillaAVStruct
#define VanillaAVStruct
struct VanillaAV_Struct
{
	// Vanilla Actor Values
	RE::ActorValueInfo* Strength;
	RE::ActorValueInfo* Perception;
	RE::ActorValueInfo* Endurance;
	RE::ActorValueInfo* Charisma;
	RE::ActorValueInfo* Intelligence;
	RE::ActorValueInfo* Agility;
	RE::ActorValueInfo* Luck;
};
#endif
extern VanillaAV_Struct VanillaActorValues;


#ifndef PA_SkillsStruct
#define PA_SkillsStruct
struct PA_Skills_Struct
{
	// PA Skill Values

	RE::ActorValueInfo* Barter;
	RE::ActorValueInfo* BigGuns;
	RE::ActorValueInfo* Doctor;
	RE::ActorValueInfo* EnergyWeapons;
	RE::ActorValueInfo* FirstAid;
	RE::ActorValueInfo* Gambling;
	RE::ActorValueInfo* Lockpick;
	RE::ActorValueInfo* MeleeWeapons;
	RE::ActorValueInfo* Outdoorsman;
	RE::ActorValueInfo* Repair;
	RE::ActorValueInfo* Science;
	RE::ActorValueInfo* SmallGuns;
	RE::ActorValueInfo* Sneak;
	RE::ActorValueInfo* Speech;
	RE::ActorValueInfo* Steal;
	RE::ActorValueInfo* Throwing;
	RE::ActorValueInfo* Traps;
	RE::ActorValueInfo* Unarmed;

	RE::ActorValueInfo* Karma;

	// PA Resists
};
#endif
extern PA_Skills_Struct PA_Skills;

#ifndef PA_GlobalsStruct
#define PA_GlobalsStruct
struct PA_Globals_Struct
{
	// PA Globals
	RE::TESGlobal* Karma;
	RE::TESGlobal* AmmoDefaultDegradation;
};
#endif
extern PA_Globals_Struct PA_Globals;

#ifndef PA_PerksStruct
#define PA_PerksStruct
struct PA_Perks_Struct
{
	RE::BGSPerk* ActionBoy;
	RE::BGSPerk* ActionGirl;
	RE::BGSPerk* ActivateFarrelSafe;
	RE::BGSPerk* AdrenalineRush;
	RE::BGSPerk* AnimalFriend;
	RE::BGSPerk* Awareness;
	RE::BGSPerk* BaseHealRate_StartupPerk;
	RE::BGSPerk* BetterCrits;
	RE::BGSPerk* BigBookOfSciencePerk;
	RE::BGSPerk* BloodyMessTrait;
	RE::BGSPerk* BonusHtHAttacks;
	RE::BGSPerk* BonusHtHDamage;
	RE::BGSPerk* BonusMove;
	RE::BGSPerk* BonusRangedDmg;
	RE::BGSPerk* BonusRateOfFire;
	RE::BGSPerk* BruiserTrait;
	RE::BGSPerk* CautiousNature;
	RE::BGSPerk* Champion;
	RE::BGSPerk* ChemReliantTrait;
	RE::BGSPerk* ChemResistantTrait;
	RE::BGSPerk* ChildKiller;
	RE::BGSPerk* Comprehension;
	RE::BGSPerk* CultOfPersonality;
	RE::BGSPerk* DeanElectronicPerk;
	RE::BGSPerk* DemolitionExpert;
	RE::BGSPerk* DermalImpactArmor01;
	RE::BGSPerk* DermalImpactArmor02;
	RE::BGSPerk* Dodger;
	RE::BGSPerk* EarlierSeq01;
	RE::BGSPerk* EarlierSeq02;
	RE::BGSPerk* EarlierSeq03;
	RE::BGSPerk* Educated01;
	RE::BGSPerk* Educated02;
	RE::BGSPerk* Educated03;
	RE::BGSPerk* Explorer;
	RE::BGSPerk* FasterHealing;
	RE::BGSPerk* FastMetabolismTrait;
	RE::BGSPerk* FastShotTrait;
	RE::BGSPerk* FinesseTrait;
	RE::BGSPerk* FortuneFinder;
	RE::BGSPerk* GainAGI;
	RE::BGSPerk* GainCHR;
	RE::BGSPerk* GainEND;
	RE::BGSPerk* GainINT;
	RE::BGSPerk* GainLCK;
	RE::BGSPerk* GainPER;
	RE::BGSPerk* GainSTR;
	RE::BGSPerk* Gambler;
	RE::BGSPerk* GeckoImmuneToFire;
	RE::BGSPerk* Ghost;
	RE::BGSPerk* GiftedTrait;
	RE::BGSPerk* GoodNaturedTrait;
	RE::BGSPerk* GunsAndBulletsPerk;
	RE::BGSPerk* Harmless;
	RE::BGSPerk* Healer;
	RE::BGSPerk* HeaveHo;
	RE::BGSPerk* HeavyHandedTrait;
	RE::BGSPerk* HereAndNow;
	RE::BGSPerk* HtHEvade;
	RE::BGSPerk* JinxedTrait;
	RE::BGSPerk* KamikazeTrait;
	RE::BGSPerk* KarmaBeacon;
	RE::BGSPerk* KarmaPerk;
	RE::BGSPerk* KarmaSutraMaster;
	RE::BGSPerk* LifeGiver;
	RE::BGSPerk* LightStep;
	RE::BGSPerk* LivingAnatomy;
	RE::BGSPerk* MagneticPersonality;
	RE::BGSPerk* MasterThief;
	RE::BGSPerk* MasterTrader;
	RE::BGSPerk* Medic;
	RE::BGSPerk* MoreCriticals;
	RE::BGSPerk* MrFixit;
	RE::BGSPerk* Mutate;
	RE::BGSPerk* MysteriousStranger;
	RE::BGSPerk* Negotiator;
	RE::BGSPerk* NightVision;
	RE::BGSPerk* OneHanderTrait;
	RE::BGSPerk* PackRat;
	RE::BGSPerk* PathFinder;
	RE::BGSPerk* PhoenixArmorImplant01;
	RE::BGSPerk* PhoenixArmorImplant02;
	RE::BGSPerk* PickPocket;
	RE::BGSPerk* PlayerChosenOnePerk;
	RE::BGSPerk* Presence;
	RE::BGSPerk* Pyromaniac;
	RE::BGSPerk* QuickPockets;
	RE::BGSPerk* QuickRecovery;
	RE::BGSPerk* RadResistance;
	RE::BGSPerk* Ranger;
	RE::BGSPerk* ReputationBerserker;
	RE::BGSPerk* ReputationChampion;
	RE::BGSPerk* ReputationChildkiller;
	RE::BGSPerk* ReputationGigolo;
	RE::BGSPerk* ReputationMarried;
	RE::BGSPerk* ReputationPrizeFighter;
	RE::BGSPerk* ReputationSeparated;
	RE::BGSPerk* ReputationSexpert;
	RE::BGSPerk* ReputationSlaver;
	RE::BGSPerk* ReputationVirginOfTheWastes;
	RE::BGSPerk* SalesMan;
	RE::BGSPerk* Scout;
	RE::BGSPerk* Scrounger;
	RE::BGSPerk* SexAppealTrait;
	RE::BGSPerk* Sharpshooter;
	RE::BGSPerk* SilentDeath;
	RE::BGSPerk* SilentRunning;
	RE::BGSPerk* SkilledTrait;
	RE::BGSPerk* SkillValueBarter;
	RE::BGSPerk* SkillValueBigGuns;
	RE::BGSPerk* SkillValueEnergyWeapons;
	RE::BGSPerk* SkillValueLockpick;
	RE::BGSPerk* SkillValueMeleeWeapons;
	RE::BGSPerk* SkillValueScience;
	RE::BGSPerk* SkillValueSmallGuns;
	RE::BGSPerk* SkillValueSneak;
	RE::BGSPerk* SkillValueSteal;
	RE::BGSPerk* SkillValueUnarmed;
	RE::BGSPerk* Slayer;
	RE::BGSPerk* SmallFrameTrait;
	RE::BGSPerk* SmoothTalker;
	RE::BGSPerk* Snakeater;
	RE::BGSPerk* Sniper;
	RE::BGSPerk* Speaker;
	RE::BGSPerk* StoneWall;
	RE::BGSPerk* StrongBack;
	RE::BGSPerk* Survivalist;
	RE::BGSPerk* SwiftLearner;
	RE::BGSPerk* Tag;
	RE::BGSPerk* Thief;
	RE::BGSPerk* Toughness;
	RE::BGSPerk* WeaponHandling;
};
#endif
extern PA_Perks_Struct PA_Perks;



#ifndef ItemDegredationStruct
#define ItemDegredationStruct
struct ItemDegredation_Struct
{
	//	these holds our weapon condition on the Weapon Form in Creation Kit
	RE::BGSDamageType* weaponConditionHealthMaxDMGT;
	RE::BGSDamageType* weaponConditionHealthStartingDMGT;

	//	Actor Values that hold information on Items
	RE::ActorValueInfo* itemConditionMaxHealth;
	RE::ActorValueInfo* itemConditionMinHealth;
	RE::ActorValueInfo* itemConditionStartCond;

	//	Reload Jam Stuff
	RE::BGSAction* actionReloadJam;

	/*
	//	Repair Lists for Weapons
	BGSListForm* repair10mmPistol;
	BGSListForm* repair10mmSub;
	BGSListForm* repair32CalPistol;
	BGSListForm* repairAlienBlaster;
	BGSListForm* repairAssaultRifle;
	BGSListForm* repairBaseballBat;
	BGSListForm* repairBBGun;
	BGSListForm* repairBrassKnuckles;
	BGSListForm* repairChineseAssault;
	BGSListForm* repairChineseSword;
	BGSListForm* repairChinesePistol;
	BGSListForm* repairDartGun;
	BGSListForm* repairFatman;
	BGSListForm* repairFlamer;
	BGSListForm* repairGatlingLaser;
	BGSListForm* repairHuntingRifle;
	BGSListForm* repairKnife;
	BGSListForm* repairKnifeCombat;
	BGSListForm* repairLaserPistol;
	BGSListForm* repairLaserRifle;
	BGSListForm* repairLeadPipe;
	BGSListForm* repairMesmetron;
	BGSListForm* repairMinigun;
	BGSListForm* repairMissileLauncher;
	BGSListForm* repairNailBoard;
	BGSListForm* repairPlasmaPistol;
	BGSListForm* repairPlasmaRifle;
	BGSListForm* repairPoliceBaton;
	BGSListForm* repairPoolCue;
	BGSListForm* repairPowerFist;
	BGSListForm* repairRailwayRifle;
	BGSListForm* repairRipper;
	BGSListForm* repairRockItLauncher;
	BGSListForm* repairRollingPin;
	BGSListForm* repairSawedOffShotgun;
	BGSListForm* repairScoped44Magnum;
	BGSListForm* repairShishkebab;
	BGSListForm* repairShotgunCombat;
	BGSListForm* repairSledgehammer;
	BGSListForm* repairSniperRifle;
	BGSListForm* repairSuperSledge;
	BGSListForm* repairSwitchblade;
	BGSListForm* repairTireIron;

	//	Repair Keywords for Weapons
	BGSKeyword* repairKW10mmPistol;
	BGSKeyword* repairKW10mmSub;
	BGSKeyword* repairKW32CalPistol;
	BGSKeyword* repairKWAlienBlaster;
	BGSKeyword* repairKWAssaultRifle;
	BGSKeyword* repairKWBaseballBat;
	BGSKeyword* repairKWBBGun;
	BGSKeyword* repairKWBrassKnuckles;
	BGSKeyword* repairKWChineseAssault;
	BGSKeyword* repairKWChineseSword;
	BGSKeyword* repairKWChinesePistol;
	BGSKeyword* repairKWDartGun;
	BGSKeyword* repairKWFatman;
	BGSKeyword* repairKWFlamer;
	BGSKeyword* repairKWGatlingLaser;
	BGSKeyword* repairKWHuntingRifle;
	BGSKeyword* repairKWKnife;
	BGSKeyword* repairKWKnifeCombat;
	BGSKeyword* repairKWLaserPistol;
	BGSKeyword* repairKWLaserRifle;
	BGSKeyword* repairKWLeadPipe;
	BGSKeyword* repairKWMesmetron;
	BGSKeyword* repairKWMinigun;
	BGSKeyword* repairKWMissileLauncher;
	BGSKeyword* repairKWNailBoard;
	BGSKeyword* repairKWPlasmaPistol;
	BGSKeyword* repairKWPlasmaRifle;
	BGSKeyword* repairKWPoliceBaton;
	BGSKeyword* repairKWPoolCue;
	BGSKeyword* repairKWPowerFist;
	BGSKeyword* repairKWRailwayRifle;
	BGSKeyword* repairKWRipper;
	BGSKeyword* repairKWRockItLauncher;
	BGSKeyword* repairKWRollingPin;
	BGSKeyword* repairKWSawedOffShotgun;
	BGSKeyword* repairKWScoped44Magnum;
	BGSKeyword* repairKWShishkebab;
	BGSKeyword* repairKWShotgunCombat;
	BGSKeyword* repairKWSledgehammer;
	BGSKeyword* repairKWSniperRifle;
	BGSKeyword* repairKWSuperSledge;
	BGSKeyword* repairKWSwitchblade;
	BGSKeyword* repairKWTireIron;
	*/

	//	Misc Forms
	RE::ActorValueInfo* tempConditionVariable;	// used by AddItemHealthPercent to temporarily add our wanted condition to weapon
};
#endif
extern ItemDegredation_Struct ItemDegredationForms;
