#pragma once

#include "CoreHeader.h"

namespace cube
{
    class GAPI;
    class Renderer;

    namespace gapi
    {
        class CommandList;
        class Fence;
    } // namespace gapi

    class BufferManager
    {
    public:
        BufferManager(Renderer& renderer);
        ~BufferManager() = default;

        void Initialize(GAPI* gapi);
        void Shutdown();

        SharedPtr<gapi::CommandList> GetBufferInitCommandList() const { return mBufferInitCommandList; }
        SharedPtr<gapi::Fence> GetBufferInitFence() const { return mBufferInitFence; }
        Uint64 GetAndMoveBufferInitFenceValue()
        {
            mBufferInitFenceLastValue++;
            return mBufferInitFenceLastValue;
        }

    private:
        GAPI* mGAPI;
        Renderer& mRenderer;

        SharedPtr<gapi::CommandList> mBufferInitCommandList;
        SharedPtr<gapi::Fence> mBufferInitFence;
        Uint64 mBufferInitFenceLastValue = 0;
    };
} // namespace cube
