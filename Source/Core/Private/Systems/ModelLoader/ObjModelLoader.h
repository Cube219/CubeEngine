#pragma once

#include "IModelLoader.h"

namespace cube
{
    class ObjModelLoader : public IModelLoader
    {
    public:
        ObjModelLoader() = default;
        virtual ~ObjModelLoader() = default;

        virtual ModelType GetModelType() const override { return ModelType::Obj; }

        virtual const Vector<ModelPathInfo>& GetModelList() override;

        virtual SharedPtr<Scene> LoadModel(const ModelPathInfo& pathInfo, const MeshMetadata& meshMetadata) override;

    private:
        Vector<ModelPathInfo> mModelList;
    };
} // namespace cube
