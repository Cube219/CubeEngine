#include "World.h"

#include "../Assertion.h"

namespace cube
{
    void World::Initialize()
    {
    }

    void World::Shutdown()
    {
        mWorldObjectTable.ReleaseAll();
        mWorldObjects.clear();
    }

    void World::Update(float dt)
    {
    }

    HWorldObject World::RegisterWorldObject(UPtr<WorldObject>&& wo)
    {
        CHECK(wo->GetID() == Uint64InvalidValue, "Failed to register GameObject. It is already registered. (id: {})", wo->GetID());

        wo->mEntity = mRegistry.create();
        mRegistry.emplace<Transform>(wo->mEntity, Vector3::Zero(), Vector3::Zero(), Vector3(1.0f, 1.0f, 1.0f));


        mWorldObjects.push_back(std::move(wo));
        HWorldObject hwo = mWorldObjectTable.CreateNewHandler(mWorldObjects.back().get());

        return hwo;
    }

    void World::UnregisterWorldObject(HWorldObject& wo)
    {
        mRegistry.destroy(wo->mEntity);

        WorldObject* pWo = mWorldObjectTable.ReleaseHandler(wo);
        auto findIter = std::find_if(mWorldObjects.cbegin(), mWorldObjects.cend(), [pWo](auto& element) {
            return element.get() == pWo;
        });
        CHECK(findIter != mWorldObjects.cend(), "Cannot unregister WorldObject. It is not registered.");
        mWorldObjects.erase(findIter);
    }
} // namespace cube
