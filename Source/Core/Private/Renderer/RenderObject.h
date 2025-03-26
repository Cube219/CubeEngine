#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    class RenderObject
    {
    public:
        RenderObject();
        ~RenderObject();

    private:
        Vector3 mPosition;
        Vector3 mRotation;
        Vector3 mScale;
    };
} // namespace cube
