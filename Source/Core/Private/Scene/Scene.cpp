#include "Scene.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "GAPI_AccelerationStructure.h"
#include "SceneObject.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/RenderGraph.h"
#include "Renderer/Renderer.h"

namespace cube
{
    Scene::Scene(bool useTLAS)
    {
        GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
        mUseTLAS = useTLAS && gAPI.GetInfo().supportsHWRT;
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
    }

    void Scene::AddMaterial(SharedPtr<Material> material)
    {
        mMaterials.push_back(material);
    }

    void Scene::BuildTLAS(RGBuilder& builder, Uint32 frameIndex)
    {
        if (!mUseTLAS)
        {
            return;
        }

        FrameVector<gapi::BLASInstance> instances; // TODO: Caching?
        for (UniquePtr<SceneObject>& sceneObject : mSceneObjects)
        {
            if (SharedPtr<Mesh> mesh = sceneObject->GetMesh())
            {
                if (SharedPtr<gapi::BLAS> blas = mesh->GetBLAS())
                {
                    gapi::BLASInstance& instance = instances.emplace_back();
                    instance.blas = blas;
                    instance.transform = sceneObject->GetModelMatrix();
                    instance.hitGroupIndex = 0;
                }
            }
        }

        if (instances.empty())
        {
            mTLAS = nullptr;
            return;
        }

        bool useUpdate = true;
        if (!mTLAS || mNumInstancesInTLAS != instances.size())
        {
            GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
            mTLAS = gAPI.CreateTLAS({
                .instances = instances,
                .allowUpdate = true,
                .debugName = CUBE_T("Scene TLAS"),
            });
            mNumInstancesInTLAS = instances.size();
            useUpdate = false;
        }
        else
        {
            // TODO: Update only if instance transforms are changed.
            mTLAS->UpdateInstances(instances);
        }

        SharedPtr<gapi::Buffer>& instanceDescBuffer = mTLASInstanceDescBuffers[frameIndex % 3];
        if (!instanceDescBuffer || instanceDescBuffer->GetSize() != mTLAS->GetInstanceDescBufferSize())
        {
            GAPI& gAPI = Engine::GetRenderer()->GetGAPI();
            instanceDescBuffer = gAPI.CreateBuffer({
                .usage = gapi::ResourceUsage::CPUtoGPU,
                .bufferInfo = {
                    .type = gapi::BufferType::Raw,
                    .size = mTLAS->GetInstanceDescBufferSize(),
                },
                .debugName = Format<FrameString>(CUBE_T("TLAS instance desc buffer [{0}]"), frameIndex % 3),
            });
        }
        RGBufferHandle rgInstanceDescBuffer = builder.RegisterBuffer(instanceDescBuffer);

        RGBufferHandle rgScratchBuffer = builder.CreateBuffer({
            .type = gapi::BufferType::Raw,
            .size = mTLAS->GetScratchBufferSize(),
            .flags = gapi::BufferFlag::UAV,
        }, CUBE_T("TLAS scratch buffer"));

        builder.AddPass(CUBE_T("Build TLAS"),
        [tlas = mTLAS, rgInstanceDescBuffer, rgScratchBuffer, useUpdate](gapi::CommandList& commandList)
        {
            commandList.BuildTLAS(tlas, rgScratchBuffer->GetGAPIBuffer(), rgInstanceDescBuffer->GetGAPIBuffer(), useUpdate);
        },
        [this, rgInstanceDescBuffer, rgScratchBuffer](RGBuilder& builder)
        {
            // TODO: Add barriers for TLAS and BLASes.
            builder.UseResource(rgInstanceDescBuffer, gapi::ResourceAccessFlag::SRV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
            builder.UseResource(rgScratchBuffer, gapi::ResourceAccessFlag::UAV, gapi::ResourceSyncFlag::BuildAccelerationStructure);
        });
    }
} // namespace cube
