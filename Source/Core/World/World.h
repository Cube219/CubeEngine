#pragma once

#include "../CoreHeader.h"

#include <entt/entt.hpp>

#include "WorldObject.h"
#include "../Handler.h"

namespace cube
{
    class World
    {
    public:
        World(entt::registry& registry) :
            mRegistry(registry),
            mWorldObjectTable(200)
        {}
        ~World() {}

        void Initialize();
        void Shutdown();

        void Update(float dt);

        HWorldObject RegisterWorldObject(UPtr<WorldObject>&& wo);
        void UnregisterWorldObject(HWorldObject& wo);

    private:
        entt::registry& mRegistry;

        HandlerTable mWorldObjectTable;
        Vector<UPtr<WorldObject>> mWorldObjects;
    };
} // namespace cube
