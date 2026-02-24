#pragma once

#include "CoreHeader.h"

#include "CubeString.h"
#include "FileSystem.h"
#include "Vector.h"

namespace cube
{
    class Material;
    class MeshData;

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

    struct ModelResources
    {
        SharedPtr<MeshData> mesh = nullptr;
        Vector<SharedPtr<Material>> materials;
    };

    class ModelLoaderSystem
    {
    public:
        ModelLoaderSystem() = delete;
        ~ModelLoaderSystem() = delete;

        static void Initialize();
        static void Shutdown();

        static void OnLoopImGUI();

        static ModelResources LoadModel(const ModelPathInfo& pathInfo);

    private:
        static void LoadModelList();
        static void LoadCurrentModelAndSet();

        static ModelResources LoadModel_glTF(const ModelPathInfo& pathInfo);
        static ModelResources LoadModel_Obj(const ModelPathInfo& pathInfo);

        static void UpdateModelMatrix();
        static void ResetModelTransform();

        static Vector<ModelPathInfo> mModelPathList;
        static int mCurrentSelectModelIndex;

        static Float3 mModelPosition;
        static Float3 mModelRotation;
        static float mModelScale;
        static bool mUseFloat16Vertices;
    };
} // namespace cube
