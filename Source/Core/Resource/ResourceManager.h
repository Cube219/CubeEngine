#pragma once

#include "../CoreHeader.h"

#include "../Thread/MutexLock.h"
#include "Resource.h"

namespace cube
{
    class CORE_EXPORT ResourceManager
    {
    public:
        ResourceManager() = delete;
        ~ResourceManager() = delete;
        ResourceManager(const ResourceManager& other) = delete;
        ResourceManager& operator=(const ResourceManager& rhs) = delete;

        static void Initialize();
        static void Shutdown();

        static void RegisterImporter(UPtr<ResourceImporter> importer);

        static RPtr<Resource> LoadResource(StringView path);
        template <typename T>
        static RPtr<T> LoadResource(StringView path)
        {
            return LoadResource(path).Cast<T>();
        }

        static void ReleaseResource(const RPtr<Resource>& resource);
        static void UnloadUnusedResources();

    private:
        friend class BaseMeshGenerator;

        static Vector<UPtr<ResourceImporter>> mImporters;

        static Mutex mLoadedResourcesMutex;
        static HashMap<String, Resource*> mLoadedResources;
    };
} // namespace cube
