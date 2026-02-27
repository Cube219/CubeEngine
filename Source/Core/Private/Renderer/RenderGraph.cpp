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

    RGTexture::RGTexture(int index, SharedPtr<gapi::Texture> texture, Uint32 mipLevel)
        : RGResource(index)
        , mTexture(texture)
        , mSRV(nullptr)
        , mUAV(nullptr)
        , mRTV(nullptr)
        , mDSV(nullptr)
        , mMipLevel(mipLevel)
    {
        mIsTransient = false;
    }

    RGTexture::~RGTexture()
    {
    }

    // ===== Builder =====

    RGTexture* RGBuilder::RegisterTexture(SharedPtr<gapi::Texture> texture, Uint32 mipLevel)
    {
        // TODO: Check duplication
        RGTexture* rgTexture = new RGTexture(mResources.size(), texture, mipLevel);
        mResources.push_back(rgTexture);

        return rgTexture;
    }

    RGBuilder::~RGBuilder()
    {
        Reset();
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
                builder.UseRTV(color.color);
            }
            if (info.depthstencil.dsv)
            {
                builder.UseDSV(info.depthstencil.dsv);
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

    void RGBuilder::UseSRV(RGTexture* texture)
    {
        CHECK(!mIsExecuting);

        if (texture->mSRV == nullptr)
        {
            texture->mSRV = texture->mTexture->CreateSRV({
                .firstMipLevel = texture->mMipLevel,
                .mipLevels = 1
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = texture->mIndex,
            .state = pass.isCompute ? gapi::ResourceStateFlag::SRV_NonPixel : gapi::ResourceStateFlag::SRV_Pixel
        });
    }

    void RGBuilder::UseUAV(RGTexture* texture)
    {
        CHECK(!mIsExecuting);

        if (texture->mUAV == nullptr)
        {
            texture->mUAV = texture->mTexture->CreateUAV({
                .mipLevel = texture->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = texture->mIndex,
            .state = gapi::ResourceStateFlag::UAV
        });
    }

    void RGBuilder::UseRTV(RGTexture* texture)
    {
        CHECK(!mIsExecuting);

        if (texture->mRTV == nullptr)
        {
            texture->mRTV = texture->mTexture->CreateRTV({
                .mipLevel = texture->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = texture->mIndex,
            .state = gapi::ResourceStateFlag::RenderTarget
        });
    }

    void RGBuilder::UseDSV(RGTexture* texture)
    {
        CHECK(!mIsExecuting);

        if (texture->mDSV == nullptr)
        {
            texture->mDSV = texture->mTexture->CreateDSV({
                .mipLevel = texture->mMipLevel
            });
        }

        PassInfo& pass = mPasses[mCurrentPassIndex];
        pass.useStates.push_back({
            .resourceIndex = texture->mIndex,
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

        FrameVector<gapi::ResourceStateFlags> currentResourceStates(mResources.size(), gapi::ResourceStateFlag::Common);

        for (int i = 0; i < numPasses; ++i)
        {
            PassInfo& pass = mPasses[i];
            mCurrentPassIndex = i;

            mUseResourceFunction[i](*this);

            for (const PassInfo::UseState& useState : pass.useStates)
            {
                if (currentResourceStates[useState.resourceIndex] != useState.state)
                {
                    RGTexture* texture = dynamic_cast<RGTexture*>(mResources[useState.resourceIndex]);

                    gapi::TransitionState& transition = pass.transitions.emplace_back();
                    transition.resourceType = gapi::TransitionState::ResourceType::Texture;
                    transition.texture = texture->mTexture;
                    transition.subresourceRange = {
                        .firstMipLevel = texture->mMipLevel,
                        .mipLevels = 1,
                        .firstArrayIndex = 0,
                        .arraySize = texture->mTexture->GetArraySize()
                    };
                    transition.src = currentResourceStates[useState.resourceIndex];
                    transition.dst = useState.state;

                    currentResourceStates[useState.resourceIndex] = useState.state;
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
            if (RGTexture* texture = dynamic_cast<RGTexture*>(resource))
            {
                PassInfo& pass = mPasses[mCurrentPassIndex];
                pass.useStates.push_back({
                    .resourceIndex = texture->mIndex,
                    .state = gapi::ResourceStateFlag::Common
                });
            }
            else
            {
                NOT_IMPLEMENTED();
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
