#pragma once

#include "../CoreHeader.h"

#include "RenderObject.h"
#include "Shader.h"

namespace cube
{
    enum class MaterialVariableType
    {
        Data,
        Texture
    };

    struct MaterialVariableData
    {
        MaterialVariableType type;
        Uint64 size;
        void* pData = nullptr;
        RPtr<Texture> texture = nullptr;
    };

    class CORE_EXPORT MaterialInstance : public RenderObject
    {
    public:
        static UPtr<MaterialInstance> Create(HMaterial material);

    public:
        MaterialInstance(HMaterial material);
        virtual ~MaterialInstance();

        virtual SPtr<rt::RenderObject> CreateRTObject() override;

        void SetVariableData(Uint32 index, void* pData, Uint64 size);
        void SetVariableData(Uint32 index, RPtr<Texture>& texture);

    private:
        Vector<MaterialVariableData> mVariableData;
        void* mpVariableBuffer;
        Uint64 mVariableBufferSize;

        HMaterial mMaterial;
    };

    namespace rt
    {
        class MaterialInstance : public RenderObject
        {
        public:
            MaterialInstance(const Vector<MaterialVariableData>& variableData);
            virtual ~MaterialInstance();

            virtual void OnCreate() override;
            virtual void OnDestroy() override;

            void SyncRTMaterial(SPtr<rt::Material>& rtMaterial);
            void SyncVariableData(Uint32 index, const MaterialVariableData& data);

        private:
            void FlushToGPU();

            bool mIsDirty;
            Vector<MaterialVariableData> mTempVariableData;
            Vector<bool> mDirtyVariables;
            void* mpTempVariableBuffer;
            SPtr<rapi::ShaderVariables> mShaderVariables;

            SPtr<Material> mRTMaterial;
        };
    } // namespace rt

    struct MaterialVariableInfo
    {
        MaterialVariableType type;
        Uint64 size;
    };

    struct MaterialCreateInfo
    {
        Uint32 numShaders;
        RPtr<Shader>* pShaders;
        Uint32 numParameterInfos;
        MaterialVariableInfo* pParameterInfos;
    };

    class CORE_EXPORT Material : public RenderObject
    {
    public:
        static HMaterial Create(const MaterialCreateInfo& createInfo);

    public:
        Material(const MaterialCreateInfo& createInfo);
        virtual ~Material();

        virtual SPtr<rt::RenderObject> CreateRTObject() override;

        const Vector<MaterialVariableInfo>& GetVariableInfos() const { return mVariableInfos; }
        HMaterialInstance CreateInstance();

    private:
        Vector<RPtr<Shader>> mShaders;
        Vector<MaterialVariableInfo> mVariableInfos;
    };

    namespace rt
    {
        class Material : public RenderObject
        {
        public:
            Material();
            virtual ~Material();

            void OnCreate() override;
            void OnDestroy() override;

            SPtr<rapi::ShaderVariablesLayout> GetShaderVariablesLayout() const { return mShaderVariablesLayout; }

            void SyncMaterial(Vector<RPtr<Shader>>& shaders, Vector<MaterialVariableInfo>& variableInfos);

        private:
            friend class RendererManager;
            friend class RendererManagerRT;

            Vector<RPtr<Shader>> mShaders;
            Vector<MaterialVariableInfo> mVariableInfos;

            SPtr<rapi::ShaderVariablesLayout> mShaderVariablesLayout;

            Int32 mMaterialIndex = -1; // Assigned by RendererManagerRT
        };
    } // namespace rt
} // namespace cube
