#include "RenderGraph.h"

#include "Checker.h"
#include "GAPI_CommandList.h"

namespace cube
{
    void RGBuilder::BeginRenderPass(ArrayView<gapi::ColorAttachment> colors, gapi::DepthStencilAttachment depthStencil)
    {
        CHECK(!mIsInRenderPass);

        AddPass(CUBE_T("##BeginRenderPass"), [colorsArr = Vector<gapi::ColorAttachment>(colors.begin(), colors.end()), depthStencil](gapi::CommandList& commandList)
        {
            commandList.BeginRenderPass(colorsArr, depthStencil);
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

    void RGBuilder::AddPass(StringView name, PassFunction&& passFunction)
    {
        CHECK(!mIsExecuting);

        mPasses.emplace_back(String(name), std::move(passFunction));
    }

    void RGBuilder::ExecuteAndSubmit(gapi::CommandList& commandList)
    {
        CHECK(!mIsInRenderPass);

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

            pass.passFunction(commandList);

            if (addGPUEvent)
            {
                commandList.EndEvent();
            }
        }

        commandList.InsertTimestamp(CUBE_T("End RGBuilder"));

        commandList.End();
        commandList.Submit();

        mPasses.clear();

        mIsExecuting = false;
    }
} // namespace cube
