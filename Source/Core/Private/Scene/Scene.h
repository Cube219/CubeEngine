#pragma once

#include "CoreHeader.h"

namespace cube
{
    namespace gapi
    {
        class BLAS;
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

        void UpdateScene(RGBuilder& builder, Uint64 renderingFrame);

        SharedPtr<gapi::TLAS> GetTLAS() const { return mTLAS; }
        Vector<SharedPtr<gapi::BLAS>> GetBLASes() const;

    private:
        void UpdateSceneObjects(Uint64 renderingFrame);
        void UpdateTLAS(RGBuilder& builder, Uint64 renderingFrame);

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
