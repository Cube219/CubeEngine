#pragma once

#include "../CoreHeader.h"

#include "../Handler.h"
#include "Material.h"

#include "Platform/DLib.h"
#include "RenderAPIs/RenderAPI/RenderAPI.h"

namespace cube
{
    enum class RenderAPIType
    {
        Vulkan
    };

    class CORE_EXPORT RendererManager
    {
    public:
        RendererManager() = delete;
        ~RendererManager() = delete;

        RendererManager(const RendererManager& other) = delete;
        RendererManager& operator=(const RendererManager& rhs) = delete;

        static void Initialize(RenderAPIType apiType);
        static void Shutdown();

        static HMaterial RegisterMaterial(UPtr<Material>&& material);
        static UPtr<Material> UnregisterMaterial(HMaterial& material);

        static void Render();

        static void Resize(Uint32 width, Uint32 height);

        static rapi::RenderAPI& GetRenderAPI() { return *mRenderAPI; }
        static SPtr<rapi::Sampler> GetDefaultSampler() { return mDefaultSampler; }

    private:
        friend class Material;

        template <typename T>
        static Handler<T> RegisterRenderObject(UPtr<T>&& renderObject)
        {
            mRenderObjects.push_back(std::move(renderObject));
            return mRenderObjectTable.CreateNewHandler(mRenderObjects.back().get());
        }

        template <typename T>
        static UPtr<T> UnregisterRenderObject(Handler<T>& hRenderObject)
        {
            T* pRenderObject = mRenderObjectTable.ReleaseHandler(hRenderObject);

            auto findIter = std::find_if(mRenderObjects.begin(), mRenderObjects.end(),
                [pRenderObject](const UPtr<RenderObject>& element) { return element.get() == pRenderObject; }
            );
            
            UPtr<T> uptrRenderObject(DCast(T*)((*findIter).release()));
            mRenderObjects.erase(findIter);

            return uptrRenderObject;
        }

        static SPtr<platform::DLib> mRenderAPIDLib;
        static SPtr<rapi::RenderAPI> mRenderAPI;

        static HandlerTable mRenderObjectTable;
        static Vector<UPtr<RenderObject>> mRenderObjects;

        static SPtr<rapi::Sampler> mDefaultSampler;
    };
} // namespace cube
