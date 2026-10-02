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

        void AddSceneObject(UniquePtr<SceneObject>&& sceneObject);
        const Vector<UniquePtr<SceneObject>>& GetSceneObjects() const { return mSceneObjects; }

        void AddMaterial(SharedPtr<Material> material);

        void BuildTLAS(RGBuilder& builder, Uint32 frameIndex);

    private:
        Vector<UniquePtr<SceneObject>> mSceneObjects;
        Vector<SharedPtr<Material>> mMaterials;

        bool mUseTLAS;
        bool mDirtyTLAS; // TODO
        Uint32 mNumInstancesInTLAS = 0;
        SharedPtr<gapi::TLAS> mTLAS;
        // TODO: Set the size to numGPUSync or remove numGPUSync logic.
        Array<SharedPtr<gapi::Buffer>, 3> mTLASInstanceDescBuffers;
    };
} // namespace cube
