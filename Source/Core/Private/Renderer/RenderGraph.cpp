#include "RenderGraph.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "GAPI_CommandList.h"
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

    RGResource::~RGResource()
    {
    }

    RGTexture::RGTexture(int index, SharedPtr<gapi::Texture> texture)
        : RGResource(index)
        , mTexture(texture)
    {
        mIsTransient = false;
    }

    RGTexture::~RGTexture()
    {
    }

    RGTextureView::RGTextureView(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGResource(index)
        , mRGTexture(rgTexture)
        , mMipLevel(mipLevel)
    {
        mSubresourceHashKey = HashCombine(reinterpret_cast<Uint64>(mRGTexture), mMipLevel);
    }

    RGTextureView::~RGTextureView()
    {
    }

    RGTextureSRV::RGTextureSRV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture, mipLevel)
        , mSRV(nullptr)
    {
    }
    
    RGTextureSRV::~RGTextureSRV()
    {
    }
    
    RGTextureUAV::RGTextureUAV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture, mipLevel)
        , mUAV(nullptr)
    {
    }
    
    RGTextureUAV::~RGTextureUAV()
    {
    }
    
    RGTextureRTV::RGTextureRTV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture, mipLevel)
        , mRTV(nullptr)
    {
    }
    
    RGTextureRTV::~RGTextureRTV()
    {
    }
    
    RGTextureDSV::RGTextureDSV(int index, RGTexture* rgTexture, Uint32 mipLevel)
        : RGTextureView(index, rgTexture, mipLevel)
        , mDSV(nullptr)
    {
    }

    RGTextureDSV::~RGTextureDSV()
    {
    }

    // ===== Builder =====

    RGBuilder::~RGBuilder()
    {
        Reset();
    }

    RGTexture* RGBuilder::RegisterTexture(SharedPtr<gapi::Texture> texture)
    {
        // TODO: Check duplication
        RGTexture* rgTexture = new RGTexture(mResources.size(), texture);
        mResources.push_back(rgTexture);

        return rgTexture;
    }

    RGTextureSRV* RGBuilder::CreateSRV(RGTexture* rgTexture, Uint32 mipLevel)
    {
        RGTextureSRV* rgSRV = new RGTextureSRV(mResources.size(), rgTexture, mipLevel);
        mResources.push_back(rgSRV);

        return rgSRV;
    }
    
    RGTextureUAV* RGBuilder::CreateUAV(RGTexture* rgTexture, Uint32 mipLevel)
    {
        RGTextureUAV* rgUAV = new RGTextureUAV(mResources.size(), rgTexture, mipLevel);
        mResources.push_back(rgUAV);

        return rgUAV;
    }
    
    RGTextureRTV* RGBuilder::CreateRTV(RGTexture* rgTexture, Uint32 mipLevel)
    {
        RGTextureRTV* rgRTV = new RGTextureRTV(mResources.size(), rgTexture, mipLevel);
        mResources.push_back(rgRTV);

        return rgRTV;
    }
    
    RGTextureDSV* RGBuilder::CreateDSV(RGTexture* rgTexture, Uint32 mipLevel)
    {
        RGTextureDSV* rgDSV = new RGTextureDSV(mResources.size(), rgTexture, mipLevel);
        mResources.push_back(rgDSV);

        return rgDSV;
    }

    void RGBuilder::BeginRenderPass(const RenderPassInfo& info)
    {
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
                .dsv = info.depthstencil.dsv ? info.depthstencil.dsv->GetDSV() : nullptr,
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
            if (info.depthstencil.dsv)
            {
                builder.UseResource(info.depthstencil.dsv);
            }
        });

        mIsInRenderPass = true;
    }

    void RGBuilder::EndRenderPass()
    {
        CHECK(mIsInRenderPass);

        AddPass(CUBE_T("##EndRenderPass"), [](gapi::CommandList& commandList)
        {
            commandList.EndRenderPass();
        });

        mIsInRenderPass = false;
    }

    void RGBuilder::AddPass(StringView name, PassFunction&& passFunction, UseResourceFunction&& useResourceFunction, bool isCompute)
    {
        CHECK(!mIsExecuting);

        int index = static_cast<int>(mPasses.size());
        mPasses.emplace_back(String(name), index, std::move(passFunction), isCompute);
        mUseResourceFunction.emplace_back(std::move(useResourceFunction));
    }

    void RGBuilder::UseResource(RGTextureSRV* rgSRV)
    {
        CHECK(!mIsExecuting);

        if (rgSRV->mSRV == nullptr)
        {
            rgSRV->mSRV = rgSRV->mRGTexture->mTexture->CreateSRV({
                .firstMipLevel = rgSRV->mMipLevel,
                .mipLevels = 1
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = rgSRV->mIndex,
            .state = pass.isCompute ? gapi::ResourceStateFlag::SRV_NonPixel : gapi::ResourceStateFlag::SRV_Pixel
        });
    }

    void RGBuilder::UseResource(RGTextureUAV* rgUAV)
    {
        CHECK(!mIsExecuting);

        if (rgUAV->mUAV == nullptr)
        {
            rgUAV->mUAV = rgUAV->mRGTexture->mTexture->CreateUAV({
                .mipLevel = rgUAV->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = rgUAV->mIndex,
            .state = gapi::ResourceStateFlag::UAV
        });
    }

    void RGBuilder::UseResource(RGTextureRTV* rgRTV)
    {
        CHECK(!mIsExecuting);

        if (rgRTV->mRTV == nullptr)
        {
            rgRTV->mRTV = rgRTV->mRGTexture->mTexture->CreateRTV({
                .mipLevel = rgRTV->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = rgRTV->mIndex,
            .state = gapi::ResourceStateFlag::RenderTarget
        });
    }

    void RGBuilder::UseResource(RGTextureDSV* rgDSV)
    {
        CHECK(!mIsExecuting);

        if (rgDSV->mDSV == nullptr)
        {
            rgDSV->mDSV = rgDSV->mRGTexture->mTexture->CreateDSV({
                .mipLevel = rgDSV->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = rgDSV->mIndex,
            .state = gapi::ResourceStateFlag::DepthWrite
        });
    }

    void RGBuilder::ExecuteAndSubmit(gapi::CommandList& commandList)
    {
        CHECK(!mIsInRenderPass);

        {
            AddPass(CUBE_T("##Last Pass"), [](gapi::CommandList& commandList) {},
            [](RGBuilder& builder)
            {
                builder.RollbackResourceStates();
            });
        }

        ResolveTransitions();

        mIsExecuting = true;

        commandList.Reset();
        commandList.Begin();

        commandList.InsertTimestamp(CUBE_T("Begin RGBuilder"));

        for (const PassInfo& pass : mPasses)
        {
            bool addGPUEvent = !pass.name.starts_with(CUBE_T("##"));

            if (addGPUEvent)
            {
                commandList.BeginEvent(pass.name);
            }

            if (!pass.transitions.empty())
            {
                commandList.ResourceTransition(pass.transitions);
            }

            pass.passFunction(commandList);

            if (addGPUEvent)
            {
                commandList.EndEvent();
            }
        }

        commandList.InsertTimestamp(CUBE_T("End RGBuilder"));

        commandList.End();
        commandList.Submit();

        mIsExecuting = false;

        Reset();
    }

    void RGBuilder::ResolveTransitions()
    {
        int numPasses = mPasses.size();
        CHECK(numPasses == mUseResourceFunction.size());

        FrameMap<Uint64, gapi::ResourceStateFlags> currentSubresourceStates;

        for (int i = 0; i < numPasses; ++i)
        {
            PassInfo& pass = mPasses[i];
            mCurrentPassIndex = i;

            mUseResourceFunction[i](*this);

            for (const PassInfo::UseState& useStateInPass : pass.useStates)
            {
                RGResource* resource = mResources[useStateInPass.resourceIndex];

                if (RGTextureView* textureView = dynamic_cast<RGTextureView*>(resource))
                {
                    Uint64 subresourceHashKey = textureView->GetSubresourceHashKey();
                    CHECK(subresourceHashKey != 0);
                    auto currentStateIt = currentSubresourceStates.find(subresourceHashKey);
                    if (currentStateIt == currentSubresourceStates.end())
                    {
                        currentStateIt = currentSubresourceStates.insert({ subresourceHashKey, gapi::ResourceStateFlag::Common }).first;
                    }
                    gapi::ResourceStateFlags currentState = currentStateIt->second;

                    if (currentState != useStateInPass.state)
                    {
                        RGTexture* texture = textureView->mRGTexture;

                        gapi::TransitionState& transition = pass.transitions.emplace_back();
                        transition.resourceType = gapi::TransitionState::ResourceType::Texture;
                        transition.texture = texture->mTexture;
                        transition.subresourceRange = {
                            .firstMipLevel = textureView->mMipLevel,
                            .mipLevels = 1,
                            .firstArrayIndex = 0,
                            .arraySize = texture->mTexture->GetArraySize()
                        };
                        transition.src = currentState;
                        transition.dst = useStateInPass.state;
                    }

                    currentStateIt->second = useStateInPass.state;
                }
                else
                {
                    CHECK_FORMAT(false, "Transition is not supported in this RGResource.");
                }
            }
        }

        mCurrentPassIndex = -1;
    }

    void RGBuilder::RollbackResourceStates()
    {
        CHECK(!mIsExecuting);

        for (RGResource* resource : mResources)
        {
            if (RGTextureView* textureView = dynamic_cast<RGTextureView*>(resource))
            {
                PassInfo& pass = mPasses[mCurrentPassIndex];
                pass.useStates.push_back({
                    .resourceIndex = textureView->mIndex,
                    .state = gapi::ResourceStateFlag::Common
                });
            }
        }
    }

    void RGBuilder::Reset()
    {
        mPasses.clear();
        mUseResourceFunction.clear();
        for (RGResource* resource : mResources)
        {
            delete resource;
        }
        mResources.clear();

        mIsInRenderPass = false;
        mIsExecuting = false;
    }
} // namespace cube
