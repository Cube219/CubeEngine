#include "ModuleManager.h"

#include "Module.h"
#include "../LogWriter.h"
#include "../Assertion.h"
#include "Platform/Platform.h"

namespace cube
{
    HashMap<String, SPtr<Module>> ModuleManager::mModuleLookupTable;
    Vector<ModuleNode> ModuleManager::mModules;

    void ModuleManager::Initialize()
    {
    }

    void ModuleManager::Shutdown()
    {
        for(auto& m : mModules) {
            m.module->OnDestroy();
        }

        // mModuleLookupTable must be freed before mModules
        // Because the module cannot be freed after its DLib has been unloaded
        mModuleLookupTable.clear();
        mModules.clear();
    }

    void ModuleManager::LoadModule(StringView moduleName)
    {
        auto findIter = mModuleLookupTable.find(moduleName.data());
        if(findIter != mModuleLookupTable.end()) {
            CUBE_LOG(LogType::Error, "Already module name '{0}' loaded.", moduleName);
            return;
        }

        ModuleNode node;
        node.dlib = platform::Platform::LoadDLib(moduleName);

        using CreateModuleFunction = Module* (*)();
        auto createModuleFunction = RCast(CreateModuleFunction)(node.dlib->GetFunction(CUBE_T("CreateModule")));
        node.module = SPtr<Module>(createModuleFunction());

        mModules.push_back(node);
        mModuleLookupTable[moduleName.data()] = node.module;
    }

    void ModuleManager::InitModules()
    {
        for(auto& m : mModules) {
            m.module->OnInit();
        }
    }

    void ModuleManager::UpdateAllModules(float dt)
    {
        for(auto& m : mModules) {
            m.module->OnUpdate(dt);
        }
    }

    SPtr<Module> ModuleManager::GetModule(StringView moduleName)
    {
        auto findIter = mModuleLookupTable.find(moduleName.data());
        CHECK(findIter != mModuleLookupTable.end(), "Cannot find module '{0}'.", moduleName);

        return findIter->second;
    }
} // namespace cube
