﻿#include "pch.h"

#include <memory>
#include <vector>

#include "Events.h"
#include "Patches.h"
#include "Scaleform.h"
#include "version.h"

// TODO
#include "AHZConsole.h"
#include "Events.h"

// TODO

#include "RE/Skyrim.h"
#include "SKSE/API.h"
#include <Papyrus.h>
#include "AHZExternalFormTable.h"
#include "AHZVanillaFormTable.h"

using namespace moreHUD;
//using namespace std;

// Just initialize to start routing to the console window
Debug::CAHZDebugConsole theDebugConsole;

namespace
{
    void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            {
                // First load the built-in (Known Vanilla) ACTI Forms and VM Script Variables
                CAHZVanillaFormTable::LoadACTIForms(CAHZFormLookup::Instance());
                CAHZVanillaFormTable::LoadVMVariables(CAHZFormLookup::Instance());

                // Second load any addional forms added externally
                logger::info("Processing any third party .mhud files that may exist...");

                // Read all .mhuf files and load in the lookup tables
                auto skyrimDataPath = CAHZUtilities::GetSkyrimDataPath();

                // Get all .mhud files from the skyrim data folder
                auto mHudFiles = CAHZUtilities::GetMHudFileList(skyrimDataPath);

                if (!mHudFiles.size()) {
                    logger::info("No third party .mHud files where detected.");
                } else {
                    // Load the external ACTI Forms and VM Script Variables
                    CAHZExternalFormTable::LoadACTIForms(CAHZFormLookup::Instance(), mHudFiles);
                    CAHZExternalFormTable::LoadVMVariables(CAHZFormLookup::Instance(), mHudFiles);

                    logger::info("%d third party .mHud file(s) processed", mHudFiles.size());
                }

                logger::info("Third party .mHud file processing completed.");

                logger::info("Registering Events"sv);
                Events::Install();
            }
            break;
        }
    }

    bool InitLog()
    {
    #ifndef NDEBUG
        auto                    msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        auto                    console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        spdlog::sinks_init_list sink_list = { msvc_sink, console_sink };
        auto                    log = std::make_shared<spdlog::logger>("multi_sink", sink_list.begin(), sink_list.end());
        log->set_level(spdlog::level::trace);
        spdlog::flush_every(std::chrono::seconds(3));
        spdlog::set_default_logger(std::move(log));
    #else
        auto path = logger::log_directory();
        if (!path) {
            //stl::report_and_fail("Failed to find standard logging directory"sv);
            return false;
        }

        *path /= "moreHUDSE.log"sv;

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
    #endif  

        return true;
    }    
}

extern "C"
{
#if defined(SE_BUILD) || defined(VR_BUILD)
    DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
    {
        if (!InitLog())
        {
            return false;
        }

        a_info->infoVersion = SKSE::PluginInfo::kVersion;
        a_info->name = "Ahzaab's moreHUD Plugin";
        a_info->version = Version::ASINT;

        if (a_skse->IsEditor()) {
            logger::critical("Loaded in editor, marking as incompatible!"sv);
            return false;
        }

        const auto ver = a_skse->RuntimeVersion();
        if (ver <= SKSE::RUNTIME_1_5_39) {
            logger::critical("Unsupported runtime version {}!"sv, ver.string().c_str());
            return false;
        }

        return true;
    }
#else
    DLLEXPORT constinit auto SKSEPlugin_Version = []() {
        SKSE::PluginVersionData v{};
        v.pluginVersion = Version::ASINT;
        v.PluginName("Ahzaab's moreHUD Plugin"sv);
        v.AuthorName("Ahzaab"sv);
        v.CompatibleVersions({ SKSE::RUNTIME_LATEST });
        v.UsesAddressLibrary(true);
        return v;
    }();
#endif


    DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
    {
        // while (!IsDebuggerPresent())
        // {
        //   Sleep(10);
        // }

        // Sleep(1000 * 2);

        try {
#if !defined(SE_BUILD) && !defined(VR_BUILD)
            if (!InitLog())
            {
                return false;
            }
#endif
            logger::info("moreHUD loading"sv);
            logger::info("moreHUD v{}"sv, Version::NAME);

            SKSE::Init(a_skse);

            SKSE::AllocTrampoline(1 << 6);

            std::vector<size_t> offsets = {
            0x0053EC60,   //Wand
            0x0060F1D0, //sentient
            0x003D0FC0, //actor soul
            0x008C0940, // item description
            0x008AFE70, // enemy
            0x3D91A0,
            0x8B2880};

             for (auto &offset: offsets){
                 //auto o2i = REL::IDDatabase::Offset2ID();
                 //auto id1 = o2i(offset);
                 auto id1 = REL::IDDatabase::get().id2offset(offset);
                 logger::info("offset: {:x}, id {}"sv, offset, id1);
             }

            auto messaging = SKSE::GetMessagingInterface();
            if (!messaging->RegisterListener("SKSE", MessageHandler)) {
                logger::critical("Could not register MessageHandler"sv);
                return false;
            }
            logger::info("registered listener"sv);

            if (!moreHUD::Papyrus::Register()) {
                logger::critical("Could not register papyrus functions"sv);
                return false;
            }

            logger::info("Installing patched"sv);
            Patches::Install();

            logger::info("Registering Callbacks"sv);
            Scaleform::RegisterCallbacks();

            logger::info("moreHUD loaded"sv);

        } catch (const std::exception& e) {
            logger::critical(e.what());
            return false;
        } catch (...) {
            logger::critical("caught unknown exception"sv);
            return false;
        }

        return true;
    }
};
