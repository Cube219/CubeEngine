#pragma once

#include "MetalHeader.h"

#include "GAPI_Buffer.h"

namespace cube
{
    class MetalDevice;
    
    namespace gapi
    {
        class MetalBuffer : public Buffer, public std::enable_shared_from_this<MetalBuffer>
        {
        public:
            MetalBuffer(const BufferCreateInfo& info, MetalDevice& device);
            virtual ~MetalBuffer();

            virtual SharedPtr<BufferSRV> CreateSRV(const BufferSRVCreateInfo& createInfo) override;
            virtual SharedPtr<BufferUAV> CreateUAV(const BufferUAVCreateInfo& createInfo) override;

            virtual void* Map() override;
            virtual void Unmap() override;

            id<MTLBuffer> GetMTLBuffer() const { return mBuffer; }
            MTLResourceOptions GetMTLResourceOptions() const { return mMTLResourceOptions; }

        private:
            MetalDevice& mDevice;

            id<MTLBuffer> mBuffer;

            MTLResourceOptions mMTLResourceOptions;
        };

        class MetalBufferSRV : public BufferSRV
        {
        public:
            MetalBufferSRV(MetalDevice& device, const BufferSRVCreateInfo& createInfo, SharedPtr<MetalBuffer> metalBuffer);
            virtual ~MetalBufferSRV() = default;

            id<MTLBuffer> GetParentMTLBuffer() const { return mParentMTLBuffer; }
            id<MTLTexture> GetTypedTextureBuffer() const { return mTypedTextureBuffer; }

        private:
            id<MTLBuffer> mParentMTLBuffer;

            id<MTLTexture> mTypedTextureBuffer;
        };

        class MetalBufferUAV : public BufferUAV
        {
        public:
            MetalBufferUAV(MetalDevice& device, const BufferUAVCreateInfo& createInfo, SharedPtr<MetalBuffer> metalBuffer);
            virtual ~MetalBufferUAV() = default;

            id<MTLBuffer> GetParentMTLBuffer() const { return mParentMTLBuffer; }
            id<MTLTexture> GetTypedTextureBuffer() const { return mTypedTextureBuffer; }

        private:
            id<MTLBuffer> mParentMTLBuffer;

            id<MTLTexture> mTypedTextureBuffer;
        };
    } // namespace gapi
} // namespace cube
