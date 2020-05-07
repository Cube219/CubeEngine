#pragma once

#include "../CoreHeader.h"

#include "Platform/DLib.h"

namespace cube
{
    // TODO: dependency에 따라서 Module들 병렬로 처리
    //       지금은 그냥 넣은 순서대로 처리중
    struct ModuleNode
    {
        enum class State
        {
            NotRun, Running, Finished
        };

        SPtr<platform::DLib> dlib;
        SPtr<Module> module;
        Uint32 remainDependencyNum;
        State state;
    };

    class CORE_EXPORT ModuleManager
    {
    public:
        ModuleManager() = delete;
        ~ModuleManager() = delete;
        ModuleManager(const ModuleManager& other) = delete;
        ModuleManager& operator=(const ModuleManager& rhs) = delete;

        static void Initialize();
        static void ShutDown();

        static void LoadModule(StringView moduleName);

        static void InitModules();
        static void UpdateAllModules(float dt);

        static SPtr<Module> GetModule(StringView moduleName);
        template <typename T>
        static SPtr<T> GetModule(StringView moduleName)
        {
            return DPCast(T)(GetModule(moduleName));
        }

    private:
        static HashMap<String, SPtr<Module>> mModuleLookupTable;
        static Vector<ModuleNode> mModules;
    };
} // namespace cube
