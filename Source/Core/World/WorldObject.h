#pragma once

#include "../CoreHeader.h"

#include <entt/entity/entity.hpp>

#include "Utility/Vector.h"
#include "Utility/Matrix.h"
#include "Utility/MatrixUtility.h"
#include "Utility/Math.h"
#include "../Handler.h"

namespace cube
{
    struct Transform
    {
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;

        Matrix GetModelMatrix() const
        {
            Matrix m = MatrixUtility::GetTranslation(position);
            m *= MatrixUtility::GetRotationXYZ(rotation * (Math::Pi / 180.0f));
            m *= MatrixUtility::GetScale(scale);

            return m;
        }
    };

    class CORE_EXPORT WorldObject : public Handlable
    {
    public:
        static HWorldObject Create();

    public:
        WorldObject() {}
        virtual ~WorldObject() {}

        HWorldObject GetHandler() const { return mMyHandler; }

        void Destroy();

    private:
        friend class World;

        entt::entity mEntity;
    };
} // namespace cube
