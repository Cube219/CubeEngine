#include "Renderer/RenderGraph.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_CommandList.h"
#include "Shader.h"
#include "Texture.h"

namespace cube
{
    // ===== Resources =====

    RGResource::RGResource(Int32 index)
        : mIsTransient(false)
        , mIndex(index)
        , mBeginPass(-1)
        , mEndPass(-1)
    {
    }

    Uint64 RGTexture::GetSubresourceHashKey(const gapi::SubresourceRange& range) const
    {
        return HashCombine(reinterpret_cast<Uint64>(mTexture.get()), range.GetHash());
    }

    RGTexture::RGTexture(int index, const gapi::TextureInfo& textureInfo)
        : RGResource(index)
        , mTexture(nullptr)
        , mTextureInfo(textureInfo)
    {
        mIsTransient = true;
    }

    RGTexture::RGTexture(int index, SharedPtr<gapi::Texture> texture)
        : RGResource(index)
        , mTexture(texture)
    {
        mIsTransient = texture->GetUsage() == gapi::ResourceUsage::Transient;
    }

    RGTextureView::RGTextureView(int index, RGTexture* rgTexture)
        : RGResource(index)
        , mRGTexture(rgTexture)
        , mSubresourceHashKey(0)
    {
        mIsTransient = rgTexture->IsTransient();
    }

    RGTextureSRV::RGTextureSRV(int index, RGTexture* rgTexture, Uint32 firstMipLevel, Int32 mipLevels)
        : RGTextureView(index, rgTexture)
    {
        mSRV = rgTexture->mTexture->CreateSRV({
            .firstMipLevel = firstMipLevel,
            .mipLevels = mipLevels
        });
        mSubresourceRange = mSRV->GetSubresourceRange();
        mSubresourceHashKey = rgTexture->GetSubresourceHashKey(mSubresourceRange);
    }
    
    RGTextureUAV::RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture)
    {
        mUAV = rgTexture->mTexture->CreateUAV({
            .mipLevel = mipLevel
        });
        mSubresourceRange = mUAV->GetSubresourceRange();
        mSubresourceHashKey = rgTexture->GetSubresourceHashKey(mSubresourceRange);
    }
    
    RGTextureRTV::RGTextureRTV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture)
    {
        mRTV = rgTexture->mTexture->CreateRTV({
            .mipLevel = mipLevel
        });
        mSubresourceRange = mRTV->GetSubresourceRange();
        mSubresourceHashKey = rgTexture->GetSubresourceHashKey(mSubresourceRange);
    }
    
    RGTextureDSV::RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture)
    {
        mDSV = rgTexture->mTexture->CreateDSV({
            .mipLevel = mipLevel
        });
        mSubresourceRange = mDSV->GetSubresourceRange();
        mSubresourceHashKey = rgTexture->GetSubresourceHashKey(mSubresourceRange);
    }

    RGShaderParameterListBase::RGShaderParameterListBase(int index, const ShaderParameterListInfo& parameterListInfo, SharedPtr<ShaderParameterList> parameterList)
        : RGResource(index)
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

    RGTextureHandle RGBuilder::CreateTexture(const gapi::TextureInfo& textureInfo, StringView debugName)
    {
        // TODO: Defer texture creation.
        const gapi::TextureCreateInfo createInfo = {
            .usage = gapi::ResourceUsage::Transient,
            .textureInfo = textureInfo,
            .debugName = debugName
        };

        SharedPtr<gapi::Texture> newTransientTexture = mRenderer.GetGAPI().CreateTexture(createInfo);
        mTransientTextures.push_back(newTransientTexture);

        RGTexture* rgTexture = new RGTexture(mResources.size(), newTransientTexture);
        mResources.push_back(rgTexture);

        return RGTextureHandle(rgTexture);
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

    RGTextureSRVHandle RGBuilder::CreateSRV(RGTextureHandle rgTexture, Uint32 firstMipLevel, Int32 mipLevels)
    {
        RGTextureSRV* rgSRV = new RGTextureSRV(mResources.size(), rgTexture.mResource, firstMipLevel, mipLevels);
        mResources.push_back(rgSRV);

        return RGTextureSRVHandle(rgSRV);
    }

    RGTextureUAVHandle RGBuilder::CreateUAV(RGTextureHandle rgTexture, Uint32 mipLevel)
    {
        RGTextureUAV* rgUAV = new RGTextureUAV(mResources.size(), rgTexture.mResource, mipLevel);
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

    RGTextureSRVHandle RGBuilder::GetDummyBlackTexture()
    {
        if (!mDummyBlackTexture.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyBlackTexture());
            mDummyBlackTexture = CreateSRV(rgTexture);
        }

        return mDummyBlackTexture;
    }

    RGTextureSRVHandle RGBuilder::GetDummyWhiteTexture()
    {
        if (!mDummyWhiteTexture.IsValid())
        {
            RGTextureHandle rgTexture = RegisterTexture(mRenderer.GetDummyWhiteTexture());
            mDummyWhiteTexture = CreateSRV(rgTexture);
        }

        return mDummyWhiteTexture;
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
        });

        mIsInRenderPass = true;
        // TODO: Extend resource usage lifetime until ending render pass?
        // TODO: Check if the resources bounded in render pass is used in other usage.
    }

    void RGBuilder::EndRenderPass()
    {
        CHECK(mState == State::Init);
        CHECK(mIsInRenderPass);

        AddPass(CUBE_T("##EndRenderPass"), [](gapi::CommandList& commandList)
        {
            commandList.EndRenderPass();
        });

        mIsInRenderPass = false;
    }

    void RGBuilder::AddDrawMeshPass(StringView name, ArrayView<DrawMeshInfo> drawMeshInfos)
    {
        CHECK(mState == State::Init);

        for (const DrawMeshInfo& drawMeshInfo : drawMeshInfos)
        {
            RGShaderParameterListHandle<ObjectShaderParameterList> objectShaderParameterList = CreateShaderParameterList<ObjectShaderParameterList>();
            objectShaderParameterList->Get()->model = drawMeshInfo.model;
            objectShaderParameterList->Get()->modelInverse = drawMeshInfo.model.Inversed();
            objectShaderParameterList->Get()->modelInverseTranspose = drawMeshInfo.model.Inversed().Transposed();
            objectShaderParameterList->Get()->WriteAllParametersToGPUBuffer();

            AddPassInternal(CUBE_T("##DrawMeshPass - Bind Vertex/Index buffer and object shader parameter list"), nullptr, nullptr, objectShaderParameterList,
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

                AddPassInternal(Format<FrameString>(CUBE_T("Mesh: {0}, Material: {1}"),
                    subMesh.debugName, material->GetDebugName()), pipeline, nullptr, materialShaderParameterList,
                [subMesh](gapi::CommandList& commandList) {
                    commandList.DrawIndexed(subMesh.numIndices, subMesh.indexOffset, subMesh.vertexOffset);
                },
                nullptr);
            }
        }
    }

    void RGBuilder::UseResource(RGTextureSRVHandle rgSRV)
    {
        CHECK(mState == State::ResourceTracking);
        CHECK(rgSRV->mSRV);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgSRV->mIndex,
            .state = (!pass.graphicsPipeline) ? gapi::ResourceStateFlag::SRV_NonPixel : gapi::ResourceStateFlag::SRV_Pixel
        });
    }

    void RGBuilder::UseResource(RGTextureUAVHandle rgUAV)
    {
        CHECK(mState == State::ResourceTracking);
        CHECK(rgUAV->mUAV);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgUAV->mIndex,
            .state = gapi::ResourceStateFlag::UAV
        });
    }

    void RGBuilder::UseResource(RGTextureRTVHandle rgRTV)
    {
        CHECK(mState == State::ResourceTracking);
        CHECK(rgRTV->mRTV);

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.resourceUseInfos.push_back({
            .rgResourceIndex = rgRTV->mIndex,
            .state = gapi::ResourceStateFlag::RenderTarget
        });
    }

    void RGBuilder::UseResource(RGTextureDSVHandle rgDSV)
    {
        CHECK(mState == State::ResourceTracking);
        CHECK(rgDSV->mDSV);

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
            .subresourceRange = range.Clamp(rgTexture->mTexture.get())
        });
    }

    void RGBuilder::ExecuteAndSubmit(gapi::CommandList& commandList)
    {
        CHECK(mState == State::Init);
        CHECK(!mIsInRenderPass);

        mState = State::ResourceTracking;

        UpdateResourceUsagesInShaderParameterList();
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

            ResolveShaderParameterListAndPipeline(pass, commandList);
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
            nullptr, nullptr, parameterList, nullptr, nullptr);
    }

    void RGBuilder::AddPassInternal(StringView name, SharedPtr<GraphicsPipeline> graphicsPipeline, SharedPtr<ComputePipeline> computePipeline, RGShaderParameterListBaseHandle parameterList, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction)
    {
        CHECK(mState == State::Init);

        CHECK(!graphicsPipeline || !computePipeline);
        CHECK_FORMAT(!computePipeline || !mIsInRenderPass, "Cannot add compute pass in render pass.");

        int index = static_cast<int>(mPasses.size());
        mPasses.push_back({
            .name = String(name),
            .index = index,
            .shaderParameterList = parameterList,
            .graphicsPipeline = std::move(graphicsPipeline),
            .computePipeline = std::move(computePipeline),
            .passFunction = std::move(passFunction),
            .useResourceFunction = std::move(useResourceFunction)
        });
    }

    void RGBuilder::ResolveShaderParameterListAndPipeline(PassInfo& pass, gapi::CommandList& commandList)
    {
        CHECK(mState == State::Executing);

        // Register shader parameter list in the pass.
        if (pass.shaderParameterList.IsValid())
        {
            const Character* name = pass.shaderParameterList->mParameterListInfo.name;
            auto findIt = mShaderParameterListBindInfos.find(name);
            if (findIt == mShaderParameterListBindInfos.end())
            {
                findIt = mShaderParameterListBindInfos.insert({ name, {} }).first;
            }

            SharedPtr<ShaderParameterList> parameterList = pass.shaderParameterList->mParameterList;
            // Reset bind index because it is a new buffer.
            findIt->second = { parameterList->GetBuffer(), -1 };
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
                        commandList.SetShaderVariableConstantBuffer(block.index, findIt->second.GPUBuffer);
                        findIt->second.bindIndex = block.index;
                    }
                }
                else
                {
                    CUBE_LOG(Error, RenderGraph, "Shader parameter list '{0}' is needed in shader '{1}' but not bounded!", block.typeName, pReflection->name);
                    CHECK(false);
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

    void RGBuilder::UpdateResourceUsagesInShaderParameterList()
    {
        CHECK(mState == State::ResourceTracking);

        mCurrentPassIndex = 0;

        for (PassInfo& pass : mPasses)
        {
            if (pass.shaderParameterList.IsValid())
            {
                ShaderParameterList* shaderParameterList = pass.shaderParameterList->mParameterList.get();
                const Vector<ShaderParameterInfo>& shaderParameterInfos = pass.shaderParameterList->mParameterListInfo.parameterInfos;
                for (const ShaderParameterInfo& shaderParameterInfo : shaderParameterInfos)
                {
                    Byte* src = reinterpret_cast<Byte*>(shaderParameterList) + shaderParameterInfo.offsetInCPU;

                    switch (shaderParameterInfo.type)
                    {
                    case ShaderParameterType::RGTextureSRV:
                    {
                        RGTextureSRVHandle& srv = *reinterpret_cast<RGTextureSRVHandle*>(src);
                        CHECK_FORMAT(srv.IsValid(), "Null srv in shader parameter '{0}.", shaderParameterInfo.name);

                        UseResource(srv);
                        break;
                    }
                    case ShaderParameterType::RGTextureUAV:
                    {
                        RGTextureUAVHandle& uav = *reinterpret_cast<RGTextureUAVHandle*>(src);
                        CHECK_FORMAT(uav.IsValid(), "Null uav in shader parameter '{0}.", shaderParameterInfo.name);

                        UseResource(uav);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            mCurrentPassIndex++;
        }

        mCurrentPassIndex = -1;
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

        int numPasses = static_cast<int>(mPasses.size());
        for (int i = 0; i < numPasses; ++i)
        {
            PassInfo& pass = mPasses[i];
            mCurrentPassIndex = i;

            if (pass.useResourceFunction)
            {
                pass.useResourceFunction(*this);
            }

            for (const PassInfo::ResourceUseInfo& resourceUseInfo : pass.resourceUseInfos)
            {
                RGResource* resource = mResources[resourceUseInfo.rgResourceIndex];

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

                if (RGTextureView* textureView = dynamic_cast<RGTextureView*>(resource))
                {
                    TryTransitionTexture(textureView->mRGTexture, textureView->GetSubresourceRange());
                }
                else if (RGTexture* texture = dynamic_cast<RGTexture*>(resource))
                {
                    TryTransitionTexture(texture, resourceUseInfo.subresourceRange);
                }
                else
                {
                    CHECK_FORMAT(false, "Transition is not supported in this RGResource.");
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

        mCurrentPassIndex = -1;
    }

    void RGBuilder::Reset()
    {
        mPasses.clear();
        mRegisteredTextures.clear();
        for (RGResource* resource : mResources)
        {
            delete resource;
        }
        mResources.clear();

        mIsInRenderPass = false;

        mState = State::Init;
    }
} // namespace cube
