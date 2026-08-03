#pragma once

#include "CoreHeader.h"

#include "Vector.h"
#include "RenderCore/Mesh.h"
#include "Systems/ModelLoader/IModelLoader.h"

namespace cube
{
    class Scene;

    class ModelLoaderSystem
    {
    public:
        ModelLoaderSystem() = delete;
        ~ModelLoaderSystem() = delete;

        static void Initialize();
        static void Shutdown();

        static void OnLoopImGUIContent();

        static SharedPtr<Scene> LoadModel(const ModelPathInfo& pathInfo);

    private:
        static void LoadModelList();
        static void LoadCurrentModelAndSet(bool resetTransform = true);

        static MeshMetadata GetMeshMetadata();

        static void UpdateModelMatrix();
        static void ResetModelTransform();

        static Vector<UniquePtr<IModelLoader>> mLoaders;

        static Vector<ModelPathInfo> mModelPathList;
        static int mCurrentSelectModelIndex;

        static Float3 mModelPosition;
        static Float3 mModelRotation;
        static float mModelScale;
        static bool mUseFloat16Vertices;
    };
} // namespace cube
