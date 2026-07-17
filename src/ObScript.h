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
#include "Prisma/Highwayman_Map.h"
#include "ItemDegradation.h"
#include "Scaleform_PArroyo.h"
#include "Shared.h"

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

		class ModCurrentWeaponCondition {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "SaveDebugTextPages") == 0;
					});

				if (it == functions.end()) {
					LOG_WARNING("Failed to reigster console command: 'SaveDebugTextPages'");
					return;
				}

				static std::array params{
					RE::SCRIPT_PARAMETER{"SkillName", RE::SCRIPT_PARAM_TYPE::kInt, true},
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				LOG_INFO("Registered 'SaveDebugTextPages' console command");
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
				int weaponCND = 0;

				auto paramsParsed = RE::Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					&weaponCND
				);

				if (!paramsParsed || weaponCND < 0 || weaponCND > 100) {
					LOG_TO_CONSOLE("Mod the current weapon's condition. mwc AMOUNT [0,100]\n");
					return true;
				}

				auto player = RE::PlayerCharacter::GetSingleton();
				auto currentWeapon = InventoryUtils::GetCurrentEquippedWeapon(player);

				if (!currentWeapon) {
					LOG_TO_CONSOLE("No weapon equipped! equip a weapon.");
					return true;
				}

				auto weaponData = ItemDegradation::WeaponConditionData(player, currentWeapon->object, currentWeapon->stackData->extra.get());

				const float previousHealth = weaponData.extraData->GetHealthPerc();

				weaponData.extraData->SetHealthPerc(weaponCND / 100.0f);

				const float conditionValue = weaponData.extraData->GetHealthPerc();

				RE::BSFixedString menuString("HUDMenu");
				if (RE::UI::GetSingleton()->GetMenuOpen(menuString)) {

					auto myHudMenu = RE::UI::GetSingleton()->GetMenu(menuString).get();
					Scaleform::GFx::Value myConditionValue[1];


					myConditionValue[0] = Scaleform::GFx::Value(conditionValue);

					const bool result = myHudMenu->uiMovie->asMovieRoot->Invoke("root.CWHUD_loader.content.SetCondition", nullptr, myConditionValue, 1);

				}

				LOG_TO_CONSOLE(std::format("Previous Condition: {}, New Condition: {}", previousHealth, conditionValue).c_str());

				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Mod the current weapon's condition. mwc AMOUNT\n"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ModWeaponCondition"sv;
			static constexpr auto SHORT_NAME = "mwc"sv;
		};

		class ShowHighwaymanMapCommand {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "TestPath") == 0;
					});

				if (it == functions.end()) {
					LOG_WARNING("Failed to reigster console command: 'TestPath'");
					return;
				}

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				//it->paramCount = static_cast<std::uint16_t>(params.size());
				//it->parameters = params.data();
				it->executeFunction = Execute;

				LOG_INFO("Registered 'TestPath' console command");
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
				
				if (PArroyo::Highwayman::g_visible)
					PArroyo::Highwayman::CloseHighwayman();
				else
					PArroyo::Highwayman::OpenUpHighwaymanMap();

				
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Show the highwayman Prisma app!"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ShowHighwayMan"sv;
			static constexpr auto SHORT_NAME = "shhwm"sv;
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
					vm->DispatchStaticCall("PArroyo:PArroyo", "ModKarma", nullptr, karmaModAmount);
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


		static void Install() {

			ModPAValueCommand::Install();
			ShowPlayerSkills::Install();
			ModCurrentWeaponCondition::Install();
			ShowHighwaymanMapCommand::Install();
		}
	}
}