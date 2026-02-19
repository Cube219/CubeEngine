#pragma once

#include "CoreHeader.h"

#include "CubeString.h"
#include "FileSystem.h"

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
        static void ResetModelScale();

        static Vector<ModelPathInfo> mModelPathList;
        static int mCurrentSelectModelIndex;

        static float mModelScale;
        static bool mUseFloat16Vertices;
    };
} // namespace cube
