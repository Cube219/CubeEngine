#pragma once

#include "CoreHeader.h"

namespace cube
{
    namespace gapi
    {
        class BLAS;
    } // namespace gapi

    class RGBuilder;
    class Renderer;

    class ResourceManager
    {
    public:
        using PreprocessTaskFunction = std::function<void(RGBuilder&)>;

        ResourceManager(Renderer& renderer);

        void QueuePreprocessTask(PreprocessTaskFunction&& task);
        void ExecutePreprocessTasks(RGBuilder& builder);

        void QueueBuildingBLAS(SharedPtr<gapi::BLAS> blas);
        void ExecuteBuildingBLAS(RGBuilder& builder);

    private:
        Vector<PreprocessTaskFunction> mPreprocessTasks;
        Vector<SharedPtr<gapi::BLAS>> mBLASesToBuild;
    };
} // namespace cube
