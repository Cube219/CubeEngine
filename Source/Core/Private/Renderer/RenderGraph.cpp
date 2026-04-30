#include "Renderer/RenderGraph.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_CommandList.h"
#include "Shader.h"
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

    RGBufferView::RGBufferView(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
        : RGResource(index, rgBuffer->GetDebugName())
        , mRGBuffer(rgBuffer)
        , mFormat(format)
        , mFirstElement(firstElement)
        , mNumElements(numElements)
    {
        mIsTransient = rgBuffer->IsTransient();
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

    RGBufferSRV::RGBufferSRV(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
        : RGBufferView(index, rgBuffer, format, firstElement, numElements)
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

    RGBufferUAV::RGBufferUAV(int index, RGBuffer* rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
        : RGBufferView(index, rgBuffer, format, firstElement, numElements)
    {
    }

    Uint64 RGTexture::GetSubresourceHashKey(const gapi::SubresourceRange& range) const
    {
        return HashCombine(reinterpret_cast<Uint64>(mTexture.get()), range.GetHash());
    }

    void RGTexture::CreateResource(GAPI& gapi)
    {
        if (mIsTransient && !mTexture)
        {
            mTexture = gapi.CreateTexture({
                .usage = gapi::ResourceUsage::Transient,
                .textureInfo = mTextureInfo,
                .debugName = mDebugName
            });
        }
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
    {
        mIsTransient = texture->GetUsage() == gapi::ResourceUsage::Transient;
        CHECK_FORMAT(!mIsTransient, "Cannot register transient texture!");

        mTextureInfo = texture->GetInfo();
    }

    RGTextureView::RGTextureView(int index, RGTexture* rgTexture)
        : RGResource(index, rgTexture->GetDebugName())
        , mRGTexture(rgTexture)
        , mSubresourceHashKey(0)
    {
        mIsTransient = rgTexture->IsTransient();
    }

    void RGTextureSRV::CreateResource(GAPI& gapi)
    {
        if (!mSRV)
        {
            mRGTexture->CreateResource(gapi);

            mSRV = mRGTexture->mTexture->CreateSRV({
                .subresourceRange = mSubresourceRangeInput
            });
            mSubresourceRange = mSRV->GetSubresourceRange();
            mSubresourceHashKey = mRGTexture->GetSubresourceHashKey(mSubresourceRange);
        }
    }

    RGTextureSRV::RGTextureSRV(int index, RGTexture* rgTexture, Uint32 firstMipLevel, Uint32 mipLevels)
        : RGTextureView(index, rgTexture)
    {
        mSubresourceRangeInput.firstMipLevel = firstMipLevel;
        mSubresourceRangeInput.mipLevels = mipLevels;
    }

    void RGTextureUAV::CreateResource(GAPI& gapi)
    {
        if (!mUAV)
        {
            mRGTexture->CreateResource(gapi);

            mUAV = mRGTexture->mTexture->CreateUAV({
                .subresourceRange = mSubresourceRangeInput
            });
            mSubresourceRange = mUAV->GetSubresourceRange();
            mSubresourceHashKey = mRGTexture->GetSubresourceHashKey(mSubresourceRange);
        }
    }

    RGTextureUAV::RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel, Uint32 firstSliceIndex, Uint32 sliceSize)
        : RGTextureView(index, rgTexture)
    {
        mSubresourceRangeInput.firstMipLevel = mipLevel;
        mSubresourceRangeInput.mipLevels = 1;
        mSubresourceRangeInput.firstSliceIndex = firstSliceIndex;
        mSubresourceRangeInput.sliceSize = sliceSize;
    }

    void RGTextureRTV::CreateResource(GAPI& gapi)
    {
        if (!mRTV)
        {
            mRGTexture->CreateResource(gapi);

            mRTV = mRGTexture->mTexture->CreateRTV({
                .subresourceRange = mSubresourceRangeInput
            });
            mSubresourceRange = mRTV->GetSubresourceRange();
            mSubresourceHashKey = mRGTexture->GetSubresourceHashKey(mSubresourceRange);
        }
    }

    RGTextureRTV::RGTextureRTV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture)
    {
        mSubresourceRangeInput.firstMipLevel = mipLevel;
        mSubresourceRangeInput.mipLevels = 1;
    }

    void RGTextureDSV::CreateResource(GAPI& gapi)
    {
        if (!mDSV)
        {
            mRGTexture->CreateResource(gapi);

            mDSV = mRGTexture->mTexture->CreateDSV({
                .subresourceRange = mSubresourceRangeInput
            });
            mSubresourceRange = mDSV->GetSubresourceRange();
            mSubresourceHashKey = mRGTexture->GetSubresourceHashKey(mSubresourceRange);
        }
    }

    RGTextureDSV::RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture)
    {
        mSubresourceRangeInput.firstMipLevel = mipLevel;
        mSubresourceRangeInput.mipLevels = 1;
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
        if (auto findIt = mRegisteredBuffers.find(buffer.get()); findIt != mRegisteredBuffers.end())
        {
            return findIt->second;
        }

        RGBuffer* rgBuffer = new RGBuffer(mResources.size(), buffer);
        mResources.push_back(rgBuffer);

        mRegisteredBuffers.insert({ buffer.get(), RGBufferHandle(rgBuffer) });

        return RGBufferHandle(rgBuffer);
    }

    RGBufferHandle RGBuilder::CreateBuffer(const gapi::BufferInfo& bufferInfo, StringView debugName)
    {
        RGBuffer* rgBuffer = new RGBuffer(mResources.size(), bufferInfo, debugName);
        mResources.push_back(rgBuffer);

        return RGBufferHandle(rgBuffer);
    }

    RGBufferSRVHandle RGBuilder::CreateSRV(RGBufferHandle rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
    {
        RGBufferSRV* rgSRV = new RGBufferSRV(mResources.size(), rgBuffer.mResource, format, firstElement, numElements);
        mResources.push_back(rgSRV);

        return RGBufferSRVHandle(rgSRV);
    }

    RGBufferUAVHandle RGBuilder::CreateUAV(RGBufferHandle rgBuffer, gapi::ElementFormat format, Uint64 firstElement, Uint64 numElements)
    {
        RGBufferUAV* rgUAV = new RGBufferUAV(mResources.size(), rgBuffer.mResource, format, firstElement, numElements);
        mResources.push_back(rgUAV);

        return RGBufferUAVHandle(rgUAV);
    }

    RGTextureHandle RGBuilder::RegisterTexture(SharedPtr<gapi::Texture> texture)
    {
        if (auto findIt = mRegisteredTextures.find(texture.get()); findIt != mRegisteredTextures.end())
        {
            return findIt->second;
        }

        RGTexture* rgTexture = new RGTexture(mResources.size(), texture);
        mResources.push_back(rgTexture);

        mRegisteredTextures.insert({ texture.get(), RGTextureHandle(rgTexture) });

        return RGTextureHandle(rgTexture);
    }

    RGTextureHandle RGBuilder::CreateTexture(const gapi::TextureInfo& textureInfo, StringView debugName)
    {
        RGTexture* rgTexture = new RGTexture(mResources.size(), textureInfo, debugName);
        mResources.push_back(rgTexture);

        return RGTextureHandle(rgTexture);
    }

    RGTextureSRVHandle RGBuilder::CreateSRV(RGTextureHandle rgTexture, Uint32 firstMipLevel, Int32 mipLevels)
    {
        RGTextureSRV* rgSRV = new RGTextureSRV(mResources.size(), rgTexture.mResource, firstMipLevel, mipLevels);
        mResources.push_back(rgSRV);

        return RGTextureSRVHandle(rgSRV);
    }

    RGTextureUAVHandle RGBuilder::CreateUAV(RGTextureHandle rgTexture, Uint32 mipLevel, Uint32 firstSliceIndex, Int32 sliceSize)
    {
        RGTextureUAV* rgUAV = new RGTextureUAV(mResources.size(), rgTexture.mResource, mipLevel, firstSliceIndex, sliceSize);
        mResources.push_back(rgUAV);

        return RGTextureUAVHandle(rgUAV);
    }

    RGTextureRTVHandle RGBuilder::CreateRTV(RGTextureHandle rgTexture, Uint32 mipLevel)
    {
        RGTextureRTV* rgRTV = new RGTextureRTV(mResources.size(), rgTexture.mResource, mipLevel);
        mResources.push_back(rgRTV);

        return RGTextureRTVHandle(rgRTV);
    }

    RGTextureDSVHandle RGBuilder::CreateDSV(RGTextureHandle rgTexture, Uint32 mipLevel)
    {
        RGTextureDSV* rgDSV = new RGTextureDSV(mResources.size(), rgTexture.mResource, mipLevel);
        mResources.push_back(rgDSV);

        return RGTextureDSVHandle(rgDSV);
    }

    RGTextureSRVHandle RGBuilder::GetDummyBlackTexture2D()
    {
        if (!mDummyBlackTexture2D.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyBlackTexture2D());
            mDummyBlackTexture2D = CreateSRV(rgTexture);
        }

        return mDummyBlackTexture2D;
    }

    RGTextureSRVHandle RGBuilder::GetDummyBlackTextureCube()
    {
        if (!mDummyBlackTextureCube.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyBlackTextureCube());
            mDummyBlackTextureCube = CreateSRV(rgTexture);
        }

        return mDummyBlackTextureCube;
    }

    RGTextureSRVHandle RGBuilder::GetDummyWhiteTexture2D()
    {
        if (!mDummyWhiteTexture2D.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyWhiteTexture2D());
            mDummyWhiteTexture2D = CreateSRV(rgTexture);
        }

        return mDummyWhiteTexture2D;
    }

    void RGBuilder::BeginRenderPass(const RenderPassInfo& info)
    {
        CHECK(mState == State::Init);
        CHECK(!mIsInRenderPass);

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
                .dsv = info.depthstencil.dsv.IsValid() ? info.depthstencil.dsv->GetDSV() : nullptr,
                .loadOperation = info.depthstencil.loadOperation,
                .storeOperation = info.depthstencil.storeOperation,
                .clearDepth = info.depthstencil.clearDepth
            };

            commandList.BeginRenderPass(colors, depthStencil);
        },
        [info](RGBuilder& builder)
        {
            for (const RenderPassInfo::ColorAttachment& color : info.colors)
            {
                builder.UseResource(color.color);
            }
            if (info.depthstencil.dsv.IsValid())
            {
                builder.UseResource(info.depthstencil.dsv);
            }

            builder.mAttachedRTVsInRenderPass.reserve(info.colors.size());
            for (const RenderPassInfo::ColorAttachment& colorAttachment : info.colors)
            {
                builder.mAttachedRTVsInRenderPass.push_back(colorAttachment.color);
            }
            builder.mAttachedDSVInRenderPass = info.depthstencil.dsv;
            builder.mRenderPassIndex = builder.mCurrentPassIndex;
            builder.mIsInRenderPass = true;
        });

        mIsInRenderPass = true;
    }

    void RGBuilder::EndRenderPass()
    {
        CHECK(mState == State::Init);
        CHECK(mIsInRenderPass);

        AddPass(CUBE_T("##EndRenderPass"), [](gapi::CommandList& commandList)
        {
            commandList.EndRenderPass();
        },
        [](RGBuilder& builder)
        {
            // Just mark use in RG resource to prevent duplicated transition.
            for (const RGTextureRTVHandle attachedRTV : builder.mAttachedRTVsInRenderPass)
            {
                attachedRTV->UpdateUsePassIndex(builder.mCurrentPassIndex);
            }
            if (builder.mAttachedDSVInRenderPass.IsValid())
            {
                builder.mAttachedDSVInRenderPass->UpdateUsePassIndex(builder.mCurrentPassIndex);
            }

            builder.mAttachedDSVInRenderPass = {};
            builder.mAttachedRTVsInRenderPass.clear();
            builder.mRenderPassIndex = -1;
            builder.mIsInRenderPass = false;
        });

        mIsInRenderPass = false;
    }

    void RGBuilder::AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists)
    {
        CHECK(mState == State::Init);

        FrameVector<RGShaderParameterListBaseHandle> paramListArray(2);
        paramListArray.insert(paramListArray.end(), parameterLists.begin(), parameterLists.end());

        for (const DrawMeshInfo& drawMeshInfo : drawMeshInfos)
        {
            RGShaderParameterListHandle<ObjectShaderParameterList> objectShaderParameterList = CreateShaderParameterList<ObjectShaderParameterList>();
            objectShaderParameterList->Get()->model = drawMeshInfo.model;
            objectShaderParameterList->Get()->modelInverse = drawMeshInfo.model.Inversed();
            objectShaderParameterList->Get()->modelInverseTranspose = drawMeshInfo.model.Inversed().Transposed();
            paramListArray[0] = objectShaderParameterList;

            AddPassInternal(CUBE_T("##DrawMeshPass - Bind Vertex/Index buffer"), nullptr, nullptr, {},
            [mesh = drawMeshInfo.mesh](gapi::CommandList& commandList){
                Uint32 vertexBufferOffset = 0;
                SharedPtr<gapi::Buffer> vertexBuffer = mesh->GetVertexBuffer();
                commandList.BindVertexBuffers(0, { &vertexBuffer, 1 }, { &vertexBufferOffset, 1 });
                commandList.BindIndexBuffer(mesh->GetIndexBuffer(), 0);
            },
            nullptr);

            const Vector<SubMesh>& subMeshes = drawMeshInfo.mesh->GetSubMeshes();
            for (const SubMesh& subMesh : subMeshes)
            {
                SharedPtr<Material> material;
                if (subMesh.materialIndex < drawMeshInfo.materials.size())
                {
                    material = drawMeshInfo.materials[subMesh.materialIndex];
                }
                else
                {
                    material = mRenderer.GetDefaultMaterial();
                }
                SharedPtr<GraphicsPipeline> pipeline = mRenderer.GetShaderManager().GetMaterialShaderManager().GetOrCreateMaterialPipeline(material, drawMeshInfo.meshMetaData, drawMeshInfo.fillMode);
                RGShaderParameterListHandle<MaterialShaderParameterList> materialShaderParameterList = material->GenerateShaderParameterList(*this);
                paramListArray[1] = materialShaderParameterList;

                AddPassInternal(Format<FrameString>(CUBE_T("Mesh: {0}, Material: {1}"), subMesh.debugName, material->GetDebugName()),
                    pipeline,
                    nullptr,
                    paramListArray,
                    [subMesh](gapi::CommandList& commandList)
                {
                    commandList.DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                },
                nullptr);
            }
        }
    }

    void RGBuilder::UseResource(RGBufferSRVHandle rgSRV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgSRV->mIndex,
            .state = (!pass.graphicsPipeline) ? gapi::ResourceStateFlag::SRV_NonPixel : gapi::ResourceStateFlag::SRV_Pixel
        });
    }

    void RGBuilder::UseResource(RGBufferUAVHandle rgUAV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgUAV->mIndex,
            .state = gapi::ResourceStateFlag::UAV
        });
    }

    void RGBuilder::UseResource(RGTextureSRVHandle rgSRV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgSRV->mIndex,
            .state = (!pass.graphicsPipeline) ? gapi::ResourceStateFlag::SRV_NonPixel : gapi::ResourceStateFlag::SRV_Pixel
        });
    }

    void RGBuilder::UseResource(RGTextureUAVHandle rgUAV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgUAV->mIndex,
            .state = gapi::ResourceStateFlag::UAV
        });
    }

    void RGBuilder::UseResource(RGTextureRTVHandle rgRTV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgRTV->mIndex,
            .state = gapi::ResourceStateFlag::RenderTarget
        });
    }

    void RGBuilder::UseResource(RGTextureDSVHandle rgDSV)
    {
        CHECK(mState == State::ResourceTracking);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgDSV->mIndex,
            .state = gapi::ResourceStateFlag::DepthWrite
        });
    }

    void RGBuilder::UseResource(RGTextureHandle rgTexture, gapi::SubresourceRangeInput range, gapi::ResourceStateFlags states)
    {
        CHECK(mState == State::ResourceTracking);
        CHECK(rgTexture.IsValid());

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgTexture->mIndex,
            .state = states,
            .subresourceRange = range
        });
    }

    void RGBuilder::ExecuteAndSubmit(gapi::CommandList& commandList)
    {
        CHECK(mState == State::Init);
        CHECK(!mIsInRenderPass);

        mState = State::ResourceTracking;

        UpdateResourceUsages();
        CreateAllResources();
        ResolveTransitions();

        mState = State::Executing;

        commandList.Reset();
        commandList.Begin();

        commandList.InsertTimestamp(CUBE_T("Begin RGBuilder"));

        for (PassInfo& pass : mPasses)
        {
            const bool addGPUEvent = !pass.name.starts_with(CUBE_T("##"));

            if (addGPUEvent)
            {
                commandList.BeginEvent(pass.name);
            }

            ResolveShaderParameterListsAndPipeline(pass, commandList);
            MarkUseResources(pass, commandList);

            if (!pass.transitions.empty())
            {
                commandList.ResourceTransition(pass.transitions);
            }

            if (pass.passFunction)
            {
                pass.passFunction(commandList);
            }

            if (addGPUEvent)
            {
                commandList.EndEvent();
            }
        }

        commandList.ResourceTransition(mLastPass.transitions);

        commandList.InsertTimestamp(CUBE_T("End RGBuilder"));

        commandList.End();
        commandList.Submit();

        mState = State::Submitted;

        Reset();
    }

    void RGBuilder::BindShaderParameterListInternal(StringView name, RGShaderParameterListBaseHandle parameterList)
    {
        CHECK(mState == State::Init);

        // Add pass that just store parameter list. Resources in the parameter list will be tracked automatically.
        AddPassInternal(Format<String>(CUBE_T("##BindShaderParameterList: {0}"), parameterList->mParameterListInfo.name),
            nullptr, nullptr, { &parameterList, 1 }, nullptr, nullptr);
    }

    void RGBuilder::AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, ConstArrayView<RGShaderParameterListBaseHandle> parameterLists, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction)
    {
        CHECK(mState == State::Init);

        CHECK(!graphicsPipeline || !computePipeline);
        CHECK_FORMAT(!computePipeline || !mIsInRenderPass, "Cannot add compute pass in render pass.");

        int index = static_cast<int>(mPasses.size());
        mPasses.push_back({
            .name = String(name),
            .index = index,
            .shaderParameterLists = { parameterLists.begin(), parameterLists.end() },
            .graphicsPipeline = std::move(graphicsPipeline),
            .computePipeline = std::move(computePipeline),
            .passFunction = std::move(passFunction),
            .useResourceFunction = std::move(useResourceFunction)
        });
    }

    void RGBuilder::ResolveShaderParameterListsAndPipeline(PassInfo& pass, gapi::CommandList& commandList)
    {
        CHECK(mState == State::Executing);

        // Register shader parameter lists in the pass.
        for (RGShaderParameterListBaseHandle& params : pass.shaderParameterLists)
        {
            const Character* name = params->mParameterListInfo.name;
            auto findIt = mShaderParameterListBindInfos.find(name);
            if (findIt == mShaderParameterListBindInfos.end())
            {
                findIt = mShaderParameterListBindInfos.insert({ name, {} }).first;
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
                if (auto findIt = mShaderParameterListBindInfos.find(block.typeName); findIt != mShaderParameterListBindInfos.end())
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
            if (mCurrentBoundGraphicsPipeline != pass.graphicsPipeline)
            {
                mCurrentBoundGraphicsPipeline = pass.graphicsPipeline;
                mCurrentBoundComputePipeline = nullptr;

                commandList.SetGraphicsPipeline(mCurrentBoundGraphicsPipeline->GetGAPIGraphicsPipeline());
            }
        }
        else if (pass.computePipeline)
        {
            if (mCurrentBoundComputePipeline != pass.computePipeline)
            {
                mCurrentBoundGraphicsPipeline = nullptr;
                mCurrentBoundComputePipeline = pass.computePipeline;

                commandList.SetComputePipeline(mCurrentBoundComputePipeline->GetGAPIComputePipeline());
            }
        }
    }

    void RGBuilder::MarkUseResources(PassInfo& pass, gapi::CommandList& commandList)
    {
        CHECK(mState == State::Executing);

        if (pass.graphicsPipeline || pass.computePipeline)
        {
            for (const PassInfo::ResourceUseInfo& resourceUseInfo : pass.resourceUseInfos)
            {
                RGResource* rgResource = mResources[resourceUseInfo.rgResourceIndex];
                if (RGTextureSRV* rgSRV = dynamic_cast<RGTextureSRV*>(rgResource))
                {
                    commandList.UseResource(rgSRV->GetSRV());
                }
                else if (RGTextureUAV* rgUAV = dynamic_cast<RGTextureUAV*>(rgResource))
                {
                    commandList.UseResource(rgUAV->GetUAV());
                }
            }
        }
    }

    void RGBuilder::UpdateResourceUsages()
    {
        CHECK(mState == State::ResourceTracking);

        mCurrentPassIndex = 0;

        for (PassInfo& pass : mPasses)
        {
            if (pass.useResourceFunction)
            {
                pass.useResourceFunction(*this);
            }

            for (RGShaderParameterListBaseHandle& paramList : pass.shaderParameterLists)
            {
                ShaderParameterList* shaderParameterList = paramList->mParameterList.get();
                const Vector<ShaderParameterInfo>& shaderParameterInfos = paramList->mParameterListInfo.parameterInfos;
                for (const ShaderParameterInfo& shaderParameterInfo : shaderParameterInfos)
                {
                    Byte* src = reinterpret_cast<Byte*>(shaderParameterList) + shaderParameterInfo.offsetInCPU;

                    switch (shaderParameterInfo.type)
                    {
                    case ShaderParameterType::RGBufferSRV:
                    {
                        RGBufferSRVHandle& srv = *reinterpret_cast<RGBufferSRVHandle*>(src);
                        CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(srv);
                        break;
                    }
                    case ShaderParameterType::RGBufferUAV:
                    {
                        RGBufferUAVHandle& uav = *reinterpret_cast<RGBufferUAVHandle*>(src);
                        CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(uav);
                        break;
                    }
                    case ShaderParameterType::RGTextureSRV:
                    {
                        RGTextureSRVHandle& srv = *reinterpret_cast<RGTextureSRVHandle*>(src);
                        CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(srv);
                        break;
                    }
                    case ShaderParameterType::RGTextureUAV:
                    {
                        RGTextureUAVHandle& uav = *reinterpret_cast<RGTextureUAVHandle*>(src);
                        CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}'.", shaderParameterInfo.name);

                        UseResource(uav);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            // Check if the resources will be used currently attached to render pass.
#if CUBE_USE_CHECK
            if (mRenderPassIndex != mCurrentPassIndex)
            {
                for (const PassInfo::ResourceUseInfo& resourceUseInfo : pass.resourceUseInfos)
                {
                    RGResource* resource = mResources[resourceUseInfo.rgResourceIndex];

                    if (RGTexture* rgTexture = dynamic_cast<RGTexture*>(resource))
                    {
                        for (RGTextureRTVHandle attachedRTV : mAttachedRTVsInRenderPass)
                        {
                            CHECK_FORMAT(rgTexture != attachedRTV->mRGTexture || !resourceUseInfo.subresourceRange.IsOverlap(attachedRTV->mSubresourceRangeInput),
                                "Cannot use subresource currently attached to render pass.");
                        }
                        if (mAttachedDSVInRenderPass.IsValid())
                        {
                            CHECK_FORMAT(rgTexture != mAttachedDSVInRenderPass->mRGTexture || !resourceUseInfo.subresourceRange.IsOverlap(mAttachedDSVInRenderPass->mSubresourceRangeInput),
                                "Cannot use subresource currently attached to render pass.");
                        }
                    }
                    else if (RGTextureView* rgTextureView = dynamic_cast<RGTextureView*>(resource))
                    {
                        for (RGTextureRTVHandle attachedRTV : mAttachedRTVsInRenderPass)
                        {
                            CHECK_FORMAT(!(attachedRTV->IsOverlap(rgTextureView)), "Cannot use subresource currently attached to render pass.");
                        }
                        if (mAttachedDSVInRenderPass.IsValid())
                        {
                            CHECK_FORMAT(!(mAttachedDSVInRenderPass->IsOverlap(rgTextureView)), "Cannot use subresource currently attached to render pass.");
                        }
                    }
                }
            }
#endif

            mCurrentPassIndex++;
        }

        mCurrentPassIndex = -1;
    }

    void RGBuilder::CreateAllResources()
    {
        CHECK(mState == State::ResourceTracking);

        GAPI& gapi = mRenderer.GetGAPI();

        const int numPasses = static_cast<int>(mPasses.size());
        for (int i = 0; i < numPasses; ++i)
        {
            PassInfo& pass = mPasses[i];
            mCurrentPassIndex = i;

            for (const PassInfo::ResourceUseInfo& resourceUseInfo : pass.resourceUseInfos)
            {
                RGResource* resource = mResources[resourceUseInfo.rgResourceIndex];
                resource->CreateResource(gapi);
                resource->UpdateUsePassIndex(i);
            }
        }
        mCurrentPassIndex = -1;

        // All RG resources were created, so write shader parameter lists at this time.
        for (RGResource* resource : mResources)
        {
            if (RGShaderParameterListBase* shaderParameterList = dynamic_cast<RGShaderParameterListBase*>(resource))
            {
                shaderParameterList->mParameterList->WriteAllParametersToGPUBuffer();
            }
        }
    }

    void RGBuilder::ResolveTransitions()
    {
        CHECK(mState == State::ResourceTracking);

        struct SubresourceKey
        {
            SharedPtr<gapi::Texture> texture;
            Uint32 subresourceIndex;
            bool needRollback;
            bool operator<(const SubresourceKey& rhs) const
            {
                return (texture == rhs.texture) ? subresourceIndex < rhs.subresourceIndex : texture < rhs.texture;
            }
        };
        FrameMap<SubresourceKey, gapi::ResourceStateFlags> currentSubresourceStates;
        struct BufferKey
        {
            SharedPtr<gapi::Buffer> buffer;
            bool needRollback;
            bool operator<(const BufferKey& rhs) const
            {
                return buffer < rhs.buffer;
            }
        };
        FrameMap<BufferKey, gapi::ResourceStateFlags> currentBufferStates;

        int numPasses = static_cast<int>(mPasses.size());
        for (int i = 0; i < numPasses; ++i)
        {
            PassInfo& pass = mPasses[i];
            mCurrentPassIndex = i;

            for (const PassInfo::ResourceUseInfo& resourceUseInfo : pass.resourceUseInfos)
            {
                RGResource* resource = mResources[resourceUseInfo.rgResourceIndex];

                auto TryTransitionBuffer = [&](RGBuffer* rgBuffer)
                {
                    SharedPtr<gapi::Buffer> buffer = rgBuffer->mBuffer;

                    const BufferKey key = { buffer, !(rgBuffer->IsTransient()) };
                    auto currentStateIt = currentBufferStates.find(key);
                    if (currentStateIt == currentBufferStates.end())
                    {
                        currentStateIt = currentBufferStates.insert({ key, gapi::ResourceStateFlag::Common }).first;
                    }

                    gapi::ResourceStateFlags currentState = currentStateIt->second;
                    if (currentState != resourceUseInfo.state)
                    {
                        gapi::TransitionState& transition = pass.transitions.emplace_back();
                        transition.resourceType = gapi::TransitionState::ResourceType::Buffer;
                        transition.buffer = buffer;
                        transition.src = currentState;
                        transition.dst = resourceUseInfo.state;
                    }
                    currentStateIt->second = resourceUseInfo.state;
                };

                auto TryTransitionTexture = [&](RGTexture* rgTexture, const gapi::SubresourceRange& subresourceRange)
                {
                    SharedPtr<gapi::Texture> texture = rgTexture->mTexture;

                    for (Uint32 sliceIndex = subresourceRange.firstSliceIndex; sliceIndex < subresourceRange.firstSliceIndex + subresourceRange.sliceSize; ++sliceIndex)
                    {
                        for (Uint32 mipLevel = subresourceRange.firstMipLevel; mipLevel < subresourceRange.firstMipLevel + subresourceRange.mipLevels; ++mipLevel)
                        {
                            const Uint32 subresourceIndex = texture->GetSubresourceIndex(sliceIndex, mipLevel);
                            const SubresourceKey key = { texture, subresourceIndex, !(rgTexture->IsTransient()) };
                            auto currentStateIt = currentSubresourceStates.find(key);
                            if (currentStateIt == currentSubresourceStates.end())
                            {
                                currentStateIt = currentSubresourceStates.insert({ key, gapi::ResourceStateFlag::Common }).first;
                            }

                            gapi::ResourceStateFlags currentState = currentStateIt->second;
                            if (currentState != resourceUseInfo.state)
                            {
                                gapi::TransitionState& transition = pass.transitions.emplace_back();
                                transition.resourceType = gapi::TransitionState::ResourceType::Texture;
                                transition.texture = texture;
                                transition.subresourceIndex = subresourceIndex;
                                transition.src = currentState;
                                transition.dst = resourceUseInfo.state;
                            }

                            currentStateIt->second = resourceUseInfo.state;
                        }
                    }
                };

                if (RGBufferView* bufferView = dynamic_cast<RGBufferView*>(resource))
                {
                    TryTransitionBuffer(bufferView->mRGBuffer);
                }
                else if (RGBuffer* buffer = dynamic_cast<RGBuffer*>(resource))
                {
                    TryTransitionBuffer(buffer);
                }
                else if (RGTextureView* textureView = dynamic_cast<RGTextureView*>(resource))
                {
                    TryTransitionTexture(textureView->mRGTexture, textureView->GetSubresourceRange());
                }
                else if (RGTexture* texture = dynamic_cast<RGTexture*>(resource))
                {
                    TryTransitionTexture(texture, resourceUseInfo.subresourceRange.Clamp(texture->GetGAPITexture().get()));
                }
                else
                {
                    NO_ENTRY_FORMAT("Transition is not supported in this RGResource.");
                }
            }
        }

        // Rollback transition at the last pass for non-transient resources.
        for (auto& [key, state] : currentSubresourceStates)
        {
            if (key.needRollback)
            {
                gapi::TransitionState& transition = mLastPass.transitions.emplace_back();
                transition.resourceType = gapi::TransitionState::ResourceType::Texture;
                transition.texture = key.texture;
                transition.subresourceIndex = key.subresourceIndex;
                transition.src = state;
                transition.dst = gapi::ResourceStateFlag::Common;
            }
        }
        for (auto& [key, state] : currentBufferStates)
        {
            if (key.needRollback)
            {
                gapi::TransitionState& transition = mLastPass.transitions.emplace_back();
                transition.resourceType = gapi::TransitionState::ResourceType::Buffer;
                transition.buffer = key.buffer;
                transition.src = state;
                transition.dst = gapi::ResourceStateFlag::Common;
            }
        }

        mCurrentPassIndex = -1;
    }

    void RGBuilder::Reset()
    {
        mPasses.clear();
        mLastPass = {};
        mRegisteredTextures.clear();
        mRegisteredBuffers.clear();
        for (RGResource* resource : mResources)
        {
            delete resource;
        }
        mResources.clear();

        mRenderPassIndex = -1;
        mAttachedDSVInRenderPass = {};
        mAttachedRTVsInRenderPass.clear();
        mIsInRenderPass = false;

        mState = State::Init;
    }
} // namespace cube
