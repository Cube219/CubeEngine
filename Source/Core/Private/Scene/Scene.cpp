#include "Scene.h"

#include "SceneObject.h"

namespace cube
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        mSceneObjects.clear();
    }

    void Scene::AddSceneObject(UniquePtr<SceneObject>&& sceneObject)
    {
        mSceneObjects.emplace_back(std::move(sceneObject));
    }

    void Scene::AddMaterial(SharedPtr<Material> material)
    {
        mMaterials.push_back(material);
    }
} // namespace cube
