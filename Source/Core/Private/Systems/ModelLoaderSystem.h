#pragma once

#include "CoreHeader.h"

#include "CubeString.h"

namespace cube
{
    class Material;
    class MeshData;

    enum class ModelType
    {
        glTF,
        obj
    };

    enum class ModelPathType
    {
        glTFSampleAsset,
        DefaultModels
    };

    struct ModelPathInfo
    {
        ModelPathType pathType;
        ModelType modelType;
        AnsiString name;
        Vector<String> pathList;
    };

    struct ModelResources
    {
        SharedPtr<MeshData> mesh = nullptr;
        Vector<SharedPtr<Material>> materials;
    };

    class ModelLoaderSubsystem
    {
    public:
        ModelLoaderSubsystem(const char* name)
            : mName(name)
        {}
        virtual ~ModelLoaderSubsystem() = default;

        virtual ModelResources LoadModel(const ModelPathInfo& pathInfo) = 0;

        const char* GetName() const { return mName; }

    private:
        const char* mName;
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
        static void UpdateModelPathList();
        static void LoadCurrentModelAndSet();

        static void UpdateModelMatrix();
        static void ResetModelScale();

        static Map<ModelType, UniquePtr<ModelLoaderSubsystem>> mSubsystems;

        static Vector<ModelPathInfo> mModelPathInfoList;
        static int mCurrentSelectModelIndex;

        static float mModelScale;
    };
} // namespace cube
