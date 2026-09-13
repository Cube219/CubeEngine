#include "Scene.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_AccelerationStructure.h"
#include "SceneObject.h"
#include "RenderCore/AccelerationStructure.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/RenderGraph.h"
#include "Renderer/Renderer.h"

namespace cube
{
    Scene::Scene(bool useTLAS)
    {
        mUseTLAS = useTLAS;
    }

    Scene::~Scene()
    {
        for (SharedPtr<gapi::Buffer>& instanceDescBuffer : mTLASInstanceDescBuffers)
        {
            instanceDescBuffer = nullptr;
        }

        mSceneObjects.clear();
    }

    void Scene::AddSceneObject(UniquePtr<SceneObject>&& sceneObject)
    {
        mSceneObjects.emplace_back(std::move(sceneObject));

        mChangedSceneObjects = true;
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
        UpdateTLAS(builder, renderingFrame);

        mChangedSceneObjects = false;
    }

    void Scene::UpdateTLAS(RGBuilder& builder, Uint64 renderingFrame)
    {
        Renderer* renderer = Engine::GetRenderer();
        GAPI& gAPI = renderer->GetGAPI();

        if (!mUseTLAS || !gAPI.GetInfo().supportsHWRT)
        {
            return;
        }

        // TODO: Also check scene transform after migration.
        bool dirtyTLAS = mChangedSceneObjects;
        if (!dirtyTLAS)
        {
            for (UniquePtr<SceneObject>& sceneObject : mSceneObjects)
            {
                if (sceneObject->IsRecentlyChangedTransform(renderingFrame))
                {
                    dirtyTLAS = true;
                    break;
                }

                if (SharedPtr<Mesh> mesh = sceneObject->GetMesh())
                {
                    if (SharedPtr<BLASResource> blas = mesh->GetBLAS();
                        blas && blas->GetLastBuildRenderingFrame() > mTLASLastBuildRenderingFrame)
                    {
                        dirtyTLAS = true;
                        break;
                    }
                }
            }
        }
        if (!dirtyTLAS)
        {
            return;
        }

        FrameVector<gapi::BLASInstance> instances;
        for (UniquePtr<SceneObject>& sceneObject : mSceneObjects)
        {
            if (SharedPtr<Mesh> mesh = sceneObject->GetMesh())
            {
                if (SharedPtr<BLASResource> blas = mesh->GetBLAS())
                {
                    gapi::BLASInstance& instance = instances.emplace_back();
                    instance.blas = blas->GetGAPIBLAS();
                    instance.transform = sceneObject->GetTransformMatrix() * renderer->GetGlobalTransformMatrix();
                    instance.hitGroupIndex = 0;
                }
            }
        }

        if (instances.empty())
        {
            mTLAS = nullptr;
            return;
        }

        if (!mTLAS || mNumInstancesInTLAS != instances.size())
        {
            mTLAS = gAPI.CreateTLAS(gapi::TLASCreateInfo{
                .instances = instances,
                .allowUpdate = true,
                .debugName = CUBE_T("Scene TLAS"),
            });
            mNumInstancesInTLAS = instances.size();
        }
        else
        {
            mTLAS->UpdateInstances(instances);
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
        }, CUBE_T("TLAS scratch buffer"));

        RGAccelerationStructureHandle rgTLAS = builder.RegisterAccelerationStructure(mTLAS);
        FrameVector<RGAccelerationStructureHandle> rgBLASes;
        rgBLASes.reserve(instances.size());
        for (const gapi::BLASInstance& instance : instances)
        {
            rgBLASes.push_back(builder.RegisterAccelerationStructure(instance.blas));
        }

        builder.AddPass(CUBE_T("Build TLAS"),
            [tlas = mTLAS, rgInstanceDescBuffer, rgScratchBuffer](gapi::CommandList& commandList)
        {
            commandList.PrepareAccelerationStructureBuild();
            commandList.BuildTLAS(tlas, rgScratchBuffer->GetGAPIBuffer(), rgInstanceDescBuffer->GetGAPIBuffer());
        },
            [rgTLAS, rgBLASes, rgInstanceDescBuffer, rgScratchBuffer](RGBuilder& builder)
        {
            for (RGAccelerationStructureHandle rgBLAS : rgBLASes)
            {
                builder.UseResource(rgBLAS, gapi::ResourceAccessFlag::ReadAccelerationStructure, gapi::ResourceSyncFlag::BuildAccelerationStructure);
            }
            builder.UseResource(rgTLAS, gapi::ResourceAccessFlag::WriteAccelerationStructure, gapi::ResourceSyncFlag::BuildAccelerationStructure);
            builder.UseResource(rgInstanceDescBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
            builder.UseResource(rgScratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
        });

        mTLASLastBuildRenderingFrame = renderer->GetCurrentRenderingFrame();
    }
} // namespace cube
