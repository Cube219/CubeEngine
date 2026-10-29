#include "ResourceManager.h"

#include "AccelerationStructure.h"
#include "Allocator/FrameAllocator.h"
#include "GAPI_AccelerationStructure.h"
#include "RenderCore/RenderGraph.h"

namespace cube
{
    ResourceManager::ResourceManager(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void ResourceManager::QueuePreprocessTask(PreprocessTaskFunction&& task)
    {
        mPreprocessTasks.push_back(std::move(task));
    }

    void ResourceManager::ExecutePreprocessTasks(RGBuilder& builder)
    {
        for (PreprocessTaskFunction& task : mPreprocessTasks)
        {
            task(builder);
        }
        mPreprocessTasks.clear();
    }

    void ResourceManager::QueueBLASBuild(SharedPtr<BLASResource> blas)
    {
        Uint64 currentRenderingFrame = mRenderer.GetCurrentRenderingFrame();
        if (currentRenderingFrame == 0 || blas->GetLastBuildRenderingFrame() < currentRenderingFrame)
        {
            mBLASesToBuild.push_back(blas);
        }
    }

    void ResourceManager::ExecuteBLASBuilds(RGBuilder& builder)
    {
        if (mScratchBufferAlignment == 0)
        {
            mScratchBufferAlignment = mRenderer.GetGAPI().GetInfo().scratchBufferAlignment;
        }
        Uint64 maxScratchBufferSize = 0;

        for (SharedPtr<BLASResource>& blas : mBLASesToBuild)
        {
            maxScratchBufferSize = std::max(maxScratchBufferSize, blas->GetGAPIBLAS()->GetScratchBufferSize());
        }

        if (!mBLASesToBuild.empty() && maxScratchBufferSize > 0)
        {
            RG_GPU_TIMESTAMP_SCOPE(builder, CUBE_T("BuildBLAS"));

            // TODO: Use multiple scratch buffer to avoid UAV barrier between builds.
            RGBufferHandle scratchBuffer = builder.CreateBuffer({
                .type = gapi::BufferType::Raw,
                .size = maxScratchBufferSize,
                .stride = 4,
                .flags = gapi::BufferFlag::UAV,
                .overrideAlignment = mScratchBufferAlignment,
            }, CUBE_T("BLAS scratch buffer"));

            const Uint64 currentRenderingFrame = mRenderer.GetCurrentRenderingFrame();

            for (SharedPtr<BLASResource>& blas : mBLASesToBuild)
            {
                RGBufferHandle vertexBuffer = builder.RegisterBuffer(blas->GetVertexBufferToBuild());
                RGBufferHandle indexBuffer;
                if (SharedPtr<gapi::Buffer> gapiIndexBuffer = blas->GetIndexBufferToBuild())
                {
                    indexBuffer = builder.RegisterBuffer(gapiIndexBuffer);
                }

                builder.AddPass(Format<FrameString>(CUBE_T("{0}"), blas->GetGAPIBLAS()->GetDebugName()),
                [gapiBLAS = blas->GetGAPIBLAS(), scratchBuffer](gapi::CommandList& commandList)
                {
                    commandList.PrepareAccelerationStructureBuild();

                    commandList.UseResource(gapiBLAS, true);
                    commandList.SetResourceBarrier(gapi::ResourceBarrier{
                        .resourceType = gapi::ResourceBarrier::ResourceType::AccelerationStructure,
                        .accelerationStructure = gapiBLAS,
                        .syncSrc = gapi::ResourceSyncFlag::None,
                        .syncDst = gapi::ResourceSyncFlag::BuildAccelerationStructure,
                        .accessSrc = gapi::ResourceAccessFlag::NoAccess,
                        .accessDst = gapi::ResourceAccessFlag::WriteAccelerationStructure,
                    });
                    commandList.BuildBLAS(gapiBLAS, scratchBuffer->GetGAPIBuffer());
                },
                [vertexBuffer, indexBuffer, scratchBuffer](RGBuilder& builder)
                {
                    builder.UseResource(vertexBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                    if (indexBuffer.IsValid())
                    {
                        builder.UseResource(indexBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                    }
                    builder.UseResource(scratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                });

                blas->UpdateLastBuildRenderingFrame(currentRenderingFrame);
            }
        }

        mBLASesToBuild.clear();
    }
} // namespace cube
