#include "Scene.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_AccelerationStructure.h"
#include "RenderCore/AccelerationStructure.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/RenderGraph.h"
#include "Renderer/Renderer.h"
#include "SceneObject.h"

namespace cube
{
    Scene::Scene(bool useRayTracingScene)
    {
        Renderer* renderer = Engine::GetRenderer();
        GAPI& gAPI = renderer->GetGAPI();

        if (useRayTracingScene && gAPI.GetInfo().supportsHWRT)
        {
            mRayTracingScene = std::make_unique<RayTracingScene>(*this);
        }
    }

    Scene::~Scene()
    {
        mRayTracingScene = nullptr;

        mSceneObjects.clear();
    }

    void Scene::AddSceneObject(UniquePtr<SceneObject>&& sceneObject)
    {
        mSceneObjects.emplace_back(std::move(sceneObject));
    }

    void Scene::AddMaterial(SharedPtr<Material> material)
    {
        mMaterials.push_back(material);
    }

    void Scene::UpdateSceneObjects(Uint64 renderingFrame)
    {
        for (UniquePtr<SceneObject>& sceneObject : mSceneObjects)
        {
            sceneObject->UpdateDirtyTransform(renderingFrame);
        }
    }

    void Scene::UpdateScene(RGBuilder& builder, Uint64 renderingFrame)
    {
        UpdateSceneObjects(renderingFrame);
        if (mRayTracingScene)
        {
            mRayTracingScene->UpdateTLAS(builder, renderingFrame);
        }
    }

    RayTracingScene* Scene::GetRayTracingScene() const
    {
        return mRayTracingScene.get();
    }

    RayTracingScene::RayTracingScene(Scene& scene)
        : mScene(scene)
    {
    }

    RayTracingScene::~RayTracingScene()
    {
    }

    void RayTracingScene::UpdateTLAS(RGBuilder& builder, Uint64 renderingFrame)
    {
        // Always run TLAS build to emit read-only barriers for TLAS/BLAS, so RGBuilder and other pass
        // no longer has to resource tracking.
        const Vector<UniquePtr<SceneObject>>& sceneObjects = mScene.GetSceneObjects();

        Renderer* renderer = Engine::GetRenderer();
        GAPI& gAPI = renderer->GetGAPI();

        CHECK_FORMAT(!(renderer->GetResourceManager().HasBLASBuildTask()), "All BLASes should be built before updating TLAS.");

        Uint32 lastNumInstances = mBLASInstances.size();

        mBLASInstances.clear();
        FrameSet<SharedPtr<gapi::BLAS>> recentlyBuiltBLASes;
        FrameSet<SharedPtr<gapi::BLAS>> otherBLASes;
        for (const UniquePtr<SceneObject>& sceneObject : sceneObjects)
        {
            if (SharedPtr<Mesh> mesh = sceneObject->GetMesh())
            {
                if (SharedPtr<BLASResource> blas = mesh->GetBLAS())
                {
                    gapi::BLASInstance& instance = mBLASInstances.emplace_back();
                    instance.blas = blas->GetGAPIBLAS();
                    instance.transform = sceneObject->GetTransformMatrix() * renderer->GetGlobalTransformMatrix();
                    instance.hitGroupIndex = 0;

                    if (blas->GetLastBuildRenderingFrame() >= renderingFrame)
                    {
                        recentlyBuiltBLASes.insert(blas->GetGAPIBLAS());
                    }
                    else
                    {
                        otherBLASes.insert(blas->GetGAPIBLAS());
                    }
                }
            }
        }

        if (mBLASInstances.empty())
        {
            mTLAS = nullptr;
            return;
        }

        if (!mTLAS || mBLASInstances.size() != lastNumInstances)
        {
            mTLAS = gAPI.CreateTLAS(gapi::TLASCreateInfo{
                .instances = mBLASInstances,
                .debugName = CUBE_T("RayTracingScene TLAS"),
            });
        }
        else
        {
            mTLAS->UpdateInstances(mBLASInstances);
        }

        SharedPtr<gapi::Buffer>& instanceDescBuffer = mTLASInstanceDescBuffers[renderingFrame % 3];
        if (!instanceDescBuffer || instanceDescBuffer->GetSize() != mTLAS->GetInstanceDescBufferSize())
        {
            instanceDescBuffer = gAPI.CreateBuffer(gapi::BufferCreateInfo{
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .bufferInfo = {
                    .type = gapi::BufferType::Raw,
                    .size = mTLAS->GetInstanceDescBufferSize(),
                },
                .debugName = Format<FrameString>(CUBE_T("TLAS instance desc buffer [{0}]"), renderingFrame % 3),
            });
        }
        RGBufferHandle rgInstanceDescBuffer = builder.RegisterBuffer(instanceDescBuffer);

        RGBufferHandle rgScratchBuffer = builder.CreateBuffer(gapi::BufferInfo{
            .type = gapi::BufferType::Raw,
            .size = mTLAS->GetScratchBufferSize(),
            .flags = gapi::BufferFlag::UAV,
            .overrideAlignment = gAPI.GetInfo().scratchBufferAlignment,
        },
        CUBE_T("TLAS scratch buffer"));

        builder.AddPass(CUBE_T("Build TLAS"), [tlas = mTLAS, rgInstanceDescBuffer, rgScratchBuffer, recentlyBuiltBLASes, otherBLASes](gapi::CommandList& commandList)
        {
            commandList.PrepareAccelerationStructureBuild();

            FrameVector<gapi::ResourceBarrier> beforeBarriers;
            gapi::ResourceBarrier beforeBarrier = gapi::ResourceBarrier{
                .resourceType = gapi::ResourceBarrier::ResourceType::AccelerationStructure,
                .syncSrc = gapi::ResourceSyncFlag::None,
                .syncDst = gapi::ResourceSyncFlag::BuildAccelerationStructure,
                .accessSrc = gapi::ResourceAccessFlag::NoAccess,
                .accessDst = gapi::ResourceAccessFlag::ReadAccelerationStructure,
            };
            for (const SharedPtr<gapi::BLAS>& recentlyBuiltBLAS : recentlyBuiltBLASes)
            {
                beforeBarrier.accelerationStructure = recentlyBuiltBLAS;
                beforeBarrier.syncSrc = gapi::ResourceSyncFlag::BuildAccelerationStructure;
                beforeBarrier.accessSrc = gapi::ResourceAccessFlag::WriteAccelerationStructure;
                beforeBarriers.push_back(beforeBarrier);

                commandList.UseResource(recentlyBuiltBLAS, false);
            }
            for (const SharedPtr<gapi::BLAS>& blas : otherBLASes)
            {
                beforeBarrier.accelerationStructure = blas;
                beforeBarrier.syncSrc = gapi::ResourceSyncFlag::None;
                beforeBarrier.accessSrc = gapi::ResourceAccessFlag::NoAccess;
                beforeBarriers.push_back(beforeBarrier);

                commandList.UseResource(blas, false);
            }
            beforeBarrier.accelerationStructure = tlas;
            beforeBarrier.syncSrc = gapi::ResourceSyncFlag::None;
            beforeBarrier.accessSrc = gapi::ResourceAccessFlag::NoAccess;
            beforeBarrier.accessDst = gapi::ResourceAccessFlag::WriteAccelerationStructure;
            beforeBarriers.push_back(beforeBarrier);
            commandList.SetResourceBarrier(beforeBarriers);
            commandList.UseResource(tlas, true);

            commandList.BuildTLAS(tlas, rgScratchBuffer->GetGAPIBuffer(), rgInstanceDescBuffer->GetGAPIBuffer());

            // Set read access barrier covering all sync types, so no further barriers are needed after this pass.
            FrameVector<gapi::ResourceBarrier> afterBarriers;
            gapi::ResourceBarrier afterBarrier = gapi::ResourceBarrier{
                .resourceType = gapi::ResourceBarrier::ResourceType::AccelerationStructure,
                .syncSrc = gapi::ResourceSyncFlag::BuildAccelerationStructure,
                .syncDst = gapi::ResourceSyncFlag::All,
                .accessSrc = gapi::ResourceAccessFlag::ReadAccelerationStructure,
                .accessDst = gapi::ResourceAccessFlag::ReadAccelerationStructure,
            };
            for (const SharedPtr<gapi::BLAS>& blas : recentlyBuiltBLASes)
            {
                afterBarrier.accelerationStructure = blas;
                afterBarriers.push_back(afterBarrier);
            }
            for (const SharedPtr<gapi::BLAS>& blas : otherBLASes)
            {
                afterBarrier.accelerationStructure = blas;
                afterBarriers.push_back(afterBarrier);
            }
            afterBarrier.accelerationStructure = tlas;
            afterBarrier.accessSrc = gapi::ResourceAccessFlag::WriteAccelerationStructure;
            afterBarriers.push_back(afterBarrier);
            commandList.SetResourceBarrier(afterBarriers);
        }, [rgInstanceDescBuffer, rgScratchBuffer](RGBuilder& builder)
        {
            builder.UseResource(rgInstanceDescBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
            builder.UseResource(rgScratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
        });
    }
} // namespace cube
