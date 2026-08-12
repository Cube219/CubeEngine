#include "ResourceManager.h"

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
} // namespace cube
