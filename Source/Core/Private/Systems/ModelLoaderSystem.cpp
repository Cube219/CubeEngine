#include "ModelLoaderSystem.h"

#include "imgui.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "FileSystem.h"
#include "ModelLoaderSubsystem_glTF.h"
#include "ModelLoaderSubsystem_obj.h"
#include "PathHelper.h"
#include "Renderer/Renderer.h"

namespace cube
{
    Map<ModelType, UniquePtr<ModelLoaderSubsystem>> ModelLoaderSystem::mSubsystems;

    Vector<ModelPathInfo> ModelLoaderSystem::mModelPathInfoList;
    int ModelLoaderSystem::mCurrentSelectModelIndex;

    float ModelLoaderSystem::mModelScale;

    void ModelLoaderSystem::Initialize()
    {
        mSubsystems.insert({ ModelType::glTF, std::make_unique<ModelLoaderSubsystem_glTF>() });
        mSubsystems.insert({ ModelType::obj, std::make_unique<ModelLoaderSubsystem_obj>() });

        mCurrentSelectModelIndex = -1;
        ResetModelScale();
    }

    void ModelLoaderSystem::Shutdown()
    {
        mSubsystems.clear();
    }

    void ModelLoaderSystem::OnLoopImGUI()
    {
        ImGui::Begin("Model Loader", 0, ImGuiWindowFlags_AlwaysAutoResize);

        const char* modelSelectPreview = mCurrentSelectModelIndex >= 0 ? mModelPathInfoList[mCurrentSelectModelIndex].name.c_str() : "";
        static bool modelDropdownExpandedLastFrame = false;
        if (ImGui::BeginCombo("Models", modelSelectPreview))
        {
            if (!modelDropdownExpandedLastFrame)
            {
                mModelPathInfoList.clear();
                UpdateModelPathList();
            }
            modelDropdownExpandedLastFrame = true;
            if (mCurrentSelectModelIndex > mModelPathInfoList.size())
            {
                mCurrentSelectModelIndex = -1;
            }

            ModelPathType currentPathType = (ModelPathType)(-1);
            for (int i = 0; i < mModelPathInfoList.size(); ++i)
            {
                const ModelPathInfo& info = mModelPathInfoList[i];
                if (info.pathType != currentPathType)
                {
                    switch (info.pathType)
                    {
                    case ModelPathType::glTFSampleAsset:
                        ImGui::SeparatorText("glTF Sample Asset");
                        break;
                    case ModelPathType::DefaultModels:
                        ImGui::SeparatorText("DefaultModels");
                        break;
                    }

                    currentPathType = info.pathType;
                }

                if (ImGui::Selectable(info.name.c_str(), i == mCurrentSelectModelIndex))
                {
                    mCurrentSelectModelIndex = i;
                    LoadCurrentModelAndSet();
                }

                if (i == mCurrentSelectModelIndex)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::PushItemWidth(60.0f);
        const float lastModelScale = mModelScale;
        ImGui::DragFloat("Model Scale", &mModelScale, 0.1f);
        ImGui::PopItemWidth();
        if (lastModelScale != mModelScale)
        {
            UpdateModelMatrix();
        }

        ImGui::NewLine();
        if (ImGui::Button("Reset"))
        {
            ResetModelScale();
        }

        ImGui::End();
    }

    ModelResources ModelLoaderSystem::LoadModel(const ModelPathInfo& pathInfo)
    {
        return mSubsystems[pathInfo.modelType]->LoadModel(pathInfo);
    }

    void ModelLoaderSystem::UpdateModelPathList()
    {
        mModelPathInfoList.clear();

        // glTF Sample Assets
        {
            FrameString resourceBasePath = Format<FrameString>(CUBE_T("{0}/Resources/Models/glTFSampleAssets/Models/"), Engine::GetRootDirectoryPath());
            static const Character* gltfLoadModels[] = {
                CUBE_T("DamagedHelmet"),
                CUBE_T("FlightHelmet"),
                CUBE_T("MetalRoughSpheres"),
                CUBE_T("MetalRoughSpheresNoTextures"),
                CUBE_T("Sponza"),
                CUBE_T("Suzanne"),
            };
            Vector<String> list = platform::FileSystem::GetList(resourceBasePath);
            for (const String& e : list)
            {
                bool exist = false;
                for (const Character* modelName : gltfLoadModels)
                {
                    if (e == modelName)
                    {
                        exist = true;
                        break;
                    }
                }

                if (exist)
                {
                    mModelPathInfoList.push_back({
                        .pathType = ModelPathType::glTFSampleAsset,
                        .modelType = ModelType::glTF,
                        .name = String_Convert<AnsiString>(e),
                        .pathList = { Format<String>(CUBE_T("{0}/{1}/glTF/{1}.gltf"), resourceBasePath, e) }
                    });
                }
            }
        }

        // DefaultModels
        {
            FrameString resourceBasePath = Format<FrameString>(CUBE_T("{0}/Resources/Models/DefaultModels/"), Engine::GetRootDirectoryPath());
            static const Character* loadModels[] = {
                CUBE_T("CornellBox"),
                CUBE_T("FireplaceRoom"),
                CUBE_T("LivingRoom"),
                CUBE_T("StanfordBunny"),
            };
            Vector<String> list = platform::FileSystem::GetList(resourceBasePath);
            for (const String& e : list)
            {
                bool exist = false;
                for (const Character* modelName : loadModels)
                {
                    if (e == modelName)
                    {
                        exist = true;
                        break;
                    }
                }

                if (exist)
                {
                    ModelPathInfo info = {
                        .pathType = ModelPathType::DefaultModels,
                        .modelType = ModelType::obj,
                        .name = String_Convert<AnsiString>(e),
                    };

                    FrameString modelPath = Format<FrameString>(CUBE_T("{0}/{1}"), resourceBasePath, e);
                    Vector<String> fileNamesInModelPath = platform::FileSystem::GetList(modelPath);
                    for (const String& fileName : fileNamesInModelPath)
                    {
                        StringView ext = PathHelper::GetExtension(fileName);
                        if (ext == CUBE_T("obj"))
                        {
                            info.pathList.push_back(Format<String>(CUBE_T("{0}/{1}"), modelPath, fileName));
                        }
                    }
                    CHECK(info.pathList.size() > 0);

                    mModelPathInfoList.push_back(std::move(info));
                }
            }
        }
    }

    void ModelLoaderSystem::LoadCurrentModelAndSet()
    {
        const ModelPathInfo& info = mModelPathInfoList[mCurrentSelectModelIndex];

        ModelResources resources = LoadModel(info);
        Engine::SetMesh(resources.mesh);
        Engine::SetMaterials(resources.materials);
    }

    void ModelLoaderSystem::UpdateModelMatrix()
    {
        Engine::GetRenderer()->SetObjectModelMatrix(Vector3::Zero(), Vector3::Zero(), Vector3(mModelScale, mModelScale, mModelScale));
    }

    void ModelLoaderSystem::ResetModelScale()
    {
        mModelScale = 1.0f;
        UpdateModelMatrix();
    }
} // namespace cube
