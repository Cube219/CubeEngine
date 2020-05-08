#pragma once

#include "../CoreHeader.h"

#include "Platform/FileSystem.h"
#include "../Allocator/FrameAllocator.h"

namespace cube
{
    class CORE_EXPORT ResourceImporter
    {
    public:
        ResourceImporter() {}
        virtual ~ResourceImporter() {}

        virtual Resource* Import(SPtr<platform::File>& file, const FrameJsonValue& info) = 0;

        StringView GetName() const { return mName; }

    protected:
        String mName;
    };
} // namespace cube
