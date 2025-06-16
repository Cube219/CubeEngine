#pragma once

#include "CoreHeader.h"

#include "CubeString.h"

namespace cube
{
    class Material;
    class MeshData;

    enum class ModelType
    {
        glTF
    };

    struct ModelPathInfo
    {
        ModelType type;
        AnsiString name;
        String path;
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

        static void UpdateModelMatrix();
        static void ResetModelScale();

        static Vector<ModelPathInfo> mModelPathList;
        static int mCurrentSelectModelIndex;

        static float mModelScale;
    };
} // namespace cube
