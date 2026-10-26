#pragma once

#include "Resource.h"

#include "GAPI_Buffer.h"

namespace cube
{
    namespace gapi
    {
        class Buffer;
    } // namespace gapi

    class BufferResource : public Resource
    {
    public:
        BufferResource(const gapi::BufferCreateInfo& createInfo);
        virtual ~BufferResource() = default;

        SharedPtr<gapi::Buffer> GetGAPIBuffer() const
        {
            return mBuffer;
        }

    private:
        SharedPtr<gapi::Buffer> mBuffer;
    };
} // namespace cube
