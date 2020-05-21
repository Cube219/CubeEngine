#include "WorldObject.h"

#include "../Core.h"

namespace cube
{
    HWorldObject WorldObject::Create()
    {
        return Core::GetWorld().RegisterWorldObject(std::make_unique<WorldObject>());
    }

    void WorldObject::Destroy()
    {
        auto h = GetHandler();
        Core::GetWorld().UnregisterWorldObject(h);
    }
} // namespace cube
