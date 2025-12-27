#pragma once

#include "MetalHeader.h"

#include "GAPI_Buffer.h"

namespace cube
{
    class MetalDevice;
    
    namespace gapi
    {
        class MetalBuffer : public Buffer
        {
        public:
            MetalBuffer(const BufferCreateInfo& info, MetalDevice& device);
            virtual ~MetalBuffer();

            virtual void* Map() override;
            virtual void Unmap() override;

            id<MTLBuffer> GetMTLBuffer() const { return mBuffer; }

        private:
            id<MTLBuffer> mBuffer;
        };
    } // namespace gapi
} // namespace cube
