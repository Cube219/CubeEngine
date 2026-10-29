#pragma once

#include "EngineHeader.h"

namespace cube
{
    class BLASResource;
    class RGBuilder;
    class Renderer;

    class ResourceManager
    {
    public:
        using PreprocessTaskFunction = std::function<void(RGBuilder&)>;

        ResourceManager(Renderer& renderer);

        void QueuePreprocessTask(PreprocessTaskFunction&& task);
        void ExecutePreprocessTasks(RGBuilder& builder);

        void QueueBLASBuild(SharedPtr<BLASResource> blas);
        void ExecuteBLASBuilds(RGBuilder& builder);

        bool HasBLASBuildTask() const { return !mBLASesToBuild.empty(); }

    private:
        Renderer& mRenderer;

        Vector<PreprocessTaskFunction> mPreprocessTasks;
        Vector<SharedPtr<BLASResource>> mBLASesToBuild;
        Uint32 mScratchBufferAlignment = 0;
    };
} // namespace cube
