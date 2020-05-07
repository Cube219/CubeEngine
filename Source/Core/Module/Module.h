#pragma once

#include "../CoreHeader.h"

namespace cube
{
    // extern "C" Module * CreateModule();

    struct ModuleInfo
    {
        const char* name;
        Vector<const char*> dependencies;
    };

    class CORE_EXPORT Module
    {
    public:
        Module() {}
        virtual ~Module() {}

        ModuleInfo GetInfo() const { return mInfo; }

        virtual void OnInit() = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnDestroy() = 0;

    protected:
        ModuleInfo mInfo;
    };
} // namespace cube

#define CREATE_MODULE_FUNC(moduleClass, exportMacro)     \
    extern "C" exportMacro cube::Module* CreateModule(){ \
        return new moduleClass();\
    }
