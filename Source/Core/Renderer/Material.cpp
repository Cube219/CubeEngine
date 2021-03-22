#include "Material.h"

#include "RendererManager.h"
#include "Texture.h"


#include "RenderAPIs/RenderAPI/Interface/ShaderVariables.h"

namespace cube
{
    UPtr<MaterialInstance> MaterialInstance::Create(HMaterial material)
    {
        UPtr<MaterialInstance> pMatIns = std::make_unique<MaterialInstance>(material);
        pMatIns->OnCreate();

        return pMatIns;
    }

    MaterialInstance::MaterialInstance(HMaterial material) :
        mMaterial(material)
    {
        auto& variableInfos = material->GetVariableInfos();

        mVariableBufferSize = 0;
        mVariableData.resize(variableInfos.size());
        for(Uint32 i = 0; i < SCast(Uint32)(variableInfos.size()); ++i) {
            auto& vInfo = variableInfos[i];
            auto& vData = mVariableData[i];

            vData.type = vInfo.type;
            vData.size = vInfo.size;

            if(vInfo.type == MaterialVariableType::Data) {
                mVariableBufferSize += vInfo.size;
            }
        }

        if(mVariableBufferSize > 0) {
            mpVariableBuffer = malloc(mVariableBufferSize);
            void* currentOffset = mpVariableBuffer;
            for(Uint32 i = 0; i < SCast(Uint32)(mVariableData.size()); ++i) {
                auto& vData = mVariableData[i];
                if(vData.type == MaterialVariableType::Data) {
                    vData.pData = currentOffset;
                    currentOffset = (char*)currentOffset + vData.size;
                }
            }
        } else {
            mpVariableBuffer = nullptr;
        }
    }

    MaterialInstance::~MaterialInstance()
    {
        if(mpVariableBuffer != nullptr) {
            free(mpVariableBuffer);
        }
    }

    SPtr<rt::RenderObject> MaterialInstance::CreateRTObject()
    {
        SPtr<rt::MaterialInstance> rtMaterialIns = std::make_shared<rt::MaterialInstance>(mVariableData);

        RenderingThread::QueueSyncTask([this]() {
            SPtr<rt::Material> rtMat = DPCast(rt::Material)(mMaterial->GetRTObject());
            DPCast(rt::MaterialInstance)(GetRTObject())->SyncRTMaterial(rtMat);
        });

        return rtMaterialIns;
    }

    void MaterialInstance::SetVariableData(Uint32 index, void* pData, Uint64 size)
    {
        auto& vData = mVariableData[index];
        CHECK(vData.type == MaterialVariableType::Data, "Only Data type can be set to byte data.");
        CHECK(vData.size == size, "Wrong variable size. (Variable size: {} / Data size: {})", vData.size, size);

        memcpy(vData.pData, pData, size);

        QueueSyncTask([this, index]() {
            DPCast(rt::MaterialInstance)(GetRTObject())->SyncVariableData(index, mVariableData[index]);
        });
    }

    void MaterialInstance::SetVariableData(Uint32 index, RPtr<Texture>& texture)
    {
        auto& vData = mVariableData[index];
        CHECK(vData.type == MaterialVariableType::Texture, "Only Texture type can be set to texture data.");

        vData.texture = texture;

        QueueSyncTask([this, index]() {
            DPCast(rt::MaterialInstance)(GetRTObject())->SyncVariableData(index, mVariableData[index]);
        });
    }

    namespace rt
    {
        MaterialInstance::MaterialInstance(const Vector<MaterialVariableData>& variableData) :
            mTempVariableData(variableData)
        {
            Uint64 bufferSize = 0;
            for(Uint32 i = 0; i < SCast(Uint32)(mTempVariableData.size()); ++i) {
                auto& vData = mTempVariableData[i];

                if(vData.type == MaterialVariableType::Data) {
                    bufferSize += vData.size;
                }
            }

            if(bufferSize > 0) {
                mpTempVariableBuffer = malloc(bufferSize);
                void* currentOffset = mpTempVariableBuffer;
                for(Uint32 i = 0; i < SCast(Uint32)(mTempVariableData.size()); ++i) {
                    auto& vData = mTempVariableData[i];
                    if(vData.type == MaterialVariableType::Data) {
                        vData.pData = currentOffset;
                        currentOffset = (char*)currentOffset + vData.size;
                    }
                }
            } else {
                mpTempVariableBuffer = nullptr;
            }

            mDirtyVariables.resize(mTempVariableData.size(), false);
        }

        MaterialInstance::~MaterialInstance()
        {
            if(mpTempVariableBuffer != nullptr) {
                free(mpTempVariableBuffer);
            }
        }

        void MaterialInstance::OnCreate()
        {
            mShaderVariables = mRTMaterial->GetShaderVariablesLayout()->CreateVariables();
        }

        void MaterialInstance::OnDestroy()
        {
            mShaderVariables = nullptr;
            mRTMaterial = nullptr;
        }

        void MaterialInstance::SyncRTMaterial(SPtr<rt::Material>& rtMaterial)
        {
            mRTMaterial = rtMaterial; // Only use for creating ShaderVariable
        }

        void MaterialInstance::SyncVariableData(Uint32 index, const MaterialVariableData& data)
        {
            auto& vData = mTempVariableData[index];
            CHECK(vData.type == data.type, "Incorrect variable type while syncing variable data. ({} != {})", vData.type, data.type);

            switch(vData.type) {
                case MaterialVariableType::Data:
                    memcpy(vData.pData, data.pData, vData.size);
                    break;
                case MaterialVariableType::Texture:
                    vData.texture = data.texture;
                    break;
            }
            mDirtyVariables[index] = true;

            if(mIsDirty == false) {
                mIsDirty = true;
                RenderingThread::QueueTask([this]() {
                    FlushToGPU();
                });
            }
        }

        void MaterialInstance::FlushToGPU()
        {
            mIsDirty = false;

            for(Uint32 i = 0; i < SCast(Uint32)(mTempVariableData.size()); ++i) {
                if(mDirtyVariables[i] == false) continue;

                auto& vData = mTempVariableData[i];

                switch(vData.type) {
                    case MaterialVariableType::Data:
                        mShaderVariables->UpdateVariable(i, vData.pData, SCast(Uint32)(vData.size));
                        break;
                    case MaterialVariableType::Texture:
                        auto rtTexture = DPCast(Texture)(vData.texture->GetRTObject());
                        SPtr<rapi::TextureView> textureView = rtTexture->GetGPUTextureView();
                        SPtr<rapi::Sampler> sampler = rtTexture->GetSampler();
                        mShaderVariables->UpdateVariable(i, textureView, sampler);
                        break;
                }

                mDirtyVariables[i] = false;
            }
        }
    } // namespace rt

    HMaterial Material::Create(const MaterialCreateInfo& createInfo)
    {
        UPtr<Material> pMat = std::make_unique<Material>(createInfo);
        pMat->OnCreate();

        return RendererManager::RegisterMaterial(std::move(pMat));
    }

    Material::Material(const MaterialCreateInfo& createInfo)
    {
        mShaders.assign(createInfo.pShaders, createInfo.pShaders + createInfo.numShaders);
        mVariableInfos.assign(createInfo.pParameterInfos,
            createInfo.pParameterInfos + createInfo.numParameterInfos);
    }

    Material::~Material()
    {
        
    }

    SPtr<rt::RenderObject> Material::CreateRTObject()
    {
        SPtr<rt::Material> rtMaterial = std::make_shared<rt::Material>();

        RenderingThread::QueueSyncTask([this]() {
            DPCast(rt::Material)(GetRTObject())->SyncMaterial(mShaders, mVariableInfos);
        });

        return rtMaterial;
    }

    HMaterialInstance Material::CreateInstance()
    {
        return RendererManager::RegisterRenderObject(MaterialInstance::Create(GetHandler()));
    }

    namespace rt
    {
        Material::Material()
        {
            
        }

        Material::~Material()
        {
            
        }

        void Material::OnCreate()
        {
            using namespace rapi;
            ShaderVariablesLayoutCreateInfo layoutCreateInfo;
            layoutCreateInfo.variableInfos.resize(mVariableInfos.size());
            for(Uint32 i = 0; i < SCast(Uint32)(mVariableInfos.size()); ++i) {
                MaterialVariableInfo& parameterInfo = mVariableInfos[i];
                ShaderVariableInfo& variableInfo = layoutCreateInfo.variableInfos[i];

                switch(parameterInfo.type) {
                    case MaterialVariableType::Data:
                        variableInfo.type = ShaderVariableType::Constant;
                        break;
                    case MaterialVariableType::Texture:
                        variableInfo.type = ShaderVariableType::SamplerTexture;
                        break;
                    default:
                        ASSERTION_FAILED("Unknown MaterialVariableType ({}).", parameterInfo.type);
                }
                variableInfo.size = SCast(Uint32)(parameterInfo.size);
                variableInfo.count = 1;
            }
            layoutCreateInfo.debugName = "Material shader variable layout";
            mShaderVariablesLayout = RendererManager::GetRenderAPI().CreateShaderVariablesLayout(layoutCreateInfo);
        }

        void Material::OnDestroy()
        {
            mShaderVariablesLayout = nullptr;
            mVariableInfos.clear();
            mShaders.clear();
        }

        void Material::SyncMaterial(Vector<RPtr<Shader>>& shaders, Vector<MaterialVariableInfo>& variableInfos)
        {
            mShaders = shaders;
            mVariableInfos = variableInfos;

            shaders.clear();
            variableInfos.clear();
        }
    } // namespace rapi
} // namespace cube
