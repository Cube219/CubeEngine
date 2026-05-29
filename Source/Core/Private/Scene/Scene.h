#pragma once

#include "CoreHeader.h"

namespace cube
{
    class Material;
    class SceneObject;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void AddSceneObject(UniquePtr<SceneObject>&& sceneObject);
        const Vector<UniquePtr<SceneObject>>& GetSceneObjects() const { return mSceneObjects; }

        void AddMaterial(SharedPtr<Material> material);

    private:
        Vector<UniquePtr<SceneObject>> mSceneObjects;
        Vector<SharedPtr<Material>> mMaterials;
    };
} // namespace cube
