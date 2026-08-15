#include "RenderGraph.h"

#include "Allocator/FrameAllocator.h"
#include "Buffer.h"
#include "Checker.h"
#include "GAPI_CommandList.h"
#include "Pipeline.h"
#include "RenderCore/RenderGraphTypes.h"
#include "Texture.h"


namespace cube
{
    // ===== Resources =====

    RGResource::RGResource(Int32 index, StringView debugName)
        : mIsTransient(false)
        , mIndex(index)
        , mBeginPass(-1)
        , mEndPass(-1)
        , mDebugName(debugName)
    {
    }

    void RGBuffer::CreateResource(GAPI& gapi)
    {
        if (mIsTransient && !mBuffer)
        {
            mBuffer = gapi.CreateBuffer({
                .usage = gapi::ResourceUsage::Transient,
                .bufferInfo = mBufferInfo,
                .debugName = mDebugName
            });
        }
    }

    RGBuffer::RGBuffer(int index, const gapi::BufferInfo& bufferInfo, StringView debugName)
        : RGResource(index, debugName)
        , mBuffer(nullptr)
        , mBufferInfo(bufferInfo)
    {
        mIsTransient = true;
    }

    RGBuffer::RGBuffer(int index, SharedPtr<gapi::Buffer> buffer)
        : RGResource(index, buffer->GetDebugName())
        , mBuffer(buffer)
    {
        mIsTransient = buffer->GetUsage() == gapi::ResourceUsage::Transient;
        CHECK_FORMAT(!mIsTransient, "Cannot register transient buffer!");

        mBufferInfo = buffer->GetInfo();
    }

    Uint64 RGBuffer::CalculateViewHashKey(RGViewType type, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements) const
    {
        // Use index because resource may not be created right now.
        return HashCombine((Uint64)type, GetIndex(), (Uint64)format, firstElement, numElements);
    }

    RGBufferView::RGBufferView(int index, RGBufferHandle rgBuffer, RGViewType type, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
        : RGResource(index, rgBuffer->GetDebugName())
        , mRGBuffer(rgBuffer)
        , mType(type)
        , mFormat(format)
        , mFirstElement(firstElement)
        , mNumElements(numElements)
    {
        mViewHashKey = mRGBuffer->CalculateViewHashKey(type, format, firstElement, numElements);
        mIsTransient = mRGBuffer->IsTransient();
    }

    void RGBufferSRV::CreateResource(GAPI& gapi)
    {
        if (!mSRV)
        {
            mRGBuffer->CreateResource(gapi);

            mSRV = mRGBuffer->mBuffer->CreateSRV({
                .typedFormat = mFormat,
                .firstElement = mFirstElement,
                .numElements = mNumElements
            });
        }
    }

    RGBufferSRV::RGBufferSRV(int index, RGBufferHandle rgBuffer, const gapi::BufferSRVCreateInfo& createInfo)
        : RGBufferView(index, rgBuffer, RGViewType::SRV, createInfo.typedFormat, createInfo.firstElement, createInfo.numElements)
    {
    }

    void RGBufferUAV::CreateResource(GAPI& gapi)
    {
        if (!mUAV)
        {
            mRGBuffer->CreateResource(gapi);

            mUAV = mRGBuffer->mBuffer->CreateUAV({
                .typedFormat = mFormat,
                .firstElement = mFirstElement,
                .numElements = mNumElements
            });
        }
    }

    RGBufferUAV::RGBufferUAV(int index, RGBufferHandle rgBuffer, const gapi::BufferUAVCreateInfo& createInfo)
        : RGBufferView(index, rgBuffer, RGViewType::UAV, createInfo.typedFormat, createInfo.firstElement, createInfo.numElements)
    {
    }

    void RGTexture::CreateResource(GAPI& gapi)
    {
        if (mIsTransient && !mTexture)
        {
            mTexture = gapi.CreateTexture({
                .usage = gapi::ResourceUsage::Transient,
                .textureInfo = mTextureInfo,
                .initialLayout = gapi::ResourceLayout::Undefined,
                .debugName = mDebugName
            });
        }
    }

    Uint64 RGTexture::CalculateViewHashKey(RGViewType type, const gapi::SubresourceRange& subresourceRange) const
    {
        // Use index because resource may not be created right now.
        return HashCombine((Uint64)type, GetIndex(), subresourceRange.GetHash());
    }

    Uint64 RGTexture::CalculateViewHashKey(RGViewType type, const gapi::SubresourceRangeInput& subresourceRangeInput) const
    {
        return CalculateViewHashKey(type, subresourceRangeInput.Clamp(mTextureInfo));
    }

    RGTexture::RGTexture(int index, const gapi::TextureInfo& textureInfo, StringView debugName)
        : RGResource(index, debugName)
        , mTexture(nullptr)
        , mTextureInfo(textureInfo)
    {
        mIsTransient = true;
    }

    RGTexture::RGTexture(int index, SharedPtr<gapi::Texture> texture)
        : RGResource(index, texture->GetDebugName())
        , mTexture(texture)
        , mTextureInfo(texture->GetInfo())
    {
        mIsTransient = texture->GetUsage() == gapi::ResourceUsage::Transient;
        CHECK_FORMAT(!mIsTransient, "Cannot register transient texture!");
    }

    RGTextureView::RGTextureView(int index, RGTextureHandle rgTexture, RGViewType type, const gapi::SubresourceRangeInput& subresourceRange)
        : RGResource(index, rgTexture->GetDebugName())
        , mRGTexture(rgTexture)
        , mType(type)
        , mSubresourceRange(subresourceRange.Clamp(rgTexture->GetTextureInfo()))
    {
        mViewHashKey = mRGTexture->CalculateViewHashKey(mType, mSubresourceRange);
        mIsTransient = rgTexture->IsTransient();
    }

    void RGTextureSRV::CreateResource(GAPI& gapi)
    {
        if (!mSRV)
        {
            mRGTexture->CreateResource(gapi);

            mSRV = mRGTexture->mTexture->CreateSRV(mCreateInfo);
            CHECK(mSubresourceRange == mSRV->GetSubresourceRange());
        }
    }

    RGTextureSRV::RGTextureSRV(int index, RGTextureHandle rgTexture, const gapi::TextureSRVCreateInfo& createInfo)
        : RGTextureView(index, rgTexture, RGViewType::SRV, createInfo.subresourceRange)
        , mCreateInfo(createInfo)
    {
    }

    void RGTextureUAV::CreateResource(GAPI& gapi)
    {
        if (!mUAV)
        {
            mRGTexture->CreateResource(gapi);

            mUAV = mRGTexture->mTexture->CreateUAV(mCreateInfo);
            CHECK(mSubresourceRange == mUAV->GetSubresourceRange());
        }
    }

    RGTextureUAV::RGTextureUAV(int index, RGTextureHandle rgTexture, const gapi::TextureUAVCreateInfo& createInfo)
        : RGTextureView(index, rgTexture, RGViewType::UAV, [range = createInfo.subresourceRange]{
            // A UAV always targets a single mip level.
            gapi::SubresourceRangeInput uavRange = range;
            uavRange.mipLevels = 1;
            return uavRange;
        }())
        , mCreateInfo(createInfo)
    {
    }

    void RGTextureRTV::CreateResource(GAPI& gapi)
    {
        if (!mRTV)
        {
            mRGTexture->CreateResource(gapi);

            mRTV = mRGTexture->mTexture->CreateRTV(mCreateInfo);
            CHECK(mSubresourceRange == mRTV->GetSubresourceRange());
        }
    }

    RGTextureRTV::RGTextureRTV(int index, RGTextureHandle rgTexture, const gapi::TextureRTVCreateInfo& createInfo)
        : RGTextureView(index, rgTexture, RGViewType::RTV, [range = createInfo.subresourceRange]{
            // An RTV always targets a single mip level.
            gapi::SubresourceRangeInput rtvRange = range;
            rtvRange.mipLevels = 1;
            return rtvRange;
        }())
        , mCreateInfo(createInfo)
    {
    }

    void RGTextureDSV::CreateResource(GAPI& gapi)
    {
        if (!mDSV)
        {
            mRGTexture->CreateResource(gapi);

            mDSV = mRGTexture->mTexture->CreateDSV(mCreateInfo);
            CHECK(mSubresourceRange == mDSV->GetSubresourceRange());
        }
    }

    RGTextureDSV::RGTextureDSV(int index, RGTextureHandle rgTexture, const gapi::TextureDSVCreateInfo& createInfo)
        : RGTextureView(index, rgTexture, RGViewType::DSV, [range = createInfo.subresourceRange]{
            // A DSV always targets a single mip level.
            gapi::SubresourceRangeInput dsvRange = range;
            dsvRange.mipLevels = 1;
            return dsvRange;
        }())
        , mCreateInfo(createInfo)
    {
    }

    RGShaderParameterListBase::RGShaderParameterListBase(int index, const ShaderParameterListInfo& parameterListInfo, SharedPtr<ShaderParameterList> parameterList)
        : RGResource(index, parameterListInfo.name)
        , mParameterListInfo(parameterListInfo)
        , mParameterList(std::move(parameterList))
    {
    }

    // ===== Builder =====

    RGBuilder::RGBuilder(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    RGBuilder::~RGBuilder()
    {
        Reset();
    }

    RGBufferHandle RGBuilder::RegisterBuffer(SharedPtr<gapi::Buffer> buffer)
    {
        if (auto findIt = mRegisteredBufferInfos.find(buffer.get()); findIt != mRegisteredBufferInfos.end())
        {
            return findIt->second.buffer;
        }

        RGBufferHandle rgBuffer(new RGBuffer(mResources.size(), buffer));
        mResources.push_back(rgBuffer);

        mRegisteredBufferInfos.insert({
            buffer.get(),
            RegisteredBufferInfo{
                .buffer = rgBuffer,
            }
        });

        return rgBuffer;
    }

    RGBufferHandle RGBuilder::CreateBuffer(const gapi::BufferInfo& bufferInfo, StringView debugName)
    {
        RGBufferHandle rgBuffer(new RGBuffer(mResources.size(), bufferInfo, debugName));
        mResources.push_back(rgBuffer);

        return rgBuffer;
    }

    RGBufferSRVHandle RGBuilder::CreateSRV(RGBufferHandle rgBuffer, const gapi::BufferSRVCreateInfo& createInfo)
    {
        const Uint64 cacheKey = rgBuffer->CalculateViewHashKey(RGViewType::SRV, createInfo.typedFormat, createInfo.firstElement, createInfo.numElements);
        if (auto findIt = mCachedBufferViews.find(cacheKey); findIt != mCachedBufferViews.end())
        {
            return findIt->second.Cast<RGBufferSRV>();
        }

        RGBufferSRVHandle rgSRV(new RGBufferSRV(mResources.size(), rgBuffer, createInfo));
        mResources.push_back(rgSRV);
        CHECK(cacheKey == rgSRV->GetViewHashKey());
        mCachedBufferViews.insert({ cacheKey, rgSRV });

        return rgSRV;
    }

    RGBufferUAVHandle RGBuilder::CreateUAV(RGBufferHandle rgBuffer, const gapi::BufferUAVCreateInfo& createInfo)
    {
        const Uint64 cacheKey = rgBuffer->CalculateViewHashKey(RGViewType::UAV, createInfo.typedFormat, createInfo.firstElement, createInfo.numElements);
        if (auto findIt = mCachedBufferViews.find(cacheKey); findIt != mCachedBufferViews.end())
        {
            return findIt->second.Cast<RGBufferUAV>();
        }

        RGBufferUAVHandle rgUAV(new RGBufferUAV(mResources.size(), rgBuffer, createInfo));
        mResources.push_back(rgUAV);
        CHECK(cacheKey == rgUAV->GetViewHashKey());
        mCachedBufferViews.insert({ cacheKey, rgUAV });

        return rgUAV;
    }

    RGTextureHandle RGBuilder::RegisterTexture(SharedPtr<gapi::Texture> texture,
        gapi::ResourceLayout srcLayout,
        gapi::ResourceLayout dstLayout)
    {
        CHECK(texture);

        if (auto findIt = mRegisteredTextureInfos.find(texture.get()); findIt != mRegisteredTextureInfos.end())
        {
            CHECK_FORMAT(srcLayout == findIt->second.srcLayout && dstLayout == findIt->second.dstLayout, "Try to register same texture with different layout settings!");
            return findIt->second.texture;
        }

        RGTextureHandle rgTexture(new RGTexture(mResources.size(), texture));
        mResources.push_back(rgTexture);

        mRegisteredTextureInfos.insert({
            texture.get(),
            RegisteredTextureInfo{
                .texture = rgTexture,
                .srcLayout = srcLayout,
                .dstLayout = dstLayout,
            }
        });

        return rgTexture;
    }

    RGTextureHandle RGBuilder::CreateTexture(const gapi::TextureInfo& textureInfo, StringView debugName)
    {
        RGTextureHandle rgTexture(new RGTexture(mResources.size(), textureInfo, debugName));
        mResources.push_back(rgTexture);

        return rgTexture;
    }

    RGTextureSRVHandle RGBuilder::CreateSRV(RGTextureHandle rgTexture, const gapi::TextureSRVCreateInfo& createInfo)
    {
        const Uint64 cacheKey = rgTexture->CalculateViewHashKey(RGViewType::SRV, createInfo.subresourceRange);
        if (auto findIt = mCachedTextureViews.find(cacheKey); findIt != mCachedTextureViews.end())
        {
            return findIt->second.Cast<RGTextureSRV>();
        }

        RGTextureSRVHandle rgSRV(new RGTextureSRV(mResources.size(), rgTexture, createInfo));
        mResources.push_back(rgSRV);
        CHECK(cacheKey == rgSRV->GetViewHashKey());
        mCachedTextureViews.insert({ cacheKey, rgSRV });

        return rgSRV;
    }

    RGTextureUAVHandle RGBuilder::CreateUAV(RGTextureHandle rgTexture, const gapi::TextureUAVCreateInfo& createInfo)
    {
        // A UAV always targets a single mip level.
        gapi::SubresourceRangeInput uavRange = createInfo.subresourceRange;
        uavRange.mipLevels = 1;
        const Uint64 cacheKey = rgTexture->CalculateViewHashKey(RGViewType::UAV, uavRange);
        if (auto findIt = mCachedTextureViews.find(cacheKey); findIt != mCachedTextureViews.end())
        {
            return findIt->second.Cast<RGTextureUAV>();
        }

        RGTextureUAVHandle rgUAV(new RGTextureUAV(mResources.size(), rgTexture, createInfo));
        mResources.push_back(rgUAV);
        CHECK(cacheKey == rgUAV->GetViewHashKey());
        mCachedTextureViews.insert({ cacheKey, rgUAV });

        return rgUAV;
    }

    RGTextureRTVHandle RGBuilder::CreateRTV(RGTextureHandle rgTexture, const gapi::TextureRTVCreateInfo& createInfo)
    {
        // An RTV always targets a single mip level.
        gapi::SubresourceRangeInput rtvRange = createInfo.subresourceRange;
        rtvRange.mipLevels = 1;
        const Uint64 cacheKey = rgTexture->CalculateViewHashKey(RGViewType::RTV, rtvRange);
        if (auto findIt = mCachedTextureViews.find(cacheKey); findIt != mCachedTextureViews.end())
        {
            return findIt->second.Cast<RGTextureRTV>();
        }

        RGTextureRTVHandle rgRTV(new RGTextureRTV(mResources.size(), rgTexture, createInfo));
        mResources.push_back(rgRTV);
        CHECK(cacheKey == rgRTV->GetViewHashKey());
        mCachedTextureViews.insert({ cacheKey, rgRTV });

        return rgRTV;
    }

    RGTextureDSVHandle RGBuilder::CreateDSV(RGTextureHandle rgTexture, const gapi::TextureDSVCreateInfo& createInfo)
    {
        // A DSV always targets a single mip level.
        gapi::SubresourceRangeInput dsvRange = createInfo.subresourceRange;
        dsvRange.mipLevels = 1;
        const Uint64 cacheKey = rgTexture->CalculateViewHashKey(RGViewType::DSV, dsvRange);
        if (auto findIt = mCachedTextureViews.find(cacheKey); findIt != mCachedTextureViews.end())
        {
            return findIt->second.Cast<RGTextureDSV>();
        }

        RGTextureDSVHandle rgDSV(new RGTextureDSV(mResources.size(), rgTexture, createInfo));
        mResources.push_back(rgDSV);
        CHECK(cacheKey == rgDSV->GetViewHashKey());
        mCachedTextureViews.insert({ cacheKey, rgDSV });

        return rgDSV;
    }

    RGTextureSRVHandle RGBuilder::GetDummyBlackTexture2D()
    {
        if (!mDummyBlackTexture2D.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyBlackTexture2D()->GetGAPITexture());
            mDummyBlackTexture2D = CreateSRV(rgTexture);
        }

        return mDummyBlackTexture2D;
    }

    RGTextureSRVHandle RGBuilder::GetDummyBlackTextureCube()
    {
        if (!mDummyBlackTextureCube.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyBlackTextureCube()->GetGAPITexture());
            mDummyBlackTextureCube = CreateSRV(rgTexture);
        }

        return mDummyBlackTextureCube;
    }

    RGTextureSRVHandle RGBuilder::GetDummyWhiteTexture2D()
    {
        if (!mDummyWhiteTexture2D.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyWhiteTexture2D()->GetGAPITexture());
            mDummyWhiteTexture2D = CreateSRV(rgTexture);
        }

        return mDummyWhiteTexture2D;
    }

    void RGBuilder::BeginRenderPass(const RenderPassInfo& info)
    {
        CHECK(mPhase == Phase::Init);
        CHECK(!mInitState.isInRenderPass);

        CHECK(info.colors.size() <= gapi::MAX_NUM_RENDER_TARGETS);
        mInitState.renderPassRenderTargetFormats.resize(info.colors.size());
        for (Uint32 i = 0; i < static_cast<Uint32>(mInitState.renderPassRenderTargetFormats.size()); ++i)
        {
            mInitState.renderPassRenderTargetFormats[i] = info.colors[i].color->GetParent()->GetTextureInfo().format;
        }
        if (info.depthStencil.dsv.IsValid())
        {
            mInitState.renderPassDepthStencilFormat = info.depthStencil.dsv->GetParent()->GetTextureInfo().format;
        }
        else
        {
            mInitState.renderPassDepthStencilFormat = gapi::ElementFormat::Unknown;
        }

        AddPass(CUBE_T("##BeginRenderPass"), [info](gapi::CommandList& commandList)
        {
            FrameVector<gapi::ColorAttachment> colors(info.colors.size());
            for (int i = 0; i < colors.size(); ++i)
            {
                colors[i] = {
                    .rtv = info.colors[i].color->GetRTV(),
                    .loadOperation = info.colors[i].loadOperation,
                    .storeOperation = info.colors[i].storeOperation,
                    .clearColor = info.colors[i].clearColor
                };
            }

            gapi::DepthStencilAttachment depthStencil = {
                .dsv = info.depthStencil.dsv.IsValid() ? info.depthStencil.dsv->GetDSV() : nullptr,
                .loadOperation = info.depthStencil.loadOperation,
                .storeOperation = info.depthStencil.storeOperation,
                .clearDepth = info.depthStencil.clearDepth
            };

            commandList.BeginRenderPass(colors, depthStencil);
        },
        [info](RGBuilder& builder)
        {
            for (const RenderPassInfo::ColorAttachment& color : info.colors)
            {
                builder.UseResource(color.color);
            }
            if (info.depthStencil.dsv.IsValid())
            {
                builder.UseResource(info.depthStencil.dsv);
            }

            builder.mTrackingResourcesState.attachedRTVsInRenderPass.reserve(info.colors.size());
            for (const RenderPassInfo::ColorAttachment& colorAttachment : info.colors)
            {
                builder.mTrackingResourcesState.attachedRTVsInRenderPass.push_back(colorAttachment.color);
            }
            builder.mTrackingResourcesState.attachedDSVInRenderPass = info.depthStencil.dsv;
            builder.mTrackingResourcesState.renderPassIndex = builder.mTrackingResourcesState.passIndex;
        });

        mInitState.isInRenderPass = true;
    }

    void RGBuilder::EndRenderPass()
    {
        CHECK(mPhase == Phase::Init);
        CHECK(mInitState.isInRenderPass);

        mInitState.renderPassRenderTargetFormats.clear();
        mInitState.renderPassDepthStencilFormat = gapi::ElementFormat::Unknown;

        AddPass(CUBE_T("##EndRenderPass"), [](gapi::CommandList& commandList)
        {
            commandList.EndRenderPass();
        },
        [](RGBuilder& builder)
        {
            // Just mark the resource as used to prevent duplicate transitions.
            for (const RGTextureRTVHandle attachedRTV : builder.mTrackingResourcesState.attachedRTVsInRenderPass)
            {
                attachedRTV->UpdateUsePassIndex(builder.mTrackingResourcesState.passIndex);
            }
            if (builder.mTrackingResourcesState.attachedDSVInRenderPass.IsValid())
            {
                builder.mTrackingResourcesState.attachedDSVInRenderPass->UpdateUsePassIndex(builder.mTrackingResourcesState.passIndex);
            }

            builder.mTrackingResourcesState.attachedDSVInRenderPass = {};
            builder.mTrackingResourcesState.attachedRTVsInRenderPass.clear();
            builder.mTrackingResourcesState.renderPassIndex = -1;
        });

        mInitState.isInRenderPass = false;
    }

    void RGBuilder::ApplyRenderTargetFormatsFromCurrentRenderPass(GraphicsPipelineInfo& inOutGraphicsPipelineInfo) const
    {
        CHECK(mPhase == Phase::Init);
        CHECK(mInitState.isInRenderPass);

        inOutGraphicsPipelineInfo.numRenderTargets = static_cast<Uint32>(mInitState.renderPassRenderTargetFormats.size());
        for (Uint32 i = 0; i < static_cast<Uint32>(mInitState.renderPassRenderTargetFormats.size()); ++i)
        {
            inOutGraphicsPipelineInfo.renderTargetFormats[i] = mInitState.renderPassRenderTargetFormats[i];
        }
        inOutGraphicsPipelineInfo.depthStencilFormat = mInitState.renderPassDepthStencilFormat;
    }

    void RGBuilder::ApplyRenderTargetFormatsFromCurrentRenderPass(MaterialPipelineStateInfo& inOutMaterialPipelineInfo) const
    {
        CHECK(mPhase == Phase::Init);
        CHECK(mInitState.isInRenderPass);

        inOutMaterialPipelineInfo.numRenderTargets = static_cast<Uint32>(mInitState.renderPassRenderTargetFormats.size());
        for (Uint32 i = 0; i < static_cast<Uint32>(mInitState.renderPassRenderTargetFormats.size()); ++i)
        {
            inOutMaterialPipelineInfo.renderTargetFormats[i] = mInitState.renderPassRenderTargetFormats[i];
        }
        inOutMaterialPipelineInfo.depthStencilFormat = mInitState.renderPassDepthStencilFormat;
    }

    void RGBuilder::AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists)
    {
        CHECK(mPhase == Phase::Init);
        CHECK(mInitState.isInRenderPass);

        MaterialPipelineStateInfo materialStateInfo = {};
        ApplyRenderTargetFormatsFromCurrentRenderPass(materialStateInfo);

        FrameVector<RGShaderParameterListBaseHandle> paramListArray(3);
        paramListArray.insert(paramListArray.end(), parameterLists.begin(), parameterLists.end());

        for (const DrawMeshInfo& drawMeshInfo : drawMeshInfos)
        {
            materialStateInfo.rasterizerState = drawMeshInfo.rasterizerState;
            materialStateInfo.depthStencilState = drawMeshInfo.depthStencilState;

            const MeshMetadata& meshMeta = drawMeshInfo.mesh->GetMeta();

            RGBufferHandle rgVertexBuffer = RegisterBuffer(drawMeshInfo.mesh->GetVertexBuffer()->GetGAPIBuffer());
            RGBufferSRVHandle rgVertexBufferSRV = CreateSRV(rgVertexBuffer);

            RGBufferHandle rgIndexBuffer = RegisterBuffer(drawMeshInfo.mesh->GetIndexBuffer()->GetGAPIBuffer());

            RGShaderParameterListHandle<ObjectShaderParameterList> objectShaderParameterList = CreateShaderParameterList<ObjectShaderParameterList>();
            objectShaderParameterList->model = drawMeshInfo.model;
            objectShaderParameterList->modelInverse = drawMeshInfo.model.Inversed();
            objectShaderParameterList->modelInverseTranspose = drawMeshInfo.model.Inversed().Transposed();
            objectShaderParameterList->vertexBuffer = rgVertexBufferSRV;
            objectShaderParameterList->useFP16 = meshMeta.useFloat16;
            paramListArray[0] = objectShaderParameterList;

            AddPassInternal(CUBE_T("##DrawMeshPass - Bind Index buffer"), nullptr, nullptr, {},
                [mesh = drawMeshInfo.mesh](gapi::CommandList& commandList){
                    commandList.BindIndexBuffer(mesh->GetIndexBuffer()->GetGAPIBuffer(), 0);
                },
                nullptr,
                false
            );

            const Vector<SubMesh>& subMeshes = drawMeshInfo.mesh->GetSubMeshes();
            for (const SubMesh& subMesh : subMeshes)
            {
                SharedPtr<Material> material = nullptr;
                if (0 <= subMesh.materialIndex && subMesh.materialIndex < drawMeshInfo.materials.size())
                {
                    material = drawMeshInfo.materials[subMesh.materialIndex].lock();
                }
                if (!material)
                {
                    material = mRenderer.GetDefaultMaterial();
                }
                SharedPtr<GraphicsPipeline> pipeline = mRenderer.GetShaderManager().GetMaterialShaderManager().GetOrCreateMaterialPipeline(material, materialStateInfo);
                RGShaderParameterListHandle<MaterialShaderParameterList> materialShaderParameterList = material->GenerateShaderParameterList(*this);
                paramListArray[1] = materialShaderParameterList;

                // HLSL does not apply baseVertex in SV_VertexID and later added SV_BaseVertexLocation in SM 6.8.
                // So transfer it via shader parameter and set 0 in DrawIndexed.
                // Metal applies it in vertex_id.
                // (See https://github.com/microsoft/DirectXShaderCompiler/pull/5770)
                auto subMeshShaderParameterList = CreateShaderParameterList<SubMeshShaderParameterList>();
                subMeshShaderParameterList->vertexBufferOffset = subMesh.vertexOffset;
                paramListArray[2] = subMeshShaderParameterList;

                AddPassInternal(Format<FrameString>(CUBE_T("Mesh: {0}[{1}] / Material: {2}"), drawMeshInfo.mesh->GetDebugName(), subMesh.debugName, material->GetDebugName()),
                    pipeline,
                    nullptr,
                    paramListArray,
                    [subMesh](gapi::CommandList& commandList)
                    {
                        commandList.DrawIndexed(subMesh.numIndices, subMesh.indexOffset, 0);
                    },
                    nullptr,
                    false
                );
            }
        }
    }

    void RGBuilder::UseResource(RGBufferSRVHandle rgSRV, gapi::ResourceSyncFlags syncs)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgSRV->mIndex,
            .syncs = syncs,
            .accesses = gapi::ResourceAccessFlag::SRV,
            .layout = gapi::ResourceLayout::Undefined,
        });
    }

    void RGBuilder::UseResource(RGBufferUAVHandle rgUAV, gapi::ResourceSyncFlags syncs)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgUAV->mIndex,
            .syncs = syncs,
            .accesses = gapi::ResourceAccessFlag::UAV,
            .layout = gapi::ResourceLayout::Undefined,
        });
    }

    void RGBuilder::UseResource(RGTextureSRVHandle rgSRV, gapi::ResourceSyncFlags syncs)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgSRV->mIndex,
            .syncs = syncs,
            .accesses = gapi::ResourceAccessFlag::SRV,
            .layout = gapi::ResourceLayout::SRV_Direct,
            .subresourceRange = rgSRV->GetSubresourceRange(),
        });
    }

    void RGBuilder::UseResource(RGTextureUAVHandle rgUAV, gapi::ResourceSyncFlags syncs)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgUAV->mIndex,
            .syncs = syncs,
            .accesses = gapi::ResourceAccessFlag::UAV,
            .layout = gapi::ResourceLayout::UAV_Direct,
            .subresourceRange = rgUAV->GetSubresourceRange(),
        });
    }

    void RGBuilder::UseResource(RGTextureRTVHandle rgRTV)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgRTV->mIndex,
            .syncs = gapi::ResourceSyncFlag::RenderTarget,
            .accesses = gapi::ResourceAccessFlag::RenderTarget,
            .layout = gapi::ResourceLayout::RenderTarget,
            .subresourceRange = rgRTV->GetSubresourceRange(),
        });
    }

    void RGBuilder::UseResource(RGTextureDSVHandle rgDSV)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgDSV->mIndex,
            .syncs = gapi::ResourceSyncFlag::DepthStencil,
            .accesses = gapi::ResourceAccessFlag::DepthStencilWrite,
            .layout = gapi::ResourceLayout::DepthStencilWrite,
            .subresourceRange = rgDSV->GetSubresourceRange(),
        });
    }

    void RGBuilder::UseResource(RGTextureHandle rgTexture, gapi::SubresourceRangeInput range, gapi::ResourceAccessFlags accesses, gapi::ResourceLayout layout, gapi::ResourceSyncFlags syncs)
    {
        CHECK(mPhase == Phase::TrackingResources);
        CHECK(rgTexture.IsValid());

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];
        pass.resourceUsages.push_back({
            .rgResourceIndex = rgTexture->mIndex,
            .syncs = syncs,
            .accesses = accesses,
            .layout = layout,
            .subresourceRange = range.Clamp(rgTexture->GetTextureInfo()),
        });
    }

    void RGBuilder::SkipUAVBarrier(RGBufferUAVHandle rgUAV)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];

        for (RGPass::ResourceUsage& usage : pass.resourceUsages)
        {
            if (usage.rgResourceIndex == rgUAV->mIndex)
            {
                usage.skipUAVBarrier = true;
                return;
            }
        }

        NO_ENTRY_FORMAT("You must call UseResource first before calling SkipUAVBarrier.");
    }

    void RGBuilder::SkipUAVBarrier(RGTextureUAVHandle rgUAV)
    {
        CHECK(mPhase == Phase::TrackingResources);

        RGPass& pass = mPasses[mTrackingResourcesState.passIndex];

        for (RGPass::ResourceUsage& usage : pass.resourceUsages)
        {
            if (usage.rgResourceIndex == rgUAV->mIndex)
            {
                usage.skipUAVBarrier = true;
                return;
            }
        }

        NO_ENTRY_FORMAT("You must call UseResource first before calling SkipUAVBarrier.");
    }

    void RGBuilder::ExecuteAndSubmit(gapi::CommandList& commandList)
    {
        commandList.Reset();
        commandList.Begin();

        Execute(commandList);

        commandList.End();
        commandList.Submit();
    }

    void RGBuilder::Execute(gapi::CommandList& commandList)
    {
        CHECK(mPhase == Phase::Init);
        CHECK(!mInitState.isInRenderPass);

        mPhase = Phase::TrackingResources;

        UpdateResourceUsages();
        CreateAllResources();
        ResolveBarriers();

        mPhase = Phase::Executing;

        commandList.BeginTimestamp(CUBE_T("RGBuilder"));

        for (RGPass& pass : mPasses)
        {
            const bool addGPUEvent = !pass.name.starts_with(CUBE_T("##"));

            if (addGPUEvent)
            {
                commandList.BeginEvent(pass.name);
            }

            if (pass.addTimestamp)
            {
                commandList.BeginTimestamp(pass.name);
            }

            ResolveShaderParameterListsAndPipeline(pass, commandList);
            MarkUseResources(pass, commandList);

            if (!pass.barriers.empty())
            {
                commandList.SetResourceBarrier(pass.barriers);
            }

            if (pass.passFunction)
            {
                pass.passFunction(commandList);
            }

            if (pass.addTimestamp)
            {
                commandList.EndTimestamp();
            }

            if (addGPUEvent)
            {
                commandList.EndEvent();
            }
        }

        commandList.SetResourceBarrier(mLastPass.barriers);

        commandList.EndTimestamp();

        mPhase = Phase::Executed;

        Reset();
    }

    void RGBuilder::BindGlobalShaderParameterListInternal(StringView name, RGShaderParameterListBaseHandle parameterList)
    {
        CHECK(mPhase == Phase::Init);

        // Add a pass that stores the parameter list. The parameter constant buffer will be bound automatically.
        AddPassInternal(Format<String>(CUBE_T("##BindGlobalShaderParameterList: {0}"), parameterList->mParameterListInfo.name),
            nullptr, nullptr, { &parameterList, 1 },
            nullptr,
            [nameStr = String(name), parameterList](RGBuilder& builder)
            {
                // Override the parameter list even if it already existed.
                builder.mTrackingResourcesState.boundGlobalShaderParameterLists[nameStr] = parameterList;
            },
            false
        );
    }

    void RGBuilder::UnbindGlobalShaderParameterListInternal(StringView name)
    {
        CHECK(mPhase == Phase::Init);

        // NOTE: The parameter constant buffer cannot be unbound while executing. It will be fixed after refactoring RGBuilder structure.
        AddPassInternal(Format<String>(CUBE_T("##UnbindGlobalShaderParameterList: {0}"), name),
            nullptr, nullptr, {},
            nullptr,
            [nameStr = String(name)](RGBuilder& builder)
            {
                builder.mTrackingResourcesState.boundGlobalShaderParameterLists.erase(nameStr);
            },
            false
        );
    }

    void RGBuilder::AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists,
        RGPass::PassFunction&& passFunction, RGPass::TrackResourceFunction&& trackResourceFunction,
        bool addTimestamp
    )
    {
        CHECK(mPhase == Phase::Init);

        CHECK(!graphicsPipeline || !computePipeline);
        CHECK_FORMAT(!computePipeline || !mInitState.isInRenderPass, "Cannot add compute pass in render pass.");

        mPasses.push_back({
            .name = String(name),
            .addTimestamp = addTimestamp,
            .shaderParameterLists = { parameterLists.begin(), parameterLists.end() },
            .graphicsPipeline = std::move(graphicsPipeline),
            .computePipeline = std::move(computePipeline),
            .passFunction = std::move(passFunction),
            .trackResourceFunction = std::move(trackResourceFunction)
        });
    }

    void RGBuilder::ResolveShaderParameterListsAndPipeline(RGPass& pass, gapi::CommandList& commandList)
    {
        CHECK(mPhase == Phase::Executing);

        // Register shader parameter lists in the pass.
        for (RGShaderParameterListBaseHandle& params : pass.shaderParameterLists)
        {
            const Character* name = params->mParameterListInfo.name;
            auto findIt = mExecuteState.shaderParameterListBindInfos.find(name);
            if (findIt == mExecuteState.shaderParameterListBindInfos.end())
            {
                findIt = mExecuteState.shaderParameterListBindInfos.insert({ name, {} }).first;
            }

            SharedPtr<ShaderParameterList> parameterList = params->mParameterList;
            if (findIt->second.GPUBuffer != parameterList->GetBuffer())
            {
                // Reset bind index because it is a new buffer.
                findIt->second = { parameterList->GetBuffer(), parameterList->GetSRV(), -1 };
            }
        }

        // Resolve shader parameter bind index.
        const gapi::ShaderReflection* pReflection = nullptr;
        if (pass.graphicsPipeline)
        {
            pReflection = &pass.graphicsPipeline->GetMergedShaderReflection();
        }
        else if (pass.computePipeline)
        {
            pReflection = &pass.computePipeline->GetShaderReflection();
        }
        if (pReflection)
        {
            for (const gapi::ShaderParameterBlockReflection& block : pReflection->blocks)
            {
                if (auto findIt = mExecuteState.shaderParameterListBindInfos.find(block.typeName); findIt != mExecuteState.shaderParameterListBindInfos.end())
                {
                    if (findIt->second.bindIndex != block.index)
                    {
                        commandList.SetConstantBuffer(block.index, findIt->second.srv);
                        findIt->second.bindIndex = block.index;
                    }
                }
                else
                {
                    CUBE_LOG(Error, RenderGraph, "Shader parameter list '{0}' is needed in shader '{1}' but not bounded!", block.typeName, pReflection->name);
                    NO_ENTRY();
                }
            }
        }

        // Bind pipeline.
        if (pass.graphicsPipeline)
        {
            if (mExecuteState.boundGraphicsPipeline != pass.graphicsPipeline)
            {
                mExecuteState.boundGraphicsPipeline = pass.graphicsPipeline;
                mExecuteState.boundComputePipeline = nullptr;

                commandList.SetGraphicsPipeline(mExecuteState.boundGraphicsPipeline->GetGAPIGraphicsPipeline());
            }
        }
        else if (pass.computePipeline)
        {
            if (mExecuteState.boundComputePipeline != pass.computePipeline)
            {
                mExecuteState.boundGraphicsPipeline = nullptr;
                mExecuteState.boundComputePipeline = pass.computePipeline;

                commandList.SetComputePipeline(mExecuteState.boundComputePipeline->GetGAPIComputePipeline());
            }
        }
    }

    void RGBuilder::MarkUseResources(RGPass& pass, gapi::CommandList& commandList)
    {
        CHECK(mPhase == Phase::Executing);

        if (pass.graphicsPipeline || pass.computePipeline)
        {
            for (const RGPass::ResourceUsage& resourceUsage : pass.resourceUsages)
            {
                RGResourceHandle rgResource = mResources[resourceUsage.rgResourceIndex];
                if (RGTextureSRVHandle rgSRV = rgResource.Cast<RGTextureSRV>(); rgSRV.IsValid())
                {
                    commandList.UseResource(rgSRV->GetSRV());
                }
                else if (RGTextureUAVHandle rgUAV = rgResource.Cast<RGTextureUAV>(); rgUAV.IsValid())
                {
                    commandList.UseResource(rgUAV->GetUAV());
                }
                else if (RGBufferSRVHandle rgSRV = rgResource.Cast<RGBufferSRV>(); rgSRV.IsValid())
                {
                    commandList.UseResource(rgSRV->GetSRV());
                }
                else if (RGBufferUAVHandle rgUAV = rgResource.Cast<RGBufferUAV>(); rgUAV.IsValid())
                {
                    commandList.UseResource(rgUAV->GetUAV());
                }
            }
        }
    }

    void RGBuilder::UpdateResourceUsages()
    {
        CHECK(mPhase == Phase::TrackingResources);

        mTrackingResourcesState.passIndex = 0;

        for (RGPass& pass : mPasses)
        {
            if (pass.trackResourceFunction)
            {
                pass.trackResourceFunction(*this);
            }

            gapi::ResourceSyncFlags syncs = gapi::ResourceSyncFlag::None;
            if (pass.IsGraphics())
            {
                // TODO: Narrow sync flags to only the pipeline stage each shader variables actually use.
                syncs = gapi::ResourceSyncFlag::Vertex | gapi::ResourceSyncFlag::Pixel;
            }
            else if (pass.IsCompute())
            {
                syncs = gapi::ResourceSyncFlag::Compute;
            }

            auto TryUseResource = [this, syncs](RGShaderParameterListBaseHandle& paramList)
            {
                ShaderParameterList* shaderParameterList = paramList->mParameterList.get();
                const Vector<ShaderParameterInfo>& shaderParameterInfos = paramList->mParameterListInfo.parameterInfos;
                for (const ShaderParameterInfo& shaderParameterInfo : shaderParameterInfos)
                {
                    Byte* src = reinterpret_cast<Byte*>(shaderParameterList) + shaderParameterInfo.offsetInCPU;

                    switch (shaderParameterInfo.type)
                    {
                    case ShaderParameterCPUType::RGBufferSRV:
                    {
                        RGBufferSRVHandle& srv = *reinterpret_cast<RGBufferSRVHandle*>(src);
                        CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(srv, syncs);
                        break;
                    }
                    case ShaderParameterCPUType::RGBufferUAV:
                    {
                        RGBufferUAVHandle& uav = *reinterpret_cast<RGBufferUAVHandle*>(src);
                        CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(uav, syncs);
                        break;
                    }
                    case ShaderParameterCPUType::RGTextureSRV:
                    {
                        RGTextureSRVHandle& srv = *reinterpret_cast<RGTextureSRVHandle*>(src);
                        CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(srv, syncs);
                        break;
                    }
                    case ShaderParameterCPUType::RGTextureUAV:
                    {
                        RGTextureUAVHandle& uav = *reinterpret_cast<RGTextureUAVHandle*>(src);
                        CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(uav, syncs);
                        break;
                    }
                    default:
                        break;
                    }
                }
            };

            for (auto& [_, globalParamList] : mTrackingResourcesState.boundGlobalShaderParameterLists)
            {
                TryUseResource(globalParamList);
            }

            for (RGShaderParameterListBaseHandle& paramList : pass.shaderParameterLists)
            {
                TryUseResource(paramList);
            }

            // Check if the resources currently attached to the render pass are being used.
#if CUBE_USE_CHECK
            if (mTrackingResourcesState.renderPassIndex != mTrackingResourcesState.passIndex)
            {
                for (const RGPass::ResourceUsage& resourceUsage : pass.resourceUsages)
                {
                    RGResourceHandle resource = mResources[resourceUsage.rgResourceIndex];

                    if (RGTextureHandle rgTexture = resource.Cast<RGTexture>(); rgTexture.IsValid())
                    {
                        for (RGTextureRTVHandle attachedRTV : mTrackingResourcesState.attachedRTVsInRenderPass)
                        {
                            CHECK_FORMAT(rgTexture != attachedRTV->mRGTexture || !resourceUsage.subresourceRange.IsOverlap(attachedRTV->mSubresourceRange),
                                "Cannot use subresource currently attached to render pass.");
                        }
                        if (mTrackingResourcesState.attachedDSVInRenderPass.IsValid())
                        {
                            CHECK_FORMAT(rgTexture != mTrackingResourcesState.attachedDSVInRenderPass->mRGTexture || !resourceUsage.subresourceRange.IsOverlap(mTrackingResourcesState.attachedDSVInRenderPass->mSubresourceRange),
                                "Cannot use subresource currently attached to render pass.");
                        }
                    }
                    else if (RGTextureViewHandle rgTextureView = resource.Cast<RGTextureView>(); rgTextureView.IsValid())
                    {
                        for (RGTextureRTVHandle attachedRTV : mTrackingResourcesState.attachedRTVsInRenderPass)
                        {
                            CHECK_FORMAT(!(attachedRTV->IsOverlap(*rgTextureView)), "Cannot use subresource currently attached to render pass.");
                        }
                        if (mTrackingResourcesState.attachedDSVInRenderPass.IsValid())
                        {
                            CHECK_FORMAT(!(mTrackingResourcesState.attachedDSVInRenderPass->IsOverlap(*rgTextureView)), "Cannot use subresource currently attached to render pass.");
                        }
                    }
                }
            }
#endif

            mTrackingResourcesState.passIndex++;
        }

        mTrackingResourcesState.passIndex = -1;
    }

    void RGBuilder::CreateAllResources()
    {
        CHECK(mPhase == Phase::TrackingResources);

        GAPI& gapi = mRenderer.GetGAPI();

        const int numPasses = static_cast<int>(mPasses.size());
        for (int i = 0; i < numPasses; ++i)
        {
            RGPass& pass = mPasses[i];
            mTrackingResourcesState.passIndex = i;

            for (const RGPass::ResourceUsage& resourceUsage : pass.resourceUsages)
            {
                RGResourceHandle resource = mResources[resourceUsage.rgResourceIndex];
                resource->CreateResource(gapi);
                resource->UpdateUsePassIndex(i);
            }
        }
        mTrackingResourcesState.passIndex = -1;

        // All RG resources were created, so write shader parameter lists at this time.
        for (RGResourceHandle resource : mResources)
        {
            if (RGShaderParameterListBaseHandle shaderParameterList = resource.Cast<RGShaderParameterListBase>(); shaderParameterList.IsValid())
            {
                shaderParameterList->mParameterList->WriteAllParametersToGPUBuffer();
            }
        }
    }

    void RGBuilder::ResolveBarriers()
    {
        CHECK(mPhase == Phase::TrackingResources);

        // Suspend emitting a barrier until a new barrier appears.
        // The syncDst cannot be determined until then.

        // Example logic:
        // (pending) PassA(Pixel-SRV) PassB(Compute-SRV)
        //   -> PassB uses the resource as SRV, which is the same as the previous pass.
        //      Don't need to insert a new barrier.
        // (pending) PassA(Pixel-SRV) PassB(Compute-SRV) PassC(Compute-UAV)
        //   -> PassC uses the resource as UAV, which is not the same as the previous pass.
        //      Need to insert a new barrier.
        // (barrier) PassA(Pixel-SRV) PassB(Compute-SRV) (pending) PassC(Compute-UAV)
        //   -> Emit the pending barrier because the syncDst is now determined.
        //      And add the next pending barrier before PassC.

        struct SubresourcePendingBarrier
        {
            gapi::ResourceType resourceType;

            gapi::ResourceSyncFlags lastSyncs = gapi::ResourceSyncFlag::None;
            gapi::ResourceAccessFlags lastAccesses = gapi::ResourceAccessFlag::NoAccess;
            gapi::ResourceLayout lastLayout = gapi::ResourceLayout::Undefined;

            gapi::ResourceSyncFlags syncs = gapi::ResourceSyncFlag::None;
            gapi::ResourceAccessFlags accesses = gapi::ResourceAccessFlag::NoAccess;
            gapi::ResourceLayout layout = gapi::ResourceLayout::Undefined;

            int firstPassIndex = -1;

            gapi::ResourceBarrier EmitBufferBarrier(SharedPtr<gapi::Buffer> buffer) const
            {
                CHECK(resourceType == gapi::ResourceType::Buffer);

                return gapi::ResourceBarrier{
                    .resourceType = gapi::ResourceBarrier::ResourceType::Buffer,
                    .buffer = buffer,
                    .syncSrc = lastSyncs,
                    .syncDst = syncs,
                    .accessSrc = lastAccesses,
                    .accessDst = accesses,
                    .layoutSrc = gapi::ResourceLayout::Undefined,
                    .layoutDst = gapi::ResourceLayout::Undefined,
                };
            }
            gapi::ResourceBarrier EmitTextureBarrier(SharedPtr<gapi::Texture> texture, Uint32 subresourceIndex) const
            {
                CHECK(resourceType == gapi::ResourceType::Texture);

                return gapi::ResourceBarrier{
                    .resourceType = gapi::ResourceBarrier::ResourceType::Texture,
                    .texture = texture,
                    .subresourceIndex = static_cast<int>(subresourceIndex),
                    .discard = (lastLayout == gapi::ResourceLayout::Undefined),
                    .syncSrc = lastSyncs,
                    .syncDst = syncs,
                    .accessSrc = lastAccesses,
                    .accessDst = accesses,
                    .layoutSrc = lastLayout,
                    .layoutDst = layout,
                };
            }
            void MoveNext(int newPassIndex, gapi::ResourceAccessFlags newAccesses, gapi::ResourceLayout newLayout)
            {
                lastSyncs = syncs;
                lastAccesses = accesses;
                lastLayout = layout;

                syncs = gapi::ResourceSyncFlag::None;
                accesses = newAccesses;
                layout = newLayout;

                firstPassIndex = newPassIndex;
            }
        };
        FrameVector<FrameVector<SubresourcePendingBarrier>> pendingBarriersByResource(mResources.size());

        const int numPasses = static_cast<int>(mPasses.size());
        for (int passIndex = 0; passIndex < numPasses; ++passIndex)
        {
            RGPass& pass = mPasses[passIndex];
            mTrackingResourcesState.passIndex = passIndex;

            for (const RGPass::ResourceUsage& resourceUsage : pass.resourceUsages)
            {
                RGResourceHandle resource = mResources[resourceUsage.rgResourceIndex];

                auto TryResolveBuffer = [&](RGBufferHandle rgBuffer)
                {
                    SharedPtr<gapi::Buffer> buffer = rgBuffer->GetGAPIBuffer();

                    FrameVector<SubresourcePendingBarrier>& pendingSubresourceBarriers = pendingBarriersByResource[rgBuffer->mIndex];
                    if (pendingSubresourceBarriers.empty())
                    {
                        pendingSubresourceBarriers.push_back({
                            .resourceType = gapi::ResourceType::Buffer,
                        });
                    }

                    SubresourcePendingBarrier& pendingBarrier = pendingSubresourceBarriers[0];
                    const bool needUAVBarrier =
                        !resourceUsage.skipUAVBarrier
                        && pendingBarrier.accesses.IsSet(gapi::ResourceAccessFlag::UAV)
                        && resourceUsage.accesses.IsSet(gapi::ResourceAccessFlag::UAV);
                    if (needUAVBarrier || pendingBarrier.accesses != resourceUsage.accesses)
                    {
                        if (pendingBarrier.firstPassIndex != -1)
                        {
                            mPasses[pendingBarrier.firstPassIndex].barriers.push_back(pendingBarrier.EmitBufferBarrier(buffer));
                        }
                        pendingBarrier.MoveNext(mTrackingResourcesState.passIndex, resourceUsage.accesses, gapi::ResourceLayout::Undefined);
                    }
                    pendingBarrier.syncs |= resourceUsage.syncs;
                };

                auto TryResolveTexture = [&](RGTextureHandle rgTexture, const gapi::SubresourceRange& subresourceRange)
                {
                    SharedPtr<gapi::Texture> texture = rgTexture->mTexture;

                    for (Uint32 sliceIndex = subresourceRange.firstSliceIndex; sliceIndex < subresourceRange.firstSliceIndex + subresourceRange.sliceSize; ++sliceIndex)
                    {
                        for (Uint32 mipLevel = subresourceRange.firstMipLevel; mipLevel < subresourceRange.firstMipLevel + subresourceRange.mipLevels; ++mipLevel)
                        {
                            FrameVector<SubresourcePendingBarrier>& pendingSubresourceBarriers = pendingBarriersByResource[rgTexture->mIndex];
                            if (pendingSubresourceBarriers.empty())
                            {
                                SubresourcePendingBarrier initBarrier = {
                                    .resourceType = gapi::ResourceType::Texture,
                                };
                                if (!(rgTexture->IsTransient()))
                                {
                                    auto registeredTextureInfoIt = mRegisteredTextureInfos.find(rgTexture->GetGAPITexture().get());
                                    CHECK(registeredTextureInfoIt != mRegisteredTextureInfos.end());

                                    initBarrier.lastLayout = registeredTextureInfoIt->second.srcLayout;
                                }
                                pendingSubresourceBarriers.resize(texture->GetNumSubresources(), initBarrier);
                                CHECK_FORMAT(!pendingSubresourceBarriers.empty(), "Empty number of subresource in the texture. The texture is not initialized properly.");
                            }

                            const Uint32 subresourceIndex = texture->GetSubresourceIndex(sliceIndex, mipLevel);
                            SubresourcePendingBarrier& pendingBarrier = pendingSubresourceBarriers[subresourceIndex];

                            const bool needUAVBarrier =
                                !resourceUsage.skipUAVBarrier
                                && pendingBarrier.accesses.IsSet(gapi::ResourceAccessFlag::UAV)
                                && resourceUsage.accesses.IsSet(gapi::ResourceAccessFlag::UAV);

                            if (needUAVBarrier ||
                                (pendingBarrier.accesses != resourceUsage.accesses
                                || pendingBarrier.layout != resourceUsage.layout))
                            {
                                if (pendingBarrier.firstPassIndex != -1)
                                {
                                    mPasses[pendingBarrier.firstPassIndex].barriers.push_back(pendingBarrier.EmitTextureBarrier(texture, subresourceIndex));
                                }
                                pendingBarrier.MoveNext(mTrackingResourcesState.passIndex, resourceUsage.accesses, resourceUsage.layout);
                            }
                            pendingBarrier.syncs |= resourceUsage.syncs;
                        }
                    }
                };

                if (RGBufferViewHandle bufferView = resource.Cast<RGBufferView>(); bufferView.IsValid())
                {
                    TryResolveBuffer(bufferView->mRGBuffer);
                }
                else if (RGBufferHandle buffer = resource.Cast<RGBuffer>(); buffer.IsValid())
                {
                    TryResolveBuffer(buffer);
                }
                else if (RGTextureViewHandle textureView = resource.Cast<RGTextureView>(); textureView.IsValid())
                {
                    TryResolveTexture(textureView->mRGTexture, textureView->GetSubresourceRange());
                }
                else if (RGTextureHandle texture = resource.Cast<RGTexture>(); texture.IsValid())
                {
                    TryResolveTexture(texture, resourceUsage.subresourceRange);
                }
                else
                {
                    NO_ENTRY_FORMAT("Barrier is not supported in this RGResource.");
                }
            }
        }

        // Emit all pending barriers and set the layout of registered resources at the last pass.
        for (Uint32 index = 0; index < pendingBarriersByResource.size(); ++index)
        {
            RGResourceHandle rgResource = mResources[index];
            FrameVector<SubresourcePendingBarrier>& pendingBarriersEachSubresources = pendingBarriersByResource[index];

            RGBufferHandle rgBuffer = rgResource.Cast<RGBuffer>();
            RGTextureHandle rgTexture = rgResource.Cast<RGTexture>();
            gapi::ResourceLayout dstLayout = gapi::ResourceLayout::Undefined;
            if (rgTexture.IsValid() && !(rgTexture->IsTransient()))
            {
                auto registeredTextureInfoIt = mRegisteredTextureInfos.find(rgTexture->mTexture.get());
                CHECK(registeredTextureInfoIt != mRegisteredTextureInfos.end());

                dstLayout = registeredTextureInfoIt->second.dstLayout;
            }

            for (Uint32 subresourceIndex = 0; subresourceIndex < pendingBarriersEachSubresources.size(); ++subresourceIndex)
            {
                SubresourcePendingBarrier& pendingBarrier = pendingBarriersEachSubresources[subresourceIndex];

                if (rgBuffer.IsValid())
                {
                    if (pendingBarrier.firstPassIndex != -1)
                    {
                        mPasses[pendingBarrier.firstPassIndex].barriers.push_back(pendingBarrier.EmitBufferBarrier(rgBuffer->GetGAPIBuffer()));
                    }
                    // NOTE: It is okay not to insert a barrier after the last pass because RGBuilder will submit
                    // the command list after writing the last pass.
                    // (An implicit global barrier is inserted between submitted command lists.)
                }
                else if (rgTexture.IsValid())
                {
                    if (pendingBarrier.firstPassIndex != -1)
                    {
                        mPasses[pendingBarrier.firstPassIndex].barriers.push_back(pendingBarrier.EmitTextureBarrier(rgTexture->GetGAPITexture(), subresourceIndex));
                    }

                    if (!(rgTexture->IsTransient()))
                    {
                        // NOTE: It is okay to set NoAccess/None after the last pass because RGBuilder will submit
                        // the command list after writing the last pass.
                        // (An implicit global barrier is inserted between submitted command lists.)
                        // A barrier is still needed for the layout transition.
                        if (pendingBarrier.layout != dstLayout)
                        {
                            pendingBarrier.MoveNext(-1, gapi::ResourceAccessFlag::NoAccess, dstLayout);
                            pendingBarrier.syncs = gapi::ResourceSyncFlag::None;
                            mLastPass.barriers.push_back(pendingBarrier.EmitTextureBarrier(rgTexture->mTexture, subresourceIndex));
                        }
                    }
                }
                else
                {
                    NO_ENTRY();
                }
            }
        }

        mTrackingResourcesState.passIndex = -1;
    }

    void RGBuilder::Reset()
    {
        mExecuteState.Reset();
        mTrackingResourcesState.Reset();
        mInitState.Reset();

        mPasses.clear();
        mLastPass = {};
        mRegisteredTextureInfos.clear();
        mRegisteredBufferInfos.clear();
        mCachedBufferViews.clear();
        mCachedTextureViews.clear();
        for (RGResourceHandle resource : mResources)
        {
            delete resource.mResource;
        }
        mResources.clear();

        mPhase = Phase::Init;
    }
} // namespace cube
