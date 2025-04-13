#pragma once

#include "MetalHeader.h"

#include "GAPI_Buffer.h"

namespace cube
{
    namespace gapi
    {
        class MetalBuffer : public Buffer
        {
        public:
            MetalBuffer(const BufferCreateInfo& info) :
                Buffer(info)
            {
                mTempBuffer = malloc(info.size);
            }
            virtual ~MetalBuffer()
            {
                free(mTempBuffer);
            }

            virtual void* Map() override { return mTempBuffer; }
            virtual void Unmap() override {}

        private:
            void* mTempBuffer;
        };
    } // namespace gapi
} // namespace cube
