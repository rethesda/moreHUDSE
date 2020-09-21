#include "PCH.h"
#include "Scaleform.h"
#include "AHZScaleform.h"
#include "SKSE/API.h"
#include "AHZPapyrusMoreHud.h"
#include "HashUtil.h"

namespace Scaleform
{

    namespace SKSE
    {
        class SKSEScaleform_InstallHooks : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
            }
        };

        class SKSEScaleform_GetTargetObjectData : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessTargetObject(CAHZPlayerInfo::GetTargetRef(), args);
            }
        };

        class SKSEScaleform_GetPlayerData : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessPlayerData(a_params);
            }
        };

        class SKSEScaleform_GetIsPlayerInCombat : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                a_params.retVal->SetBoolean(CAHZPlayerInfo::GetIsInCombat());
            }
        };

        class SKSEScaleform_GetTargetEffects : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessTargetEffects(CAHZPlayerInfo::GetTargetRef(), a_params);
            }
        };

        class SKSEScaleform_GetIsBookAndWasRead : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                auto pTargetReference = CAHZPlayerInfo::GetTargetRef();

                // If the target is not valid or it can't be picked up by the player
                if (!pTargetReference) {
                    a_params.retVal->SetBoolean(false);
                    return;
                }
                a_params.retVal->SetBoolean(CAHZScaleform::GetIsBookAndWasRead(pTargetReference));
            }
        };

        class SKSEScaleform_GetArmorWeightClassString : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessArmorClass(CAHZPlayerInfo::GetTargetRef(), a_params);
            }
        };

        class SKSEScaleform_GetValueToWeightString : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessValueToWeight(CAHZPlayerInfo::GetTargetRef(), a_params);
            }
        };

        class SKSEScaleform_GetTargetWarmthRating : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                a_params.retVal->SetNumber(CAHZScaleform::GetArmorWarmthRating(CAHZPlayerInfo::GetTargetRef()));
            }
        };

        class SKSEScaleform_AHZLog : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                assert(a_params.argCount); 
                logger::trace("{}", a_params.args[0].GetString());
            }
        };

        class SKSEScaleform_GetBookSkillString : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessBookSkill(CAHZPlayerInfo::GetTargetRef(), a_params);
            }
        };

        class SKSEScaleform_GetIsValidTarget : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessValidTarget(CAHZPlayerInfo::GetTargetRef(), a_params);
            }
        };

        class SKSEScaleform_GetEnemyInformation : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                CAHZScaleform::ProcessEnemyInformation(a_params);
            }
        };

        class SKSEScaleform_IsAKnownEnchantedItem : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                auto pTargetReference = CAHZPlayerInfo::GetTargetRef();

                // If the target is not valid or it can't be picked up by the player
                if (!pTargetReference) {
                    a_params.retVal->SetNumber(0);
                    return;
                }
                a_params.retVal->SetNumber(CAHZScaleform::GetIsKnownEnchantment(pTargetReference));
            }
        };

        class SKSEScaleform_IsTargetInFormList : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                assert(a_params.args);
                assert(a_params.argCount);
                if (a_params.args[0].GetType() == RE::GFxValue::ValueType::kString) {
                    auto pTargetReference = CAHZPlayerInfo::GetTargetRef();
                    // If the target is not valid then say false
                    if (!pTargetReference) {
                        a_params.retVal->SetBoolean(false);
                        return;
                    }

                    auto keyName = string(a_params.args[0].GetString());

                    a_params.retVal->SetBoolean(PapyrusMoreHud::HasForm(keyName, pTargetReference->GetBaseObject()->formID));
                    return;
                }

                a_params.retVal->SetBoolean(false);
                return;
            }
        };

        class SKSEScaleform_IsTargetInIconList : public RE::GFxFunctionHandler
        {
        public:
            virtual void Call(Params& a_params)
            {
                assert(a_params.args);
                assert(a_params.argCount);
                if (a_params.args[0].GetType() == RE::GFxValue::ValueType::kString) {
                    auto iconName = string(a_params.args[0].GetString());

                    auto pTargetReference = CAHZPlayerInfo::GetTargetRef();
                    // If the target is not valid then say false
                    if (!pTargetReference) {
                        a_params.retVal->SetBoolean(false);
                        return;
                    }

                    const char*  name = nullptr;
                    auto pFullName = DYNAMIC_CAST(pTargetReference->GetBaseObject() ,RE::TESForm, RE::TESFullName);
                    if (pFullName)
                        name = pFullName->GetFullName();

                    // Can't get the same for the crc
                    if (!name) {
                        a_params.retVal->SetBoolean(false);
                        return;
                    }

                    auto hash = static_cast<int32_t>(SKSE::HashUtil::CRC32(name, pTargetReference->GetBaseObject()->formID & 0x00FFFFFF));

                    auto resultIconName = string(PapyrusMoreHud::GetIconName(hash));

                    if (!resultIconName.length()) {
                        a_params.retVal->SetBoolean(false);
                        return;
                    }

                    a_params.retVal->SetBoolean(resultIconName == iconName);
                    return;
                }
                a_params.retVal->SetBoolean(false);
            }
        };

    }

	void RegisterCallbacks()
	{



		logger::info("Registered all scaleform callbacks");
	}


	void RegisterCreators()
	{
		logger::info("Registered all scaleform creators");
	}
}
