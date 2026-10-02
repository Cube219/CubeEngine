#pragma once

#include "CoreHeader.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
        class TLAS;
    } // namespace gapi

    class Material;
    class RGBuilder;
    class SceneObject;

    class Scene
    {
    public:
        Scene(bool useTLAS);
        ~Scene();

        const Vector<UniquePtr<SceneObject>>& GetSceneObjects() const { return mSceneObjects; }

        void AddSceneObject(UniquePtr<SceneObject>&& sceneObject);
        void AddMaterial(SharedPtr<Material> material);

        void UpdateScene(RGBuilder& builder, Uint32 frameIndex);

    private:
        void UpdateTLAS(RGBuilder& builder, Uint32 frameIndex);

        bool mChangedSceneObjects = false;
        Vector<UniquePtr<SceneObject>> mSceneObjects;
        Vector<SharedPtr<Material>> mMaterials;

        bool mUseTLAS;
        Uint32 mNumInstancesInTLAS = 0;
        Uint64 mTLASLastBuildRenderingFrame = 0;
        SharedPtr<gapi::TLAS> mTLAS;
        // TODO: Set the size to numGPUSync or remove numGPUSync logic.
        Array<SharedPtr<gapi::Buffer>, 3> mTLASInstanceDescBuffers;
    };
} // namespace cube
