#pragma once

#include "EngineHeader.h"

#include "GAPI_AccelerationStructure.h"

namespace cube
{
    namespace gapi
    {
        class CommandList;
        class Buffer;
    } // namespace gapi

    class Material;
    class RGBuilder;
    class SceneObject;

    class RayTracingScene;

    class Scene
    {
    public:
        Scene(bool useRayTracingScene);
        ~Scene();

        const Vector<UniquePtr<SceneObject>>& GetSceneObjects() const { return mSceneObjects; }

        void AddSceneObject(UniquePtr<SceneObject>&& sceneObject);
        void AddMaterial(SharedPtr<Material> material);

        void UpdateScene(RGBuilder& builder, Uint64 renderingFrame);

        RayTracingScene* GetRayTracingScene() const;

    private:
        void UpdateSceneObjects(Uint64 renderingFrame);

        Vector<UniquePtr<SceneObject>> mSceneObjects;
        Vector<SharedPtr<Material>> mMaterials;

        UniquePtr<RayTracingScene> mRayTracingScene;
    };

    class RayTracingScene
    {
    public:
        RayTracingScene(Scene& scene);
        ~RayTracingScene();

        SharedPtr<gapi::TLAS> GetTLAS() const { return mTLAS; }

        void UpdateTLAS(RGBuilder& builder, Uint64 renderingFrame);

    private:
        Scene& mScene;

        Vector<gapi::BLASInstance> mBLASInstances;

        SharedPtr<gapi::TLAS> mTLAS;
        // TODO: Set the size to numGPUSync or remove numGPUSync logic.
        Array<SharedPtr<gapi::Buffer>, 3> mTLASInstanceDescBuffers;
    };
} // namespace cube
