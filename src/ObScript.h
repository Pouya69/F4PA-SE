#pragma once
#include "debugLog.h"
#include <RE/S/SCRIPT_PARAMETER.h>
#include <RE/T/TESObjectREFR.h>
#include <string.h>
#include <algorithm>
#include <RE/S/Script.h>
#include <RE/S/ScriptLocals.h>
#include <RE/S/SCRIPT_FUNCTION.h>
#include <cstdint>
#include <array>
#include <RE/S/SCRIPT_PARAM_TYPE.h>
#include "skills.h"
#include <string>
#include <string_view>
#include <variant>
#include <RE/P/PlayerCharacter.h>
#include <format>
#include "GameForms.h"

namespace PArroyo {
	namespace ObScript {

		// ShowPlayerSkills SkillName ====> Will show the skillName requested.
		class ShowPlayerSkills {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "ShowNodes") == 0;
					});

				if (it == functions.end()) {
					LOG_WARNING("Failed to reigster console command: 'ShowNodes'");
					return;
				}

				static std::array params{
					RE::SCRIPT_PARAMETER{"SkillName", RE::SCRIPT_PARAM_TYPE::kChar, true},
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				LOG_INFO("Registered 'ShowNodes' console command");
			}

		private:
			static bool Execute(
				const RE::SCRIPT_PARAMETER* a_parameters,
				const char* a_compiledParams,
				RE::TESObjectREFR* a_refObject,
				RE::TESObjectREFR* a_container,
				RE::Script* a_script,
				RE::ScriptLocals* a_scriptLocals,
				float&,
				std::uint32_t& a_offset)
			{
				std::array<char, 0x200> skillName = {'\0'};

				auto paramsParsed = RE::Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					skillName.data()
				);

				if (!paramsParsed || skillName[0] == '\0') {
					PA_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus(std::monostate(), RE::PlayerCharacter::GetSingleton());
					return true;
				}

				auto skill = GetSkillByName(skillName.data());

				if (!skill) {
					LOG_TO_CONSOLE("Skill not found: " + *skillName.data());
					return true;
				}

				PA_SkillsPapyrus::DEBUG_LogSkillToConsole_Papyrus(std::monostate(), RE::PlayerCharacter::GetSingleton(), skill);

				

				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Show the current player skills.\nadd a param to show a specific skill."sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ShowPlayerSkills"sv;
			static constexpr auto SHORT_NAME = "shpk"sv;
		};

		class ModPAValueCommand {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "ShowPivot") == 0;
					});

				if (it == functions.end()) {
					LOG_WARNING("Failed to reigster console command: 'ShowPivot'");
					return;
				}

				static std::array params{
					RE::SCRIPT_PARAMETER{"skillName", RE::SCRIPT_PARAM_TYPE::kChar, true},
					RE::SCRIPT_PARAMETER{"KarmaAmount", RE::SCRIPT_PARAM_TYPE::kInt, true},
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				LOG_INFO("Registered 'ShowPivot' console command");
			}

		private:
			static bool Execute(
				const RE::SCRIPT_PARAMETER* a_parameters,
				const char* a_compiledParams,
				RE::TESObjectREFR* a_refObject,
				RE::TESObjectREFR* a_container,
				RE::Script* a_script,
				RE::ScriptLocals* a_scriptLocals,
				float&,
				std::uint32_t& a_offset)
			{
				int karmaModAmount = 0;
				std::array<char, 0x200> skillName = { '\0' };

				auto paramsParsed = RE::Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					skillName.data(),
					&karmaModAmount
				);

				if (!paramsParsed || skillName[0] == '\0' || karmaModAmount == 0) {
					LOG_TO_CONSOLE("Mods the player's karma.\nModPAValue SKILL/KARMA 100 OR ModPAValue SKILL/KARMA -100");
					// CW_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus(std::monostate(), RE::PlayerCharacter::GetSingleton());
					return true;
				}

				const float oldKarma = PA_Globals.Karma->value;

				auto vm = RE::GameVM::GetSingleton()->GetVM();
				// PA_Globals.Karma->value += karmaModAmount;

				std::string nameS = skillName.data();

				if (_stricmp(nameS.c_str(), "karma") == 0) {
					vm->DispatchStaticCall("PArroyo:PArroyo", "ModPlayerKarma", nullptr, karmaModAmount);
				}
				else {
					auto skillAV = GetSkillByName(nameS);
					if (skillAV) {
						auto player = RE::PlayerCharacter::GetSingleton();
						player->SetBaseActorValue(*skillAV, karmaModAmount);

						// ModPermanentSkillValue(player, skillAV, karmaModAmount);
						//auto s = std::format("'{}' new value for player: {}", GetBaseAVValue()).c_str();
						//LOG_TO_CONSOLE();
					}
					else {
						auto s = std::format("Skill '{}' is not a valid CWSkill.", nameS).c_str();
						LOG_TO_CONSOLE(s);
					}
				}


				// LOG_TO_CONSOLE(std::format("Old Karma: {}, New Karma: {}", oldKarma, PA_Globals.Karma->value).c_str());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Mods the player's skill/karma.\nModPAValue 100 OR ModPAValue -100"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ModPAValue"sv;
			static constexpr auto SHORT_NAME = "mcwv"sv;
		};

		class ModCurrentWeaponCommand {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "ToggleEventLog") == 0;
					});

				if (it == functions.end()) {
					LOG_WARNING("Failed to reigster console command: 'ToggleEventLog'");
					return;
				}

				static std::array params{
					RE::SCRIPT_PARAMETER{"skillName", RE::SCRIPT_PARAM_TYPE::kChar, true}
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				LOG_INFO("Registered 'ToggleEventLog' console command");
			}

		private:
			static bool Execute(
				const RE::SCRIPT_PARAMETER* a_parameters,
				const char* a_compiledParams,
				RE::TESObjectREFR* a_refObject,
				RE::TESObjectREFR* a_container,
				RE::Script* a_script,
				RE::ScriptLocals* a_scriptLocals,
				float&,
				std::uint32_t& a_offset)
			{
				float karmaModAmount = 0;

				auto paramsParsed = RE::Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					&karmaModAmount
				);

				if (karmaModAmount < 0.0f || karmaModAmount > 100.0f) {
					LOG_TO_CONSOLE("Weapon Condition has to be from [0, 100]\n");
					// CW_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus(std::monostate(), RE::PlayerCharacter::GetSingleton());
					return true;
				}

				RE::TESObjectWEAP* weaponObject = nullptr;
				RE::ExtraDataList* extraData = nullptr;

				auto player = RE::PlayerCharacter::GetSingleton();

				for (RE::BGSInventoryItem& inventoryItem : player->inventoryList->data) {
					if (!inventoryItem.IsEquipped(0))
						continue;
					if (!inventoryItem.object || inventoryItem.object->GetFormType() != RE::ENUM_FORM_ID::kWEAP)
						continue;

					auto weaponObj = static_cast<RE::TESObjectWEAP*>(inventoryItem.object);

					if (weaponObj) {
						weaponObject = weaponObj;
						extraData = inventoryItem.stackData->extra.get();
						break;
					}

				}

				if (!weaponObject || !extraData) {
					return true;
				}

				const float oldKarma = extraData->GetHealthPerc();
				double finalP = karmaModAmount / 100.0f;
				extraData->SetHealthPerc(finalP);


				LOG_TO_CONSOLE(std::format("Old Condition: {}, New Condition: {}", oldKarma * 100, extraData->GetHealthPerc()).c_str());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Mods the current weapon condition."sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ModCurrentWeaponCondition"sv;
			static constexpr auto SHORT_NAME = "mcwc"sv;
		};

		static void Install() {
			ModPAValueCommand::Install();
			ShowPlayerSkills::Install();
			ModCurrentWeaponCommand::Install();
		}
	}
}