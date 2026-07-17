#include "LevelUpMenu.h"
#include <string.h>
#include <Windows.h>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <future>
#include <string>
#include <thread>
#include <variant>
#include <RE/A/ActorValueInfo.h>
#include <RE/A/ACTOR_VALUE_MODIFIER.h>
#include <RE/B/BGSPerk.h>
#include <RE/B/BSScript_IVirtualMachine.h>
#include <RE/B/BSSoundHandle.h>
#include <RE/B/BSStringT.h>
#include <RE/B/BSTArray.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESGlobal.h>
#include <RE/U/UI.h>
#include <RE/U/UIMessageQueue.h>
#include <RE/U/UI_MESSAGE_TYPE.h>
#include <Scaleform/G/GFx_ASMovieRootBase.h>
#include <Scaleform/G/GFx_Movie.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/P/Ptr.h>
#include <REX/LOG.h>
#include <GameForms.h>
#include <IGlobalConfig.h>
#include <PerkHelpers.h>
#include <Serialization.h>
#include <Shared.h>
#include <skills.h>

namespace PArroyo_Menus {
	namespace LevelUpMenu {

		bool bForceLevelUp;

		RE::BSSoundHandle previousSoundHandle;

		float perkPoints;

		std::uint32_t menuModeType = 0;
		std::uint32_t tagPointsValue = 0;

		bool allowRetag = false;

		RE::BSTArray<RE::ActorValueInfo*> scaleformSkills;

		namespace InitialValues {
			std::uint32_t SmallGuns;
			std::uint32_t BigGuns;
			std::uint32_t EnergyWeapons;
			std::uint32_t Unarmed;
			std::uint32_t MeleeWeapons;
			std::uint32_t Throwing;
			std::uint32_t FirstAid;
			std::uint32_t Doctor;
			std::uint32_t Sneak;
			std::uint32_t Lockpick;
			std::uint32_t Steal;
			std::uint32_t Traps;
			std::uint32_t Science;
			std::uint32_t Repair;
			std::uint32_t Speech;
			std::uint32_t Barter;
			std::uint32_t Gambling;
			std::uint32_t Outdoorsman;
		}

		namespace TempModValues {
			std::uint32_t SmallGuns;
			std::uint32_t BigGuns;
			std::uint32_t EnergyWeapons;
			std::uint32_t Unarmed;
			std::uint32_t MeleeWeapons;
			std::uint32_t Throwing;
			std::uint32_t FirstAid;
			std::uint32_t Doctor;
			std::uint32_t Sneak;
			std::uint32_t Lockpick;
			std::uint32_t Steal;
			std::uint32_t Traps;
			std::uint32_t Science;
			std::uint32_t Repair;
			std::uint32_t Speech;
			std::uint32_t Barter;
			std::uint32_t Gambling;
			std::uint32_t Outdoorsman;
		}
		void GetINIOptions()
		{
			std::string myINI = "./Data/CapitalWasteland.ini";
			bool forceLevelUp;
			forceLevelUp = GetPrivateProfileInt("Gameplay", "bForceLevelUp", 1, myINI.c_str());

			bForceLevelUp = forceLevelUp;
		}
		bool GetForcedLevelUp()
		{
			return bForceLevelUp;
		}

		void InitializeValues()
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

			InitialValues::SmallGuns = playerCharacter->GetPermanentActorValue(*PA_Skills.SmallGuns);
			InitialValues::BigGuns = playerCharacter->GetPermanentActorValue(*PA_Skills.BigGuns);
			InitialValues::EnergyWeapons = playerCharacter->GetPermanentActorValue(*PA_Skills.EnergyWeapons);
			InitialValues::Unarmed = playerCharacter->GetPermanentActorValue(*PA_Skills.Unarmed);
			InitialValues::MeleeWeapons = playerCharacter->GetPermanentActorValue(*PA_Skills.MeleeWeapons);
			InitialValues::Throwing = playerCharacter->GetPermanentActorValue(*PA_Skills.Throwing);
			InitialValues::FirstAid = playerCharacter->GetPermanentActorValue(*PA_Skills.FirstAid);
			InitialValues::Doctor = playerCharacter->GetPermanentActorValue(*PA_Skills.Doctor);
			InitialValues::Sneak = playerCharacter->GetPermanentActorValue(*PA_Skills.Sneak);
			InitialValues::Lockpick = playerCharacter->GetPermanentActorValue(*PA_Skills.Lockpick);
			InitialValues::Steal = playerCharacter->GetPermanentActorValue(*PA_Skills.Steal);
			InitialValues::Traps = playerCharacter->GetPermanentActorValue(*PA_Skills.Traps);
			InitialValues::Science = playerCharacter->GetPermanentActorValue(*PA_Skills.Science);
			InitialValues::Repair = playerCharacter->GetPermanentActorValue(*PA_Skills.Repair);
			InitialValues::Speech = playerCharacter->GetPermanentActorValue(*PA_Skills.Speech);
			InitialValues::Barter = playerCharacter->GetPermanentActorValue(*PA_Skills.Barter);
			InitialValues::Gambling = playerCharacter->GetPermanentActorValue(*PA_Skills.Gambling);
			InitialValues::Outdoorsman = playerCharacter->GetPermanentActorValue(*PA_Skills.Outdoorsman);
		}

		float GetInitialActorValue(RE::ActorValueInfo* a_skill)
		{
			if (a_skill == PA_Skills.Barter)
			{
				return InitialValues::Barter;
			}
			else if (a_skill == PA_Skills.EnergyWeapons)
			{
				return InitialValues::EnergyWeapons;
			}
			else if (a_skill == PA_Skills.Traps)
			{
				return InitialValues::Traps;
			}
			else if (a_skill == PA_Skills.BigGuns)
			{
				return InitialValues::BigGuns;
			}
			else if (a_skill == PA_Skills.Lockpick)
			{
				return InitialValues::Lockpick;
			}
			else if (a_skill == PA_Skills.Doctor)
			{
				return InitialValues::Doctor;
			}
			else if (a_skill == PA_Skills.MeleeWeapons)
			{
				return InitialValues::MeleeWeapons;
			}
			else if (a_skill == PA_Skills.Repair)
			{
				return InitialValues::Repair;
			}
			else if (a_skill == PA_Skills.Science)
			{
				return InitialValues::Science;
			}
			else if (a_skill == PA_Skills.Sneak)
			{
				return InitialValues::Sneak;
			}
			else if (a_skill == PA_Skills.Speech)
			{
				return InitialValues::Speech;
			}
			else if (a_skill == PA_Skills.SmallGuns)
			{
				return InitialValues::SmallGuns;
			}
			else if (a_skill == PA_Skills.Unarmed)
			{
				return InitialValues::Unarmed;
			}
			else if (a_skill == PA_Skills.FirstAid)
			{
				return InitialValues::FirstAid;
			}
			else if (a_skill == PA_Skills.Steal)
			{
				return InitialValues::Steal;
			}
			else if (a_skill == PA_Skills.Gambling)
			{
				return InitialValues::Gambling;
			}
			else if (a_skill == PA_Skills.Outdoorsman)
			{
				return InitialValues::Outdoorsman;
			}
			else if (a_skill == PA_Skills.Throwing)
			{
				return InitialValues::Throwing;
			}
			else
			{
				return -1;
			}
		}
		void PopulateSkillEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, RE::ActorValueInfo* a_skill)
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			float buffedValue = playerCharacter->GetActorValue(*a_skill);

			Scaleform::GFx::Value skillEntry;
			a_movieRoot->CreateObject(&skillEntry);

			RE::BSStringT<char> description;
			a_skill->GetDescription(description);

			skillEntry.SetMember("text", a_skill->fullName.c_str());
			skillEntry.SetMember("editorID", a_skill->GetFormEditorID());
			skillEntry.SetMember("description", description.c_str());
			skillEntry.SetMember("formid", a_skill->formID);

			float baseValue = GetInitialActorValue(a_skill);
			float value = playerCharacter->GetPermanentActorValue(*a_skill);
			skillEntry.SetMember("value", (std::uint32_t)baseValue);
			skillEntry.SetMember("baseValue", (std::uint32_t)baseValue);
			skillEntry.SetMember("buffedValue", (std::uint32_t)value);

			skillEntry.SetMember("alreadyTagged", PArroyoSerialization::IsSkillTagged(a_skill->formID));
			skillEntry.SetMember("tagged", PArroyoSerialization::IsSkillTagged(a_skill->formID));
			a_destination->PushBack(skillEntry);
		}
		bool ProcessSkillsList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			Scaleform::GFx::Value argumentsArray[3];
			a_movieRoot->CreateArray(&argumentsArray[0]);

			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.SmallGuns);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.BigGuns);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.EnergyWeapons);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Unarmed);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.MeleeWeapons);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Throwing);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.FirstAid);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Doctor);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Sneak);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Lockpick);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Steal);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Traps);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Science);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Repair);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Speech);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Barter);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Gambling);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Outdoorsman);

			argumentsArray[1] = PArroyoSerialization::GetSkillPoints();
			argumentsArray[2] = RE::PlayerCharacter::GetSingleton()->GetLevel();
			a_movieRoot->Invoke("root.Menu_mc.onLevelUpStart", nullptr, argumentsArray, 3);

			return true;
		}
		bool ProcessTagSkillsList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, std::uint32_t a_tagSkills, bool a_retag)
		{
			Scaleform::GFx::Value argumentsArray[3];
			a_movieRoot->CreateArray(&argumentsArray[0]);

			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.SmallGuns);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.BigGuns);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.EnergyWeapons);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Unarmed);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.MeleeWeapons);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Throwing);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.FirstAid);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Doctor);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Sneak);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Lockpick);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Steal);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Traps);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Science);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Repair);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Speech);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Barter);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Gambling);
			PopulateSkillEntry(&argumentsArray[0], a_movieRoot, PA_Skills.Outdoorsman);

			argumentsArray[1] = a_tagSkills;
			argumentsArray[2] = a_retag;

			a_movieRoot->Invoke("root.Menu_mc.onTagSkillsStart", nullptr, argumentsArray, 3);

			return true;
		}
		void PopulateSpecialEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, RE::ActorValueInfo* a_skill)
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

			Scaleform::GFx::Value skillEntry;
			a_movieRoot->CreateObject(&skillEntry);

			RE::BSStringT<char> description;
			a_skill->GetDescription(description);

			skillEntry.SetMember("text", a_skill->fullName.c_str());
			skillEntry.SetMember("editorID", a_skill->GetFormEditorID());
			skillEntry.SetMember("description", description.c_str());
			skillEntry.SetMember("formid", a_skill->formID);

			float baseAVValue = playerCharacter->GetBaseActorValue(*a_skill);
			float avValue = playerCharacter->GetActorValue(*a_skill);

			skillEntry.SetMember("value", (std::uint32_t)baseAVValue);
			skillEntry.SetMember("baseValue", (std::uint32_t)baseAVValue);
			skillEntry.SetMember("buffedValue", (std::uint32_t)avValue);

			a_destination->PushBack(skillEntry);
		}
		bool ProcessSpecialList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			Scaleform::GFx::Value argumentsArray[1];
			a_movieRoot->CreateArray(&argumentsArray[0]);

			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Strength);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Perception);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Endurance);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Charisma);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Intelligence);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Agility);
			PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, VanillaActorValues.Luck);

			switch (menuModeType) {
			case kSpecialRespec:
				a_movieRoot->Invoke("root.Menu_mc.onSpecialRespecStart", nullptr, argumentsArray, 1);
				break;
			case kIntenseTraining:
				a_movieRoot->Invoke("root.Menu_mc.onIntenseTrainingStart", nullptr, argumentsArray, 1);
				break;
			default:
				break;
			}

			return true;
		}
		void PopulatePerkEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, RE::BGSPerk* a_perk, bool a_eligibleOnly)
		{
			PArroyo::PerkHelpers::AvailablePerk currentPerk = PArroyo::PerkHelpers::GetAvailablePerk(a_perk);

			if (currentPerk.perk != nullptr)
			{
				REX::DEBUG("Processing perk: {}", currentPerk.perk->GetFormEditorID());

				PArroyo::PerkHelpers::PerkData temporaryPerkData = PArroyo::PerkHelpers::GetPerkRequirements(currentPerk.perk);

				Scaleform::GFx::Value perkEntry;
				a_movieRoot->CreateObject(&perkEntry);
				perkEntry.SetMember("text", currentPerk.perk->fullName.c_str());
				perkEntry.SetMember("reqs", temporaryPerkData.requirementsString.c_str());
				perkEntry.SetMember("filterFlag", temporaryPerkData.filterFlag);
				perkEntry.SetMember("iselig", temporaryPerkData.isEligible);
				perkEntry.SetMember("isHighLevel", temporaryPerkData.isHighLevel);
				perkEntry.SetMember("reqLevel", temporaryPerkData.requiredLevel);
				perkEntry.SetMember("qname", a_perk->formID);
				perkEntry.SetMember("formid", currentPerk.perk->formID);
				perkEntry.SetMember("level", currentPerk.level);
				perkEntry.SetMember("SWFPath", temporaryPerkData.SWFPath.c_str());
				perkEntry.SetMember("numranks", currentPerk.numRanks);
				perkEntry.SetMember("type", PerkTypes::kDefault);
				perkEntry.SetMember("ranksinfo", "");

				RE::BSStringT<char> description;
				currentPerk.perk->GetDescription(description);

				perkEntry.SetMember("description", description.c_str());
				perkEntry.SetMember("isTagged", temporaryPerkData.isTagged);

				if (a_eligibleOnly)
				{
					if (temporaryPerkData.isEligible && temporaryPerkData.isAllowable)
					{
						a_destination->PushBack(perkEntry);
					}
				}
				else
				{
					if (!temporaryPerkData.isEligible && temporaryPerkData.isAllowable)
					{
						a_destination->PushBack(perkEntry);
					}
				}
			}
		}
		bool ProcessPerkList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			Scaleform::GFx::Value argumentsArray[2];
			a_movieRoot->CreateArray(&argumentsArray[0]);

			// Eligible perks iterated through first to sort at top.
			for (std::uint32_t p1 = 0; p1 < Skills::PAPerksLevelUp.size(); p1++)
			{
				RE::BGSPerk* perk = Skills::PAPerksLevelUp.at(p1);
				PopulatePerkEntry(&argumentsArray[0], a_movieRoot, perk, true);
			}

			// Ineligible perks iterated through last to sort at bottom.
			for (std::uint32_t p2 = 0; p2 < Skills::PAPerksLevelUp.size(); p2++)
			{
				RE::BGSPerk* perk = Skills::PAPerksLevelUp.at(p2);
				PopulatePerkEntry(&argumentsArray[0], a_movieRoot, perk, false);
			}

			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

			argumentsArray[1] = playerCharacter->perkCount;

			a_movieRoot->Invoke("root.Menu_mc.onSwitchToPerks", nullptr, argumentsArray, 2);
			return true;
		}
		std::uint32_t GetSkillArrayIndexByEditorID(const char* myEditorID)
		{
			if (myEditorID == "00PA_Barter")
			{
				return SkillArray::Barter;
			}
			else if (myEditorID == "00PA_BigGuns")
			{
				return SkillArray::BigGuns;
			}
			else if (myEditorID == "00PA_EnergyWeapons")
			{
				return SkillArray::EnergyWeapons;
			}
			else if (myEditorID == "00PA_Explosives")
			{
				return SkillArray::Traps;
			}
			else if (myEditorID == "00PA_Lockpick")
			{
				return SkillArray::Lockpick;
			}
			else if (myEditorID == "00PA_Medicine")
			{
				return SkillArray::Doctor;
			}
			else if (myEditorID == "00PA_MeleeWeapons")
			{
				return SkillArray::MeleeWeapons;
			}
			else if (myEditorID == "00PA_Repair")
			{
				return SkillArray::Repair;
			}
			else if (myEditorID == "00PA_Science")
			{
				return SkillArray::Science;
			}
			else if (myEditorID == "00PA_SmallGuns")
			{
				return SkillArray::SmallGuns;
			}
			else if (myEditorID == "00PA_Sneak")
			{
				return SkillArray::Sneak;
			}
			else if (myEditorID == "00PA_Speech")
			{
				return SkillArray::Speech;
			}
			else if (myEditorID == "00PA_Unarmed")
			{
				return SkillArray::Unarmed;
			}
			else if (myEditorID == "00PA_FirstAid")
			{
				return SkillArray::FirstAid;
			}
			else if (myEditorID == "00PA_Steal")
			{
				return SkillArray::Steal;
			}
			else if (myEditorID == "00PA_Gambling")
			{
				return SkillArray::Gambling;
			}
			else if (myEditorID == "00PA_Outdoorsman")
			{
				return SkillArray::Outdoorsman;
			}
			else if (myEditorID == "00PA_Throwing")
			{
				return SkillArray::Throwing;
			}
			else
			{
				return -1;
			}
		}

		float GetSkillPointsToAdd(std::uint16_t levelsToProgress)
		{
			// Fallout 3 formula is 10 + base intelligence
			// Extra 3 points if the player has "Educated" perk
			// Using a global in 'FalloutCascaida.esm' we can adjust the base value.

			RE::TESGlobal* baseSkillPointsGlobal = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x0D1BF7, MOD_ESM);
			float skillPointsValue = 0;

			if (baseSkillPointsGlobal)
			{
				skillPointsValue = baseSkillPointsGlobal->value;
			}
			else
			{
				skillPointsValue = 10;
			}

			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();

			float playerIntelligence = playerCharacter->GetBaseActorValue(*VanillaActorValues.Intelligence);

			skillPointsValue = (skillPointsValue + playerIntelligence);

			// @TODO: They have 3 perks for educated.
			RE::BGSPerk* educatedPerk = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSPerk>(0x008B87, MOD_ESM);

			if (educatedPerk)
			{
				if (playerCharacter->GetPerkRank(educatedPerk) != 0)
				{
					skillPointsValue = (skillPointsValue + 3);
				}
			}

			return skillPointsValue * levelsToProgress;
		}
		bool CanLevelUpMenuBeShown()
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			if (playerCharacter->IsInCombat())
			{
				return false;
			}

			if (Shared::InMenuMode())
			{
				return false;
			}

			if (RE::IsPlayerInDialogue())
			{
				return false;
			}

			if (Shared::IsXPMetervisible())
			{
				return false;
			}

			return true;
		}
		void ShowLvlUpMenu()
		{
			RE::UIMessageQueue* uiMessageQueue = RE::UIMessageQueue::GetSingleton();
			if (RE::UI* ui = RE::UI::GetSingleton())
			{
				if (ui->menuMap.contains("CWLevelUpMenu"))
				{
					uiMessageQueue->AddMessage("CWLevelUpMenu", RE::UI_MESSAGE_TYPE::kShow);
				}
			}
		}

		void WaitForLevelUpReady()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			if (!CanLevelUpMenuBeShown())
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));

				std::async(std::launch::async, WaitForLevelUpReady);
				return;
			}
			else
			{
				ShowLvlUpMenu();
			}
		}

		void ModPerkCount(std::int8_t a_count)
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			std::int8_t result = playerCharacter->perkCount;

			if (a_count < -result)
			{
				result = 0;
			}
			else
			{
				result = result + a_count;
				if (result > 255)
				{
					result = 255;
				}
			}
			playerCharacter->SetPerkCount(result);
		}

		void SetSkillTagged(RE::ActorValueInfo* a_skill, bool a_tag)
		{
			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			auto formID = a_skill->formID;

			if (a_tag)
			{
				if (!PArroyoSerialization::IsSkillTagged(formID))
				{
					PArroyoSerialization::SetSkillTagged(formID);
					playerCharacter->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, *a_skill, 15);
				}
			}
			else
			{
				if (PArroyoSerialization::IsSkillTagged(formID))
				{
					PArroyoSerialization::RemoveSkillTagged(formID);
					playerCharacter->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, *a_skill, -15);
				}
			}
		}

		void HandleLevelUp(std::uint16_t levelsToProgress)
		{
			float pointsToAdd = GetSkillPointsToAdd(levelsToProgress);

			PArroyoSerialization::ModSkillPoints(pointsToAdd);

			Skills::PAPerksLevelUp.clear();
			Skills::PASkillsLevelUp.clear();
			Skills::ScaleformSkillSounds.clear();

			GetLevelUpFormsFromGame();

			PArroyoSerialization::SetReadyToLevelUp(true);

			std::thread LevelUpWait(WaitForLevelUpReady);
			LevelUpWait.detach();
		}

		void ModSkillByName(std::string a_skillName, std::uint32_t a_value, std::uint32_t a_baseValue)
		{
			std::string skillNameString = a_skillName;
			skillNameString.erase(std::remove_if(skillNameString.begin(), skillNameString.end(), isspace), skillNameString.end());

			RE::ActorValueInfo* skill = GetSkillByName(skillNameString);
			const char* skillEditorID = skill->GetFormEditorID();

			std::uint32_t modValue = (a_value - a_baseValue);

			if (skillEditorID == "00PA_Barter")
			{
				TempModValues::Barter = modValue;
			}
			else if (skillEditorID == "00PA_EnergyWeapons")
			{
				TempModValues::EnergyWeapons = modValue;
			}
			else if (skillEditorID == "00PA_Explosives")
			{
				TempModValues::Traps = modValue;
			}
			else if (skillEditorID == "00PA_BigGuns")
			{
				TempModValues::BigGuns = modValue;
			}
			else if (skillEditorID == "00PA_Lockpick")
			{
				TempModValues::Lockpick = modValue;
			}
			else if (skillEditorID == "00PA_Medicine")
			{
				TempModValues::Doctor = modValue;
			}
			else if (skillEditorID == "00PA_MeleeWeapons")
			{
				TempModValues::MeleeWeapons = modValue;
			}
			else if (skillEditorID == "00PA_Repair")
			{
				TempModValues::Repair = modValue;
			}
			else if (skillEditorID == "00PA_Science")
			{
				TempModValues::Science = modValue;
			}
			else if (skillEditorID == "00PA_Sneak")
			{
				TempModValues::Sneak = modValue;
			}
			else if (skillEditorID == "00PA_Speech")
			{
				TempModValues::Speech = modValue;
			}
			else if (skillEditorID == "00PA_SmallGuns")
			{
				TempModValues::SmallGuns = modValue;
			}
			else if (skillEditorID == "00PA_Unarmed")
			{
				TempModValues::Unarmed = modValue;
			}
			else if (skillEditorID == "00PA_FirstAid")
			{
				TempModValues::FirstAid = modValue;
			}
			else if (skillEditorID == "00PA_Steal")
			{
				TempModValues::Steal = modValue;
			}
			else if (skillEditorID == "00PA_Gambling")
			{
				TempModValues::Gambling = modValue;
			}
			else if (skillEditorID == "00PA_Outdoorsman")
			{
				TempModValues::Outdoorsman = modValue;
			}
			else if (skillEditorID == "00PA_Throwing")
			{
				TempModValues::Throwing = modValue;
			}

			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			playerCharacter->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, *skill, modValue);
		}

		void RevertSkill(RE::ActorValueInfo* skill)
		{
			const char* editorID = skill->GetFormEditorID();

			std::uint32_t modValue;

			if (editorID == "00PA_Barter")
			{
				modValue = TempModValues::Barter;
				TempModValues::Barter = 0;
			}
			else if (editorID == "00PA_EnergyWeapons")
			{
				modValue = TempModValues::EnergyWeapons;
				TempModValues::EnergyWeapons = 0;
			}
			else if (editorID == "00PA_Explosives")
			{
				modValue = TempModValues::Traps;
				TempModValues::Traps = 0;
			}
			else if (editorID == "00PA_BigGuns")
			{
				modValue = TempModValues::BigGuns;
				TempModValues::BigGuns = 0;
			}
			else if (editorID == "00PA_Lockpick")
			{
				modValue = TempModValues::Lockpick;
				TempModValues::Lockpick = 0;
			}
			else if (editorID == "00PA_Medicine")
			{
				modValue = TempModValues::Doctor;
				TempModValues::Doctor = 0;
			}
			else if (editorID == "00PA_MeleeWeapons")
			{
				modValue = TempModValues::MeleeWeapons;
				TempModValues::MeleeWeapons = 0;
			}
			else if (editorID == "00PA_Repair")
			{
				modValue = TempModValues::Repair;
				TempModValues::Repair = 0;
			}
			else if (editorID == "00PA_Science")
			{
				modValue = TempModValues::Science;
				TempModValues::Science = 0;
			}
			else if (editorID == "00PA_Sneak")
			{
				modValue = TempModValues::Sneak;
				TempModValues::Sneak = 0;
			}
			else if (editorID == "00PA_Speech")
			{
				modValue = TempModValues::Speech;
				TempModValues::Speech = 0;
			}
			else if (editorID == "00PA_SmallGuns")
			{
				modValue = TempModValues::SmallGuns;
				TempModValues::SmallGuns = 0;
			}
			else if (editorID == "00PA_Unarmed")
			{
				modValue = TempModValues::Unarmed;
				TempModValues::Unarmed = 0;
			}
			else if (editorID == "00PA_FirstAid")
			{
				modValue = TempModValues::FirstAid;
				TempModValues::FirstAid = 0;
			}
			else if (editorID == "00PA_Steal")
			{
				modValue = TempModValues::Steal;
				TempModValues::Steal = 0;
			}
			else if (editorID == "00PA_Gambling")
			{
				modValue = TempModValues::Gambling;
				TempModValues::Gambling = 0;
			}
			else if (editorID == "00PA_Outdoorsman")
			{
				modValue = TempModValues::Outdoorsman;
				TempModValues::Outdoorsman = 0;
			}
			else if (editorID == "00PA_Throwing")
			{
				modValue = TempModValues::Throwing;
				TempModValues::Throwing = 0;
			}

			RE::PlayerCharacter* playerCharacter = RE::PlayerCharacter::GetSingleton();
			playerCharacter->ModActorValue(RE::ACTOR_VALUE_MODIFIER::kPermanent, *skill, modValue);
		}

		void CheckForLevelUp()
		{
			if (PArroyoSerialization::IsReadyToLevelUp())
			{
				std::thread LevelUpWait(WaitForLevelUpReady);
				LevelUpWait.detach();
			}
		}

		void OnLevelUpStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			ProcessSkillsList(a_movieRoot);
		}

		void OnTagSkillsStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			ProcessTagSkillsList(a_movieRoot, tagPointsValue, allowRetag);
		}

		void OnSpecialRespecStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			ProcessSpecialList(a_movieRoot);
		}

		void OnIntenseTrainingStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			ProcessSpecialList(a_movieRoot);
		}

		void HandleLevelUpMenuOpen(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
		{
			previousSoundHandle.Stop();
			switch (menuModeType)
			{
			case kLevelUp:
				OnLevelUpStart(a_movieRoot);
				break;

			case kTagSkills:
				OnTagSkillsStart(a_movieRoot);
				break;

			case kSpecialRespec:
				OnSpecialRespecStart(a_movieRoot);
				break;

			case kIntenseTraining:
				OnIntenseTrainingStart(a_movieRoot);
				break;

			default:
				break;
			}
		}

		void CompleteLevelUp()
		{
			switch (menuModeType)
			{
			case kLevelUp:
				PArroyoSerialization::SetReadyToLevelUp(false);
				PArroyoSerialization::SetSkillPoints(0);
				REX::DEBUG("'CompleteLevelUp' - 'kLevelUp'");
				break;

			case kTagSkills:
				tagPointsValue = 0;
				allowRetag = false;
				REX::DEBUG("'CompleteLevelUp' - 'kTagSkills'");
				break;

			case kSpecialRespec:
				REX::DEBUG("'CompleteLevelUp' - 'kSpecialRespec'");
				break;

			case kIntenseTraining:
				REX::DEBUG("'CompleteLevelUp' - 'kIntenseTraining'");
				break;
			default:
				break;
			}

			menuModeType = 0;
		}

		bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
		{
			Scaleform::GFx::Value currentSWFPath;
			// Register native code handlers.
			if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
			{
				if (_stricmp(currentSWFPath.GetString(), "Interface/CWLevelUpMenu.swf") == 0)
				{
					Scaleform::GFx::Value bgsCodeObj;
					a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

					Shared::RegisterFunction<OpenMenu>(&bgsCodeObj, a_view->asMovieRoot, "OpenMenu");
					Shared::RegisterFunction<SetSkills>(&bgsCodeObj, a_view->asMovieRoot, "SetSkills");
					Shared::RegisterFunction<ResetSkills>(&bgsCodeObj, a_view->asMovieRoot, "ResetSkills");
					Shared::RegisterFunction<ResetTagSkills>(&bgsCodeObj, a_view->asMovieRoot, "ResetTagSkills");
					Shared::RegisterFunction<BackToSkills>(&bgsCodeObj, a_view->asMovieRoot, "BackToSkills");
					Shared::RegisterFunction<UpdatePerkMenu>(&bgsCodeObj, a_view->asMovieRoot, "UpdatePerkMenu");
					Shared::RegisterFunction<AddPerks>(&bgsCodeObj, a_view->asMovieRoot, "AddPerks");
					Shared::RegisterFunction<TagSkills>(&bgsCodeObj, a_view->asMovieRoot, "TagSkills");
					Shared::RegisterFunction<LearnSpecial>(&bgsCodeObj, a_view->asMovieRoot, "LearnSpecial");
					Shared::RegisterFunction<ResetSpecial>(&bgsCodeObj, a_view->asMovieRoot, "ResetSpecial");
					Shared::RegisterFunction<PlaySkillSound>(&bgsCodeObj, a_view->asMovieRoot, "PlaySkillSound");
					Shared::RegisterFunction<PlayPerkSound>(&bgsCodeObj, a_view->asMovieRoot, "PlayPerkSound");
					Shared::RegisterFunction<CloseMenu>(&bgsCodeObj, a_view->asMovieRoot, "CloseMenu");
					Shared::RegisterFunction<PlayUISound>(&bgsCodeObj, a_view->asMovieRoot, "PlayUISound");

					a_view->asMovieRoot->Invoke("root.Menu_mc.onCodeObjCreate", nullptr, nullptr, 0);

					REX::DEBUG("CWLevelUpMenu.swf scaleform registeration...");
				}
				return true;
			}
			return false;
		}

		void RegisterMenu()
		{
			if (RE::UI* ui = RE::UI::GetSingleton())
			{
				if (!ui->menuMap.contains("CWLevelUpMenu"))
				{
					ui->RegisterMenu("CWLevelUpMenu", CWLevelUpMenu::Create);
					REX::DEBUG("Registered 'CWLevelUpMenu'.");
				}
			}
		}

		namespace LevelUpMenu_Papyrus
		{
			void OpenLevelUpMenu_Papyrus(std::monostate)
			{
				menuModeType = LevelTypes::kLevelUp;
				ShowLvlUpMenu();
			}

			void OpenTagSkillsMenu_Papyrus(std::monostate, std::uint32_t tagPoints, bool reTag)
			{
				menuModeType = LevelTypes::kTagSkills;
				tagPointsValue = tagPoints;
				allowRetag = reTag;
				ShowLvlUpMenu();
			}

			void OpenSpecialRespecMenu_Papyrus(std::monostate)
			{
				menuModeType = LevelTypes::kSpecialRespec;
				ShowLvlUpMenu();
			}

			void OpenIntenseTrainingMenu_Papyrus(std::monostate)
			{
				menuModeType = LevelTypes::kIntenseTraining;
				ShowLvlUpMenu();
			}

			bool GetSkillTagged_Papyrus(std::monostate, RE::ActorValueInfo* mySkill)
			{
				return PArroyoSerialization::IsSkillTagged(mySkill->GetFormID());
			}

			void SetSkillTagged_Papyrus(std::monostate, RE::ActorValueInfo* mySkill, bool bTag)
			{
				SetSkillTagged(mySkill, bTag);
			}

			bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
			{
				vm->BindNativeMethod("PArroyo:PArroyo", "OpenLevelUpMenu", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::OpenLevelUpMenu_Papyrus, false, false);
				vm->BindNativeMethod("PArroyo:PArroyo", "OpenTagSkillsMenu", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::OpenTagSkillsMenu_Papyrus, false, false);
				vm->BindNativeMethod("PArroyo:PArroyo", "OpenSpecialRespecMenu", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::OpenSpecialRespecMenu_Papyrus, false, false);
				vm->BindNativeMethod("PArroyo:PArroyo", "OpenIntenseTrainingMenu", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::OpenIntenseTrainingMenu_Papyrus, false, false);
				vm->BindNativeMethod("PArroyo:PArroyo", "GetSkillTagged", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::GetSkillTagged_Papyrus, false, false);
				vm->BindNativeMethod("PArroyo:PArroyo", "SetSkillTagged", PArroyo_Menus::LevelUpMenu::LevelUpMenu_Papyrus::SetSkillTagged_Papyrus, false, false);
				return true;
			}
		}
	}
}