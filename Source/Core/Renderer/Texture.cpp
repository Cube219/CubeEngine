#include "Texture.h"

#include "RendererManager.h"

#include "Core/Resource/ResourceManager.h"

namespace cube
{
    TextureData::TextureData() :
        mWidth(0),
        mHeight(0),
        mDepth(0),
        mMipLevels(0),
        mpData(nullptr),
        mSize(0)
    {}

    TextureData::TextureData(Uint32 width, Uint32 height, Uint32 depth, Uint32 mipLevel, Uint64 size, void* pData):
        mWidth(width),
        mHeight(height),
        mDepth(depth),
        mMipLevels(mipLevel),
        mSize(size)
    {
        mpData = malloc(mSize);
        memcpy(mpData, pData, mSize);
    }

    TextureData::~TextureData()
    {
        if(mpData != nullptr) free(mpData);
    }

    RPtr<Texture> Texture::Load(StringView path)
    {
        RPtr<Texture> texture = ResourceManager::LoadResource<Texture>(path);
        texture->OnCreate();

        return texture;
    }

    Texture::Texture(const SPtr<TextureData>& textureData) :
        mTextureData(textureData)
    {}

    Texture::~Texture()
    {}

    SPtr<rt::RenderObject> Texture::CreateRTObject()
    {
        // Move texture data into rt::Texture
        SPtr<rt::Texture> rtTexture = std::make_shared<rt::Texture>(mTextureData);
        mTextureData = nullptr;

        return rtTexture;
    }

    namespace rt
    {
        Texture::Texture(const SPtr<TextureData>& textureData) :
            mTextureData(textureData)
        {
            mSampler = RendererManager::GetDefaultSampler();
        }

        Texture::~Texture()
        {}

        void Texture::OnCreate()
        {
            if(mTextureData != nullptr) {
                FlushToGPU();
            }
        }

        void Texture::OnDestroy()
        {
            mGPUTextureView = nullptr;
            mGPUTexture = nullptr;
            mTextureData = nullptr;
        }

        void Texture::FlushToGPU()
        {
            auto& rAPI =  RendererManager::GetRenderAPI();

            rapi::TextureCreateInfo textureCreateInfo;
            textureCreateInfo.usage = rapi::ResourceUsage::Default;
            textureCreateInfo.dimension = rapi::TextureDimension::Texture2D;
            textureCreateInfo.width = mTextureData->GetWidth();
            textureCreateInfo.height = mTextureData->GetHeight();
            textureCreateInfo.depth = mTextureData->GetDepth();
            textureCreateInfo.arraySize = 1;
            textureCreateInfo.format = rapi::TextureFormat::RGBA_8_UNorm;
            textureCreateInfo.bindTypeFlags = rapi::TextureBindTypeFlag::ShaderResource;
            textureCreateInfo.generateMipmaps = false;
            textureCreateInfo.samplesNum = 1;
            textureCreateInfo.debugName = "Texture";
            rapi::TextureData td;
            td.pData = mTextureData->GetData();
            td.size = SCast(Uint32)(mTextureData->GetDataSize());
            textureCreateInfo.data.push_back(td);

            mGPUTexture = rAPI.CreateTexture(textureCreateInfo);
            mGPUTextureView = mGPUTexture->CreateDefaultView();

            mTextureData = nullptr;
        }
    } // namespace rt
} // namespace cube
