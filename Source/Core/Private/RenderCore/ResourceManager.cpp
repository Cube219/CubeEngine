#include "ResourceManager.h"

#include "Allocator/FrameAllocator.h"
#include "GAPI_AccelerationStructure.h"
#include "RenderCore/RenderGraph.h"

namespace cube
{
    ResourceManager::ResourceManager(Renderer& renderer)
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

    void ResourceManager::QueueBLASBuild(SharedPtr<gapi::BLAS> blas)
    {
        mBLASesToBuild.push_back(blas);
    }

    void ResourceManager::ExecuteBLASBuilds(RGBuilder& builder)
    {
        Uint64 maxScratchBufferSize = 0;

        for (SharedPtr<gapi::BLAS>& blas : mBLASesToBuild)
        {
            maxScratchBufferSize = std::max(maxScratchBufferSize, blas->GetScratchBufferSize());
        }

        if (!mBLASesToBuild.empty() && maxScratchBufferSize > 0)
        {
            RG_GPU_TIMESTAMP_SCOPE(builder, CUBE_T("BuildingBLAS"));

            // TODO: Use multiple scratch buffer to avoid UAV barrier between builds.
            RGBufferHandle scratchBuffer = builder.CreateBuffer({
                .type = gapi::BufferType::Raw,
                .size = maxScratchBufferSize,
                .stride = 4,
                .flags = gapi::BufferFlag::UAV,
            }, CUBE_T("BLAS scratch buffer"));

            for (const SharedPtr<gapi::BLAS>& blas : mBLASesToBuild)
            {
                RGBufferHandle vertexBuffer = builder.RegisterBuffer(blas->GetVertexBufferToBuild());
                RGBufferHandle indexBuffer;
                if (SharedPtr<gapi::Buffer> gapiIndexBuffer = blas->GetIndexBufferToBuild())
                {
                    indexBuffer = builder.RegisterBuffer(gapiIndexBuffer);
                }

                builder.AddPass(Format<FrameString>(CUBE_T("{0}"), blas->GetDebugName()),
                [blas, scratchBuffer](gapi::CommandList& commandList)
                {
                    commandList.BuildBLAS(blas, scratchBuffer->GetGAPIBuffer());
                },
                [vertexBuffer, indexBuffer, scratchBuffer](RGBuilder& builder)
                {
                    // TODO: Add barriers for BLAS.
                    builder.UseResource(vertexBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                    if (indexBuffer.IsValid())
                    {
                        builder.UseResource(indexBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                    }
                    builder.UseResource(scratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                });
            }
        }

        mBLASesToBuild.clear();
    }
} // namespace cube
