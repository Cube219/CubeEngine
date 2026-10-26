#pragma once

#include "IModelLoader.h"

namespace cube
{
    class GLTFModelLoader : public IModelLoader
    {
    public:
        GLTFModelLoader() = default;
        virtual ~GLTFModelLoader() = default;

        virtual ModelType GetModelType() const override { return ModelType::glTF; }

        virtual const Vector<ModelPathInfo>& GetModelList() override;

        virtual SharedPtr<Scene> LoadModel(const ModelPathInfo& pathInfo, const MeshMetadata& meshMetadata) override;

    private:
        Vector<ModelPathInfo> mModelList;
    };
} // namespace cube
