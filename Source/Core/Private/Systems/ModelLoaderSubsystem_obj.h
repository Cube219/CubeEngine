#pragma once

#include "CoreHeader.h"

#include "ModelLoaderSystem.h"

namespace cube
{
    class ModelLoaderSubsystem_obj : public ModelLoaderSubsystem
    {
    public:
        ModelLoaderSubsystem_obj();
        virtual ~ModelLoaderSubsystem_obj();

        virtual ModelResources LoadModel(const ModelPathInfo& pathInfo) override;

    private:
    };
} // namespace cube
