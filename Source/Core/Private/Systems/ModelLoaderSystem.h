#pragma once

#include "CoreHeader.h"

#include "CubeString.h"

namespace cube
{
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

    class ModelLoaderSystem
    {
    public:
        ModelLoaderSystem() = delete;
        ~ModelLoaderSystem() = delete;

        static void Initialize();
        static void Shutdown();

        static void OnLoopImGUI();

        static SharedPtr<MeshData> LoadModel(ModelType type, StringView path);

    private:
        static void LoadModelList();
        static void LoadCurrentModelAndSet();

        static SharedPtr<MeshData> LoadModel_glTF(StringView path);

        static void UpdateModelMatrix();
        static void ResetModelScale();

        static Vector<ModelPathInfo> mModelPathList;
        static int mCurrentSelectModelIndex;

        static float mModelScale;
    };
} // namespace cube
