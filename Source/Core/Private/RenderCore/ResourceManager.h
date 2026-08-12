#pragma once

#include "CoreHeader.h"

namespace cube
{
    class RGBuilder;
    class Renderer;

    class ResourceManager
    {
    public:
        using PreprocessTaskFunction = std::function<void(RGBuilder&)>;

        ResourceManager(Renderer& renderer);

        void QueuePreprocessTask(PreprocessTaskFunction&& task);
        void ExecutePreprocessTasks(RGBuilder& builder);

    private:
        Vector<PreprocessTaskFunction> mPreprocessTasks;
    };
} // namespace cube
