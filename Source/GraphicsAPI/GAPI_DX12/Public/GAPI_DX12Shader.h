#pragma once

#include "DX12Header.h"

#include "GAPI_Shader.h"

#include "DX12APIObject.h"

namespace cube
{
    namespace gapi
    {
        class DX12Shader : public Shader, public DX12APIObject
        {
        public:
            DX12Shader(Blob&& shader);
            virtual ~DX12Shader();

            BlobView GetShader() const { return mShader; }

        private:
            Blob mShader;
        };
    } // namespace gapi
} // namespace cube
