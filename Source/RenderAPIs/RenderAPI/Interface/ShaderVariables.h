#pragma once

#include "../RenderAPIHeader.h"

#include "Core/Allocator/FrameAllocator.h"

namespace cube
{
    namespace rapi
    {
        struct ShaderVariableInfo
        {
            ShaderVariableType type;
            Uint32 size;
            Uint32 count;

            const char* debugName = "";
        };

        class ShaderVariables
        {
        public:
            ShaderVariables() = default;
            virtual ~ShaderVariables() = default;

            virtual void UpdateVariable(Uint32 index, void* pData, Uint32 size) = 0;
            virtual void UpdateVariable(Uint32 index, SPtr<TextureView>& textureView) = 0;
            virtual void UpdateVariable(Uint32 index, SPtr<Sampler>& sampler) = 0;
            virtual void UpdateVariable(Uint32 index, SPtr<TextureView>& textureView, SPtr<Sampler>& sampler) = 0;
            // virtual void UpdateVariable(StringView name, void* pData, Uint32 size) = 0; // TODO: 이름 기반으로 찾는 것 차후 구현
        };

        struct ShaderVariablesLayoutCreateInfo
        {
            FrameVector<ShaderVariableInfo> variableInfos;

            const char* debugName = "";
        };

        class ShaderVariablesLayout
        {
        public:
            ShaderVariablesLayout() = default;
            virtual ~ShaderVariablesLayout() = default;

            virtual SPtr<ShaderVariables> CreateVariables() = 0;
        };
    } // namespace rapi
} // namespace cube
