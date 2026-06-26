#pragma once

#include "CoreHeader.h"

#include "CubeString.h"
#include "FileSystem.h"
#include "Vector.h"
#include "Renderer/Mesh.h"

namespace cube
{
    class Scene;

    enum class ModelType
    {
        glTF,
        Obj
    };

    struct ModelPathInfo
    {
        ModelType type;
        AnsiString name;
        platform::FilePath path;
        Vector3 position = Vector3::Zero();
        Vector3 rotation = Vector3::Zero();
        Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
    };

    class IModelLoader
    {
    public:
        IModelLoader() = default;
        virtual ~IModelLoader() = default;

        virtual ModelType GetModelType() const = 0;

        // Return this loader's available models.
        virtual const Vector<ModelPathInfo>& GetModelList() = 0;

        virtual SharedPtr<Scene> LoadModel(const ModelPathInfo& pathInfo, const MeshMetadata& meshMetadata) = 0;
    };
} // namespace cube
