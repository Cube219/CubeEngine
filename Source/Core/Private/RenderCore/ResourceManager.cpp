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

    void ResourceManager::QueueBuildingBLAS(SharedPtr<gapi::BLAS> blas)
    {
        mBLASesToBuild.push_back(blas);
    }

    void ResourceManager::ExecuteBuildingBLAS(RGBuilder& builder)
    {
        Uint64 maxScratchBufferSize = 0;

        // TODO: Keep vertex/index buffer until BLAS built.
        for (SharedPtr<gapi::BLAS>& blas : mBLASesToBuild)
        {
            maxScratchBufferSize = std::max(maxScratchBufferSize, blas->GetScratchBufferSize());
        }

        if (!mBLASesToBuild.empty() && maxScratchBufferSize > 0)
        {
            RG_GPU_TIMESTAMP_SCOPE(builder, CUBE_T("BuildingBLAS"));

            RGBufferHandle scratchBuffer = builder.CreateBuffer({
                .type = gapi::BufferType::Raw,
                .size = maxScratchBufferSize,
                .stride = 4,
                .flags = gapi::BufferFlag::UAV,
            }, CUBE_T("BLAS scratch buffer"));

            for (const SharedPtr<gapi::BLAS>& blas : mBLASesToBuild)
            {
                builder.AddPass(Format<FrameString>(CUBE_T("{0}"), blas->GetDebugName()),
                [blas, scratchBuffer](gapi::CommandList& commandList)
                {
                    commandList.BuildBLAS(blas, scratchBuffer->GetGAPIBuffer());
                },
                [scratchBuffer](RGBuilder& builder)
                {
                    builder.UseResource(scratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
                    // TODO: Add vertex / index buffer barrier.
                });
            }
        }

        mBLASesToBuild.clear();
    }
} // namespace cube
