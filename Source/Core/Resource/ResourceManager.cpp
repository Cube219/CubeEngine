#include "ResourceManager.h"

#include "ResourceImporter.h"
#include "Platform/FileSystem.h"
#include "../Assertion.h"

namespace cube
{
    Vector<UPtr<ResourceImporter>> ResourceManager::mImporters;

    Mutex ResourceManager::mLoadedResourcesMutex;
    HashMap<String, Resource*> ResourceManager::mLoadedResources;

    void ResourceManager::Initialize()
    {
    }

    void ResourceManager::Shutdown()
    {
        UnloadUnusedResources();

        for(auto& r : mLoadedResources) {
            delete r.second;
        }
        mLoadedResources.clear();
        mImporters.clear();
    }

    void ResourceManager::RegisterImporter(UPtr<ResourceImporter> importer)
    {
        mImporters.push_back(std::move(importer));
    }

    RPtr<Resource> ResourceManager::LoadResource(StringView path)
    {
        using namespace platform;

        // Check if the resource is already loaded
        {
            Lock lock(mLoadedResourcesMutex);

            auto findIter = mLoadedResources.find(path.data()); // TODO: UUID로 바꾸기
            if(findIter != mLoadedResources.end()) {
                return RPtr<Resource>(findIter->second);
            }
        }

        // Get a metadata
        FrameString metaPath(path.data(), path.size());
        metaPath.append(CUBE_T(".cmeta"));
        SPtr<File> metaFile = FileSystem::OpenFile(metaPath, FileAccessModeFlag::Read);

        Uint64 size = metaFile->GetFileSize();
        char* metaString = (char*)GetFrameAllocator().Allocate(size);

        metaFile->Read(metaString, size);
        metaString[size] = '\0';

        FrameJson metaJson;
        metaJson.Parse(metaString);
        GetFrameAllocator().Free(metaString);

        FrameU8String importerNameU8 = metaJson["importer_name"].GetString();
        FrameString importerName;
        String_ConvertAndAppend(importerName, importerNameU8);

        Resource* loadedResource = nullptr;
        // Find importer to import the resource
        bool isFindImporter = false;
        for(auto& importer : mImporters) {
            if(importer->GetName() == importerName) {
                SPtr<File> resourceFile = FileSystem::OpenFile(path, FileAccessModeFlag::Read);
                const FrameJsonValue& info = metaJson["Info"];

                loadedResource = importer->Import(resourceFile, info);
                isFindImporter = true;
                break;
            }
        }

        CHECK(isFindImporter == true, "Cannot find the importer '{0}'.", importerName);
        CHECK(loadedResource != nullptr, "Failed to load the resource.");

        {
            Lock lock(mLoadedResourcesMutex);

            mLoadedResources[path.data()] = loadedResource; // TODO: UUID로 바꾸기
        }

        return RPtr<Resource>(loadedResource);
    }

    void ResourceManager::ReleaseResource(const RPtr<Resource>& resource)
    {
        Lock lock(mLoadedResourcesMutex);
    }

    void ResourceManager::UnloadUnusedResources()
    {
        Lock lock(mLoadedResourcesMutex);

        Vector<String> resourcesToUnload;
        for(auto& r : mLoadedResources) {
            if(r.second->mRefCount == 0) {
                resourcesToUnload.push_back(r.first);
            }
        }

        for(auto& r : resourcesToUnload) {
            delete mLoadedResources[r];
            mLoadedResources.erase(r);
        }
    }
} // namespace cube
