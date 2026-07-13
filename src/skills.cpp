#include "skills.h"
#include "GameForms.h"
#include "IGlobalConfig.h"
#include "debugLog.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <RE/A/Actor.h>
#include <RE/A/ActorValueInfo.h>
#include <RE/A/ActorValueOwner.h>
#include <RE/B/BGSPerk.h>
#include <RE/B/BSFixedString.h>
#include <RE/B/BSScript_IVirtualMachine.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESForm.h>
#include <RE/T/TESGlobal.h>
#include <format>

namespace Skills {
	RE::BSTArray<RE::BGSPerk*> PAPerksLevelUp;
	RE::BSTArray<RE::ActorValueInfo*> PASkillsLevelUp;
	RE::BSTArray<RE::BGSSoundDescriptorForm*> ScaleformSkillSounds;
	RE::BSTArray<RE::BGSSoundDescriptorForm*> ScaleformPerkSounds;

	std::vector<RE::ActorValueInfo*> SkillsList;
	std::unordered_map<std::string, RE::ActorValueInfo*> strSkillMap = std::unordered_map<std::string, RE::ActorValueInfo*>();

	std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>> skillsToSpecialMap = std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>>();
	std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>> specialToSkillsMap = std::unordered_map<const RE::ActorValueInfo*, std::vector<RE::ActorValueInfo*>>();

	std::unordered_map<std::string, RE::ActorValueInfo*> PerksMap = std::unordered_map<std::string, RE::ActorValueInfo*>();

	std::vector<RE::BGSPerk*> PANPCPerksList = std::vector<RE::BGSPerk*>();
}

// VanillaAV_Struct VanillaActorValues;

using namespace Skills;




bool RegisterSkillFunctions(RE::BSScript::IVirtualMachine& vm)
{
	
	vm.BindNativeMethod("PArroyo", "ModPermanentSkillValue", PA_SkillsPapyrus::ModPermanentSkillValue_Papyrus, false, false);
	vm.BindNativeMethod("PArroyo", "GetSkillAV", PA_SkillsPapyrus::GetSkillByName_Papyrus, false, false);
	vm.BindNativeMethod("PArroyo", "GetSkill", PA_SkillsPapyrus::GetSkillValueByName_Papyrus, false, false);
	vm.BindNativeMethod("PArroyo", "GetBaseSkill", PA_SkillsPapyrus::GetBaseSkillValueByName_Papyrus, false, false);

	// Debug
	vm.BindNativeMethod("PArroyo", "DEBUG_LogSkillsToConsole", PA_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus, false, false);
	vm.BindNativeMethod("PArroyo", "DEBUG_LogSkillToConsole", PA_SkillsPapyrus::DEBUG_LogSkillToConsole_Papyrus, false, false);
	
	return true;
}

void InitializeGameVariables(RE::TESDataHandler* dataHandler, std::string mod_esm)
{
	Initialize_TOBEMERGED_Vars(dataHandler);
	InitializeGlobalVariables(dataHandler);

	// S.P.E.C.I.A.L
	VanillaActorValues.Strength = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::StrengthID, mod_esm);
	VanillaActorValues.Perception = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::PerceptionID, mod_esm);
	VanillaActorValues.Endurance = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::EnduranceID, mod_esm);
	VanillaActorValues.Charisma = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::CharismaID, mod_esm);
	VanillaActorValues.Intelligence = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::IntelligenceID, mod_esm);
	VanillaActorValues.Agility = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::AgilityID, mod_esm);
	VanillaActorValues.Luck = dataHandler->LookupForm<RE::ActorValueInfo>(SPECIALFormIDs::LuckID, mod_esm);

	// Skills
	PA_Skills.Barter = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6D, mod_esm);
	PA_Skills.BigGuns = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D5F, mod_esm);
	PA_Skills.Doctor = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D65, mod_esm);
	PA_Skills.EnergyWeapons = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D60, mod_esm);
	PA_Skills.FirstAid = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D64, mod_esm);
	PA_Skills.Gambling = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6E, mod_esm);
	PA_Skills.Lockpick = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D67, mod_esm);
	PA_Skills.MeleeWeapons = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D62, mod_esm);
	PA_Skills.Outdoorsman = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6F, mod_esm);
	PA_Skills.Repair = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6B, mod_esm);
	PA_Skills.Science = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6A, mod_esm);
	PA_Skills.SmallGuns = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D5E, mod_esm);
	PA_Skills.Sneak = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D66, mod_esm);
	PA_Skills.Speech = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D6C, mod_esm);
	PA_Skills.Steal = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D68, mod_esm);
	PA_Skills.Throwing = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D63, mod_esm);
	PA_Skills.Traps = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D69, mod_esm);
	PA_Skills.Unarmed = dataHandler->LookupForm<RE::ActorValueInfo>(0x005D61, mod_esm);

	SkillsList.emplace_back(PA_Skills.Barter);
	SkillsList.emplace_back(PA_Skills.BigGuns);
	SkillsList.emplace_back(PA_Skills.Doctor);
	SkillsList.emplace_back(PA_Skills.EnergyWeapons);
	SkillsList.emplace_back(PA_Skills.FirstAid);
	SkillsList.emplace_back(PA_Skills.Gambling);
	SkillsList.emplace_back(PA_Skills.Lockpick);
	SkillsList.emplace_back(PA_Skills.MeleeWeapons);
	SkillsList.emplace_back(PA_Skills.Outdoorsman);
	SkillsList.emplace_back(PA_Skills.Repair);
	SkillsList.emplace_back(PA_Skills.Science);
	SkillsList.emplace_back(PA_Skills.SmallGuns);
	SkillsList.emplace_back(PA_Skills.Sneak);
	SkillsList.emplace_back(PA_Skills.Speech);
	SkillsList.emplace_back(PA_Skills.Steal);
	SkillsList.emplace_back(PA_Skills.Throwing);
	SkillsList.emplace_back(PA_Skills.Traps);
	SkillsList.emplace_back(PA_Skills.Unarmed);


	InitializePerks(dataHandler, mod_esm);

	LOG_INFO("Finished initializing skill variables.");

}

void InitializePerks(RE::TESDataHandler* dataHandler, std::string mod_esm) {
	// Perks
	PA_Perks.ActionBoy = dataHandler->LookupForm<RE::BGSPerk>(0x008B6F, mod_esm);
	PA_Perks.ActionGirl = dataHandler->LookupForm<RE::BGSPerk>(0x008B70, mod_esm);
	PA_Perks.ActivateFarrelSafe = dataHandler->LookupForm<RE::BGSPerk>(0x038555, mod_esm);
	PA_Perks.AdrenalineRush = dataHandler->LookupForm<RE::BGSPerk>(0x008B71, mod_esm);
	PA_Perks.AnimalFriend = dataHandler->LookupForm<RE::BGSPerk>(0x008BDC, mod_esm);
	PA_Perks.Awareness = dataHandler->LookupForm<RE::BGSPerk>(0x008B72, mod_esm);
	PA_Perks.BaseHealRate_StartupPerk = dataHandler->LookupForm<RE::BGSPerk>(0x008CB4, mod_esm);
	PA_Perks.BetterCrits = dataHandler->LookupForm<RE::BGSPerk>(0x008B73, mod_esm);
	PA_Perks.BigBookOfSciencePerk = dataHandler->LookupForm<RE::BGSPerk>(0x012F94, mod_esm);
	PA_Perks.BloodyMessTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B5F, mod_esm);
	PA_Perks.BonusHtHAttacks = dataHandler->LookupForm<RE::BGSPerk>(0x008B76, mod_esm);
	PA_Perks.BonusHtHDamage = dataHandler->LookupForm<RE::BGSPerk>(0x008B77, mod_esm);
	PA_Perks.BonusMove = dataHandler->LookupForm<RE::BGSPerk>(0x008B7A, mod_esm);
	PA_Perks.BonusRangedDmg = dataHandler->LookupForm<RE::BGSPerk>(0x008B7C, mod_esm);
	PA_Perks.BonusRateOfFire = dataHandler->LookupForm<RE::BGSPerk>(0x008B7E, mod_esm);
	PA_Perks.BruiserTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B60, mod_esm);
	PA_Perks.CautiousNature = dataHandler->LookupForm<RE::BGSPerk>(0x008B7F, mod_esm);
	PA_Perks.Champion = dataHandler->LookupForm<RE::BGSPerk>(0x0A112C, mod_esm);
	PA_Perks.ChemReliantTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B61, mod_esm);
	PA_Perks.ChemResistantTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B62, mod_esm);
	PA_Perks.ChildKiller = dataHandler->LookupForm<RE::BGSPerk>(0x0A112D, mod_esm);
	PA_Perks.Comprehension = dataHandler->LookupForm<RE::BGSPerk>(0x008B80, mod_esm);
	PA_Perks.CultOfPersonality = dataHandler->LookupForm<RE::BGSPerk>(0x008B81, mod_esm);
	PA_Perks.DeanElectronicPerk = dataHandler->LookupForm<RE::BGSPerk>(0x012F8E, mod_esm);
	PA_Perks.DemolitionExpert = dataHandler->LookupForm<RE::BGSPerk>(0x008B82, mod_esm);
	PA_Perks.DermalImpactArmor01 = dataHandler->LookupForm<RE::BGSPerk>(0x012E6F, mod_esm);
	PA_Perks.DermalImpactArmor02 = dataHandler->LookupForm<RE::BGSPerk>(0x012E70, mod_esm);
	PA_Perks.Dodger = dataHandler->LookupForm<RE::BGSPerk>(0x008B83, mod_esm);
	PA_Perks.EarlierSeq01 = dataHandler->LookupForm<RE::BGSPerk>(0x008B84, mod_esm);
	PA_Perks.EarlierSeq02 = dataHandler->LookupForm<RE::BGSPerk>(0x0139E6, mod_esm);
	PA_Perks.EarlierSeq03 = dataHandler->LookupForm<RE::BGSPerk>(0x0139E7, mod_esm);
	PA_Perks.Educated01 = dataHandler->LookupForm<RE::BGSPerk>(0x008B87, mod_esm);
	PA_Perks.Educated02 = dataHandler->LookupForm<RE::BGSPerk>(0x008B88, mod_esm);
	PA_Perks.Educated03 = dataHandler->LookupForm<RE::BGSPerk>(0x008B89, mod_esm);
	PA_Perks.Explorer = dataHandler->LookupForm<RE::BGSPerk>(0x008B8B, mod_esm);
	PA_Perks.FasterHealing = dataHandler->LookupForm<RE::BGSPerk>(0x008B8C, mod_esm);
	PA_Perks.FastMetabolismTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B63, mod_esm);
	PA_Perks.FastShotTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B64, mod_esm);
	PA_Perks.FinesseTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B65, mod_esm);
	PA_Perks.FortuneFinder = dataHandler->LookupForm<RE::BGSPerk>(0x008B8F, mod_esm);
	PA_Perks.GainAGI = dataHandler->LookupForm<RE::BGSPerk>(0x008B95, mod_esm);
	PA_Perks.GainCHR = dataHandler->LookupForm<RE::BGSPerk>(0x008B93, mod_esm);
	PA_Perks.GainEND = dataHandler->LookupForm<RE::BGSPerk>(0x008B92, mod_esm);
	PA_Perks.GainINT = dataHandler->LookupForm<RE::BGSPerk>(0x008B94, mod_esm);
	PA_Perks.GainLCK = dataHandler->LookupForm<RE::BGSPerk>(0x008B96, mod_esm);
	PA_Perks.GainPER = dataHandler->LookupForm<RE::BGSPerk>(0x008B91, mod_esm);
	PA_Perks.GainSTR = dataHandler->LookupForm<RE::BGSPerk>(0x008B90, mod_esm);
	PA_Perks.Gambler = dataHandler->LookupForm<RE::BGSPerk>(0x008B97, mod_esm);
	PA_Perks.GeckoImmuneToFire = dataHandler->LookupForm<RE::BGSPerk>(0x03E763, mod_esm);
	PA_Perks.Ghost = dataHandler->LookupForm<RE::BGSPerk>(0x008B98, mod_esm);
	PA_Perks.GiftedTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B66, mod_esm);
	PA_Perks.GoodNaturedTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B67, mod_esm);
	PA_Perks.GunsAndBulletsPerk = dataHandler->LookupForm<RE::BGSPerk>(0x008C8E, mod_esm);
	PA_Perks.Harmless = dataHandler->LookupForm<RE::BGSPerk>(0x008B99, mod_esm);
	PA_Perks.Healer = dataHandler->LookupForm<RE::BGSPerk>(0x008B9A, mod_esm);
	PA_Perks.HeaveHo = dataHandler->LookupForm<RE::BGSPerk>(0x008B9C, mod_esm);
	PA_Perks.HeavyHandedTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B68, mod_esm);
	PA_Perks.HereAndNow = dataHandler->LookupForm<RE::BGSPerk>(0x008B9F, mod_esm);
	PA_Perks.HtHEvade = dataHandler->LookupForm<RE::BGSPerk>(0x008BA0, mod_esm);
	PA_Perks.JinxedTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B69, mod_esm);
	PA_Perks.KamikazeTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B6A, mod_esm);
	PA_Perks.KarmaBeacon = dataHandler->LookupForm<RE::BGSPerk>(0x008BA2, mod_esm);
	PA_Perks.KarmaPerk = dataHandler->LookupForm<RE::BGSPerk>(0x047F96, mod_esm);
	PA_Perks.KarmaSutraMaster = dataHandler->LookupForm<RE::BGSPerk>(0x008BA1, mod_esm);
	PA_Perks.LifeGiver = dataHandler->LookupForm<RE::BGSPerk>(0x008BA3, mod_esm);
	PA_Perks.LightStep = dataHandler->LookupForm<RE::BGSPerk>(0x008BA5, mod_esm);
	PA_Perks.LivingAnatomy = dataHandler->LookupForm<RE::BGSPerk>(0x008BA6, mod_esm);
	PA_Perks.MagneticPersonality = dataHandler->LookupForm<RE::BGSPerk>(0x008BA7, mod_esm);
	PA_Perks.MasterThief = dataHandler->LookupForm<RE::BGSPerk>(0x008BA8, mod_esm);
	PA_Perks.MasterTrader = dataHandler->LookupForm<RE::BGSPerk>(0x008BA9, mod_esm);
	PA_Perks.Medic = dataHandler->LookupForm<RE::BGSPerk>(0x008BAA, mod_esm);
	PA_Perks.MoreCriticals = dataHandler->LookupForm<RE::BGSPerk>(0x008BAB, mod_esm);
	PA_Perks.MrFixit = dataHandler->LookupForm<RE::BGSPerk>(0x008BAE, mod_esm);
	PA_Perks.Mutate = dataHandler->LookupForm<RE::BGSPerk>(0x008BAF, mod_esm);
	PA_Perks.MysteriousStranger = dataHandler->LookupForm<RE::BGSPerk>(0x008BB0, mod_esm);
	PA_Perks.Negotiator = dataHandler->LookupForm<RE::BGSPerk>(0x008BB1, mod_esm);
	PA_Perks.NightVision = dataHandler->LookupForm<RE::BGSPerk>(0x008BB2, mod_esm);
	PA_Perks.OneHanderTrait = dataHandler->LookupForm<RE::BGSPerk>(0x020632, mod_esm);
	PA_Perks.PackRat = dataHandler->LookupForm<RE::BGSPerk>(0x008BB3, mod_esm);
	PA_Perks.PathFinder = dataHandler->LookupForm<RE::BGSPerk>(0x008BB4, mod_esm);
	PA_Perks.PhoenixArmorImplant01 = dataHandler->LookupForm<RE::BGSPerk>(0x012E71, mod_esm);
	PA_Perks.PhoenixArmorImplant02 = dataHandler->LookupForm<RE::BGSPerk>(0x012E72, mod_esm);
	PA_Perks.PickPocket = dataHandler->LookupForm<RE::BGSPerk>(0x008BB6, mod_esm);
	PA_Perks.PlayerChosenOnePerk = dataHandler->LookupForm<RE::BGSPerk>(0x049153, mod_esm);
	PA_Perks.Presence = dataHandler->LookupForm<RE::BGSPerk>(0x008BB7, mod_esm);
	PA_Perks.Pyromaniac = dataHandler->LookupForm<RE::BGSPerk>(0x008BBA, mod_esm);
	PA_Perks.QuickPockets = dataHandler->LookupForm<RE::BGSPerk>(0x008BBB, mod_esm);
	PA_Perks.QuickRecovery = dataHandler->LookupForm<RE::BGSPerk>(0x008BBC, mod_esm);
	PA_Perks.RadResistance = dataHandler->LookupForm<RE::BGSPerk>(0x008BBD, mod_esm);
	PA_Perks.Ranger = dataHandler->LookupForm<RE::BGSPerk>(0x008BBF, mod_esm);
	PA_Perks.ReputationBerserker = dataHandler->LookupForm<RE::BGSPerk>(0x061123, mod_esm);
	PA_Perks.ReputationChampion = dataHandler->LookupForm<RE::BGSPerk>(0x03C98D, mod_esm);
	PA_Perks.ReputationChildkiller = dataHandler->LookupForm<RE::BGSPerk>(0x03C98F, mod_esm);
	PA_Perks.ReputationGigolo = dataHandler->LookupForm<RE::BGSPerk>(0x03C98E, mod_esm);
	PA_Perks.ReputationMarried = dataHandler->LookupForm<RE::BGSPerk>(0x008C87, mod_esm);
	PA_Perks.ReputationPrizeFighter = dataHandler->LookupForm<RE::BGSPerk>(0x019FC4, mod_esm);
	PA_Perks.ReputationSeparated = dataHandler->LookupForm<RE::BGSPerk>(0x008C88, mod_esm);
	PA_Perks.ReputationSexpert = dataHandler->LookupForm<RE::BGSPerk>(0x03C990, mod_esm);
	PA_Perks.ReputationSlaver = dataHandler->LookupForm<RE::BGSPerk>(0x012C3C, mod_esm);
	PA_Perks.ReputationVirginOfTheWastes = dataHandler->LookupForm<RE::BGSPerk>(0x008C89, mod_esm);
	PA_Perks.SalesMan = dataHandler->LookupForm<RE::BGSPerk>(0x008BC0, mod_esm);
	PA_Perks.Scout = dataHandler->LookupForm<RE::BGSPerk>(0x008BC1, mod_esm);
	PA_Perks.Scrounger = dataHandler->LookupForm<RE::BGSPerk>(0x008BC2, mod_esm);
	PA_Perks.SexAppealTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B6C, mod_esm);
	PA_Perks.Sharpshooter = dataHandler->LookupForm<RE::BGSPerk>(0x008BC3, mod_esm);
	PA_Perks.SilentDeath = dataHandler->LookupForm<RE::BGSPerk>(0x008BC4, mod_esm);
	PA_Perks.SilentRunning = dataHandler->LookupForm<RE::BGSPerk>(0x008BC5, mod_esm);
	PA_Perks.SkilledTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B6D, mod_esm);
	PA_Perks.SkillValueBarter = dataHandler->LookupForm<RE::BGSPerk>(0x0140D2, mod_esm);
	PA_Perks.SkillValueBigGuns = dataHandler->LookupForm<RE::BGSPerk>(0x0140CE, mod_esm);
	PA_Perks.SkillValueEnergyWeapons = dataHandler->LookupForm<RE::BGSPerk>(0x0140CF, mod_esm);
	PA_Perks.SkillValueLockpick = dataHandler->LookupForm<RE::BGSPerk>(0x0140C8, mod_esm);
	PA_Perks.SkillValueMeleeWeapons = dataHandler->LookupForm<RE::BGSPerk>(0x0140D1, mod_esm);
	PA_Perks.SkillValueScience = dataHandler->LookupForm<RE::BGSPerk>(0x0140D3, mod_esm);
	PA_Perks.SkillValueSmallGuns = dataHandler->LookupForm<RE::BGSPerk>(0x0140CA, mod_esm);
	PA_Perks.SkillValueSneak = dataHandler->LookupForm<RE::BGSPerk>(0x0140C9, mod_esm);
	PA_Perks.SkillValueSteal = dataHandler->LookupForm<RE::BGSPerk>(0x0140D4, mod_esm);
	PA_Perks.SkillValueUnarmed = dataHandler->LookupForm<RE::BGSPerk>(0x0140D0, mod_esm);
	PA_Perks.Slayer = dataHandler->LookupForm<RE::BGSPerk>(0x008BC6, mod_esm);
	PA_Perks.SmallFrameTrait = dataHandler->LookupForm<RE::BGSPerk>(0x008B6E, mod_esm);
	PA_Perks.SmoothTalker = dataHandler->LookupForm<RE::BGSPerk>(0x008BC7, mod_esm);
	PA_Perks.Snakeater = dataHandler->LookupForm<RE::BGSPerk>(0x008BCA, mod_esm);
	PA_Perks.Sniper = dataHandler->LookupForm<RE::BGSPerk>(0x008BCC, mod_esm);
	PA_Perks.Speaker = dataHandler->LookupForm<RE::BGSPerk>(0x008BCD, mod_esm);
	PA_Perks.StoneWall = dataHandler->LookupForm<RE::BGSPerk>(0x008BCE, mod_esm);
	PA_Perks.StrongBack = dataHandler->LookupForm<RE::BGSPerk>(0x008BCF, mod_esm);
	PA_Perks.Survivalist = dataHandler->LookupForm<RE::BGSPerk>(0x008BD2, mod_esm);
	PA_Perks.SwiftLearner = dataHandler->LookupForm<RE::BGSPerk>(0x008BD3, mod_esm);
	PA_Perks.Tag = dataHandler->LookupForm<RE::BGSPerk>(0x008BD6, mod_esm);
	PA_Perks.Thief = dataHandler->LookupForm<RE::BGSPerk>(0x008BD7, mod_esm);
	PA_Perks.Toughness = dataHandler->LookupForm<RE::BGSPerk>(0x008BD8, mod_esm);
	PA_Perks.WeaponHandling = dataHandler->LookupForm<RE::BGSPerk>(0x008BDB, mod_esm);
}

// Files in ESPs.
void Initialize_TOBEMERGED_Vars(RE::TESDataHandler* dataHandler) {
	
}

void InitializeGlobalVariables(RE::TESDataHandler* dataHandler)
{
	// Globals
	PA_Globals.Karma = dataHandler->LookupForm<RE::TESGlobal>(0x0D014D, MOD_ESM);

	PA_Globals.AmmoDefaultDegradation = dataHandler->LookupForm<RE::TESGlobal>(0x001B06, CURRENT_ESP);
	
}

// ================================================================================
// Papyrus Functions

RE::ActorValueInfo* PA_SkillsPapyrus::GetSkillByName_Papyrus(std::monostate, RE::BSFixedString skillToGet)
{
	return GetSkillByName(skillToGet.c_str());
}

uint32_t PA_SkillsPapyrus::GetSkillValueByName_Papyrus(std::monostate, RE::Actor* Actor, RE::BSFixedString skillToGet)
{
	return GetAVValue(Actor, GetSkillByName(skillToGet.c_str()));
}

uint32_t PA_SkillsPapyrus::GetBaseSkillValueByName_Papyrus(std::monostate, RE::Actor* Actor, RE::BSFixedString skillToGet)
{
	return GetBaseAVValue(Actor, GetSkillByName(skillToGet.c_str()));
}

void PA_SkillsPapyrus::ModPermanentSkillValue_Papyrus(std::monostate, RE::TESForm* Form, RE::ActorValueInfo* AVToModify, float modAmount)
{
	ModPermanentSkillValue(Form, AVToModify, modAmount);
}

void PA_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus(std::monostate, RE::Actor* Actor)
{
	std::string result = "Skills: ";
	for (int i = 0; i < SkillsList.size(); i++)
	{
		auto skill = SkillsList[i];
		result += std::format("{}, {}\t|\t", skill->GetFullName(), Actor->GetActorValue(*skill));
	}

	LOG_TO_CONSOLE(result.c_str());
}

void PA_SkillsPapyrus::DEBUG_LogSkillToConsole_Papyrus(std::monostate, RE::Actor* Actor, RE::ActorValueInfo* AV)
{
	LOG_TO_CONSOLE(std::format("Skill {}, {}\n", AV->GetFullName(), Actor->GetActorValue(*AV)).c_str());
}

// ================================================================================
// Internal Functions

void GetLevelUpFormsFromGame()
{
	RE::TESDataHandler* tesDataHandler = RE::TESDataHandler::GetSingleton();

	RE::BGSListForm* perkList = tesDataHandler->LookupForm<RE::BGSListForm>(0x005D71, MOD_ESM);
	for (std::uint32_t perkEntry = 0; perkEntry < perkList->arrayOfForms.size(); perkEntry++)
	{
		RE::BGSPerk* perk = static_cast<RE::BGSPerk*>(perkList->arrayOfForms[perkEntry]);
		if (perk == nullptr)
		{
			// Entry in list is not a perk.
			REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Perk formlist is not a perk.", perkList->arrayOfForms[perkEntry]->GetFormEditorID());
			continue;
		}

		if (!perk->data.trait && perk->data.playable)
		{
			PAPerksLevelUp.push_back(perk);
		}
	}

	auto soundsList = tesDataHandler->LookupForm<RE::BGSListForm>(0x0D1EAB, MOD_ESM);
	for (std::uint32_t soundEntry = 0; soundEntry < soundsList->arrayOfForms.size(); soundEntry++) {
		RE::BGSSoundDescriptorForm* soundForm = static_cast<RE::BGSSoundDescriptorForm*>(soundsList->arrayOfForms[soundEntry]);
		if (soundForm == nullptr) {
			// Entry in list is not a sound.
			REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Sound formlist is not a sound. SkillSounds", soundsList->arrayOfForms[soundEntry]->GetFormEditorID());
			continue;
		}

		Skills::ScaleformSkillSounds.push_back(soundForm);
	}

	auto soundsList2 = tesDataHandler->LookupForm<RE::BGSListForm>(0x0D1EAA, MOD_ESM);
	for (std::uint32_t soundEntry = 0; soundEntry < soundsList2->arrayOfForms.size(); soundEntry++) {
		RE::BGSSoundDescriptorForm* soundForm = static_cast<RE::BGSSoundDescriptorForm*>(soundsList2->arrayOfForms[soundEntry]);
		if (soundForm == nullptr) {
			// Entry in list is not a sound.
			REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Sound formlist is not a sound. PerkSounds, Index: {}", soundsList2->arrayOfForms[soundEntry]->GetFormEditorID(), soundEntry);
			continue;
		}

		Skills::ScaleformPerkSounds.push_back(soundForm);
	}




	RE::BGSListForm* skillList = tesDataHandler->LookupForm<RE::BGSListForm>(0x005D70, MOD_ESM);
	for (std::uint32_t skillEntry = 0; skillEntry < skillList->arrayOfForms.size(); skillEntry++)
	{
		RE::ActorValueInfo* skill = static_cast<RE::ActorValueInfo*>(skillList->arrayOfForms[skillEntry]);
		if (skill == nullptr)
		{
			// Entry in list is not a skill.
			REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Skill formlist is not a skill. Index: {}", perkList->arrayOfForms[skillEntry]->GetFormEditorID(), skillEntry);
			continue;
		}
		PASkillsLevelUp.push_back(skill);
	}
}

float GetAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo)
{
	if (!Actor)
		return NULL;

	return Actor->GetActorValue(*valueInfo);
}

float GetBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo)
{
	if (!Actor)
		return NULL;

	return Actor->GetBaseActorValue(*valueInfo);
}

void ModBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int modAmount)
{
	if (!Actor)
		return;

	return Actor->ModBaseActorValue(*valueInfo, modAmount);
}

void SetBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int setAmount)
{
	if (!Actor)
		return;

	return Actor->SetBaseActorValue(*valueInfo, setAmount);
}

void ModPermanentSkillValue(RE::TESForm* Form, RE::ActorValueInfo* AVToModify, float modAmount)
{
	if (auto valueOwner = dynamic_cast<RE::ActorValueOwner*>(Form)) {
		valueOwner->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kTemporary, *AVToModify, modAmount);
	}
}

RE::ActorValueInfo* GetSkillByName(std::string skillName)
{
	auto result = strSkillMap.find(skillName);
	if (result == strSkillMap.end())
		return nullptr;

	return result->second;
}

float GetPlayerAVValue(RE::ActorValueInfo* valueInfo)
{
	return GetAVValue(RE::PlayerCharacter::GetSingleton(), valueInfo);
}

float GetPlayerBaseAVValue(RE::ActorValueInfo* valueInfo)
{
	return GetBaseAVValue(RE::PlayerCharacter::GetSingleton(), valueInfo);
}

void ModPlayerBaseAVValue(RE::ActorValueInfo* valueInfo, int modAmount)
{
	ModBaseAVValue(RE::PlayerCharacter::GetSingleton(), valueInfo, modAmount);
}

void SetPlayerBaseAVValue(RE::Actor* Actor, RE::ActorValueInfo* valueInfo, int setAmount)
{
	return SetBaseAVValue(RE::PlayerCharacter::GetSingleton(), valueInfo, setAmount);
}

void Skills::RegisterForSkillLink()
{
	LOG_INFO(std::format("Skills: Linking Skills from {}", MOD_ESM));

	strSkillMap.clear();
	skillsToSpecialMap.clear();
	specialToSkillsMap.clear();

	// Link Skills to their SPECIAL skills.
	RegisterLinkedAV(PA_Skills.SmallGuns, SkillOffsetsCalcFunctions::CalculateSmallGunsOffset,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.BigGuns, SkillOffsetsCalcFunctions::CalculateBigGuns,
		VanillaActorValues.Strength);

	RegisterLinkedAV(PA_Skills.EnergyWeapons, SkillOffsetsCalcFunctions::CalculateEnergyWeapons,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.Unarmed, SkillOffsetsCalcFunctions::CalculateUnarmed,
		VanillaActorValues.Strength,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.MeleeWeapons, SkillOffsetsCalcFunctions::CalculateMelee,
		VanillaActorValues.Strength,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.FirstAid, SkillOffsetsCalcFunctions::CalculateFirstAid,
		VanillaActorValues.Perception,
		VanillaActorValues.Intelligence);

	RegisterLinkedAV(PA_Skills.Doctor, SkillOffsetsCalcFunctions::CalculateDoctor,
		VanillaActorValues.Perception,
		VanillaActorValues.Intelligence);

	RegisterLinkedAV(PA_Skills.Sneak, SkillOffsetsCalcFunctions::CalculateSneak,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.Lockpick, SkillOffsetsCalcFunctions::CalculateLockpick,
		VanillaActorValues.Perception,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.Steal, SkillOffsetsCalcFunctions::CalculateSteal,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.Traps, SkillOffsetsCalcFunctions::CalculateTraps,
		VanillaActorValues.Perception,
		VanillaActorValues.Agility);

	RegisterLinkedAV(PA_Skills.Science, SkillOffsetsCalcFunctions::CalculateScience,
		VanillaActorValues.Intelligence);

	RegisterLinkedAV(PA_Skills.Repair, SkillOffsetsCalcFunctions::CalculateRepair,
		VanillaActorValues.Intelligence);

	RegisterLinkedAV(PA_Skills.Speech, SkillOffsetsCalcFunctions::CalculateSpeech,
		VanillaActorValues.Charisma);

	RegisterLinkedAV(PA_Skills.Barter, SkillOffsetsCalcFunctions::CalculateBarter,
		VanillaActorValues.Charisma);

	RegisterLinkedAV(PA_Skills.Gambling, SkillOffsetsCalcFunctions::CalculateGambling,
		VanillaActorValues.Luck);

	RegisterLinkedAV(PA_Skills.Outdoorsman, SkillOffsetsCalcFunctions::CalculateOutdoorsman,
		VanillaActorValues.Endurance,
		VanillaActorValues.Intelligence);

	RegisterLinkedAV(PA_Skills.Throwing, SkillOffsetsCalcFunctions::CalculateOutdoorsman,
		VanillaActorValues.Agility);

	// For being able to find these later by name.
	strSkillMap.emplace("Barter", PA_Skills.Barter);
	strSkillMap.emplace("BigGuns", PA_Skills.BigGuns);
	strSkillMap.emplace("Doctor", PA_Skills.Doctor);
	strSkillMap.emplace("EnergyWeapons", PA_Skills.EnergyWeapons);
	strSkillMap.emplace("FirstAid", PA_Skills.FirstAid);
	strSkillMap.emplace("Gambling", PA_Skills.Gambling);
	strSkillMap.emplace("Lockpick", PA_Skills.Lockpick);
	strSkillMap.emplace("MeleeWeapons", PA_Skills.MeleeWeapons);
	strSkillMap.emplace("Outdoorsman", PA_Skills.Outdoorsman);
	strSkillMap.emplace("Repair", PA_Skills.Repair);
	strSkillMap.emplace("Science", PA_Skills.Science);
	strSkillMap.emplace("SmallGuns", PA_Skills.SmallGuns);
	strSkillMap.emplace("Sneak", PA_Skills.Sneak);
	strSkillMap.emplace("Speech", PA_Skills.Speech);
	strSkillMap.emplace("Steal", PA_Skills.Steal);
	strSkillMap.emplace("Throwing", PA_Skills.Throwing);
	strSkillMap.emplace("Traps", PA_Skills.Traps);
	strSkillMap.emplace("Unarmed", PA_Skills.Unarmed);

}


float Skills::CalculateSkillOffset(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;


	//	(Dependant x 2) + 2 + (Dependant_02 / 2)
	auto dependants = *GetDependantAVs(&a_info);
	if (dependants.size() == 1)
		return (a_actor->GetActorValue(*dependants[0]) * 2) + 2 + ceilf(a_actor->GetActorValue(*VanillaActorValues.Luck) / 2);
	else
		return (a_actor->GetActorValue(*dependants[0]) * 2) + 2 + ceilf(a_actor->GetActorValue(*dependants[1]) / 2);
}

void Skills::RegisterLinkedAV(RE::ActorValueInfo* AV, RE::ActorValueInfo::DerivationFunction_t* CalcFunction, RE::ActorValueInfo* linkedToSPECIAL, RE::ActorValueInfo* linkedToSPECIAL_02)
{
	specialToSkillsMap[linkedToSPECIAL].push_back(AV);
	specialToSkillsMap[linkedToSPECIAL_02].push_back(AV);

	skillsToSpecialMap[AV].push_back(linkedToSPECIAL);
	skillsToSpecialMap[AV].push_back(linkedToSPECIAL_02);

	AV->derivationFunction = CalcFunction;
}

std::vector<RE::ActorValueInfo*>* Skills::GetDependantAVs(const RE::ActorValueInfo* a_info)
{
	auto result = skillsToSpecialMap.find(a_info);

	if (result == skillsToSpecialMap.end())
		return nullptr;

	return &result->second;
}

float SkillOffsetsCalcFunctions::CalculateSmallGunsOffset(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	5 + 4 * AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 5 + (a_actor->GetActorValue(*dependants[0]) * 4);
}

float SkillOffsetsCalcFunctions::CalculateBigGuns(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	2 * STR
	auto& dependants = *GetDependantAVs(&a_info);
	return 2 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateEnergyWeapons(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	2 * AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 2 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateThrowing(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	4 * AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 2 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateUnarmed(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	30 + 2 * (STR + AGL)
	auto& dependants = *GetDependantAVs(&a_info);
	return 30 + 2 * (a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]));
}

float SkillOffsetsCalcFunctions::CalculateMelee(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	20 + 2 * (STR + AGL)
	auto& dependants = *GetDependantAVs(&a_info);
	return 20 + 2 * (a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]));
}

float SkillOffsetsCalcFunctions::CalculateFirstAid(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	2 * (PE + IN)
	auto& dependants = *GetDependantAVs(&a_info);
	return 2 * a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]);
}

float SkillOffsetsCalcFunctions::CalculateDoctor(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	5 + (PE + IN)
	auto& dependants = *GetDependantAVs(&a_info);
	return 5 + a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]);
}

float SkillOffsetsCalcFunctions::CalculateSneak(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	5 + (3 * AGL)
	auto& dependants = *GetDependantAVs(&a_info);
	return 5 + (3 * a_actor->GetActorValue(*dependants[0]));
}

float SkillOffsetsCalcFunctions::CalculateLockpick(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	10 + PER + AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 10 + a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]);
}

float SkillOffsetsCalcFunctions::CalculateSteal(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	3 * AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 3 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateTraps(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	10 + PER + AGL
	auto& dependants = *GetDependantAVs(&a_info);
	return 10 + a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]);
}

float SkillOffsetsCalcFunctions::CalculateScience(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	10 + (2 * INT)
	auto& dependants = *GetDependantAVs(&a_info);
	return 10 + (2 * a_actor->GetActorValue(*dependants[0]));
}

float SkillOffsetsCalcFunctions::CalculateRepair(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info) {
	if (!a_actor)
		return 0.0f;

	//	3 + INT
	auto& dependants = *GetDependantAVs(&a_info);
	return 3 + a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateSpeech(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info) {
	if (!a_actor)
		return 0.0f;

	//	5 * CHR
	auto& dependants = *GetDependantAVs(&a_info);
	return 5 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateBarter(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info) {
	if (!a_actor)
		return 0.0f;

	//	4 * CHR
	auto& dependants = *GetDependantAVs(&a_info);
	return 4 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateGambling(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info) {
	if (!a_actor)
		return 0.0f;

	//	5 * LCK
	auto& dependants = *GetDependantAVs(&a_info);
	return 5 * a_actor->GetActorValue(*dependants[0]);
}

float SkillOffsetsCalcFunctions::CalculateOutdoorsman(const RE::ActorValueOwner* a_actor, const RE::ActorValueInfo& a_info)
{
	if (!a_actor)
		return 0.0f;

	//	2 * (END + INT)
	auto& dependants = *GetDependantAVs(&a_info);
	return 2 * (a_actor->GetActorValue(*dependants[0]) + a_actor->GetActorValue(*dependants[1]));
}