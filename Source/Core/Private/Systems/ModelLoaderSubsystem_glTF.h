#pragma once

#include "CoreHeader.h"

#include "ModelLoaderSystem.h"

namespace cube
{
    class ModelLoaderSubsystem_glTF : public ModelLoaderSubsystem
    {
    public:
        ModelLoaderSubsystem_glTF();
        virtual ~ModelLoaderSubsystem_glTF();

        virtual ModelResources LoadModel(const ModelPathInfo& pathInfo) override;
    };
} // namespace cube
