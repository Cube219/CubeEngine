#include "ModelLoaderSystem.h"

#include "imgui.h"

#include "Checker.h"
#include "CubeMath.h"
#include "Engine.h"
#include "Logger.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/MeshHelper.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"
#include "Systems/ModelLoader/GLTFModelLoader.h"
#include "Systems/ModelLoader/ObjModelLoader.h"

namespace cube
{
    Vector<UniquePtr<IModelLoader>> ModelLoaderSystem::mLoaders;

    Vector<ModelPathInfo> ModelLoaderSystem::mModelPathList;
    int ModelLoaderSystem::mCurrentSelectModelIndex;

    Float3 ModelLoaderSystem::mModelPosition;
    Float3 ModelLoaderSystem::mModelRotation;
    float ModelLoaderSystem::mModelScale;
    bool ModelLoaderSystem::mUseFloat16Vertices = true;

    void ModelLoaderSystem::Initialize()
    {
        // Register model loaders. Order determines the model list grouping order.
        mLoaders.push_back(std::make_unique<GLTFModelLoader>());
        mLoaders.push_back(std::make_unique<ObjModelLoader>());

        mCurrentSelectModelIndex = -1;
        ResetModelTransform();

        // Load model at initialization using parameter.
        if (AnsiStringView modelParam = Engine::GetCommandLineParam("model"); !modelParam.empty())
        {
            // Parse format: <type>_<index> (e.g., gltf_0, default_2)
            SizeType underscorePos = modelParam.rfind('_');
            if (underscorePos == AnsiStringView::npos || underscorePos == 0 || underscorePos == modelParam.size() - 1)
            {
                CUBE_LOG(Error, ModelLoaderSystem, "Invalid --model format ({0}). Expected <type>_<index>.", modelParam);
                return;
            }

            AnsiStringView modelTypeStr = modelParam.substr(0, underscorePos);
            AnsiStringView indexStr = modelParam.substr(underscorePos + 1);

            int index = -1;
            try
            {
                index = std::stoi(indexStr.data());
            }
            catch (...)
            {
                CUBE_LOG(Error, ModelLoaderSystem, "Invalid --model index ({0}). Must be an integer.", indexStr);
                return;
            }

            ModelType type;
            if (modelTypeStr == "gltf")
            {
                type = ModelType::glTF;
            }
            else if (modelTypeStr == "default")
            {
                type = ModelType::Obj;
            }
            else
            {
                CUBE_LOG(Error, ModelLoaderSystem, "Invalid --model type ({0}). Must be 'gltf' or 'default'.", modelTypeStr);
                return;
            }

            LoadModelList();

            int typeCount = 0;
            for (int i = 0; i < static_cast<int>(mModelPathList.size()); ++i)
            {
                if (mModelPathList[i].type == type)
                {
                    if (typeCount == index)
                    {
                        mCurrentSelectModelIndex = i;
                        CUBE_LOG(Info, ModelLoaderSystem, "Loading model from command line: {0}", mModelPathList[i].name);
                        LoadCurrentModelAndSet();
                        return;
                    }
                    ++typeCount;
                }
            }

            CUBE_LOG(Error, ModelLoaderSystem, "Model index {0} out of range for type ({1}) (size: {2}).", index, modelTypeStr, typeCount);
        }
    }

    void ModelLoaderSystem::Shutdown()
    {
        mLoaders.clear();
    }

    void ModelLoaderSystem::OnLoopImGUIContent()
    {
        const char* modelSelectPreview = mCurrentSelectModelIndex >= 0 ? mModelPathList[mCurrentSelectModelIndex].name.c_str() : "";
        static bool modelDropdownExpandedLastFrame = false;
        if (ImGui::BeginCombo("Models", modelSelectPreview))
        {
            if (!modelDropdownExpandedLastFrame)
            {
                mModelPathList.clear();
                LoadModelList();
            }
            modelDropdownExpandedLastFrame = true;
            if (mCurrentSelectModelIndex > mModelPathList.size())
            {
                mCurrentSelectModelIndex = -1;
            }

            ModelType currentType = (ModelType)(-1);
            for (int i = 0; i < mModelPathList.size(); ++i)
            {
                const ModelPathInfo& info = mModelPathList[i];
                if (info.type != currentType)
                {
                    switch (info.type)
                    {
                    case ModelType::glTF:
                        ImGui::SeparatorText("glTF Sample Asset");
                        break;
                    case ModelType::Obj:
                        ImGui::SeparatorText("Obj (DefaultModels)");
                        break;
                    }

                    currentType = info.type;
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
        else
        {
            modelDropdownExpandedLastFrame = false;
        }

        ImGui::Separator();

        ImGui::PushItemWidth(200.0f);
        if (ImGui::DragFloat3("Position", &mModelPosition.x, 0.1f))
        {
            UpdateModelMatrix();
        }
        if (ImGui::DragFloat3("Rotation", &mModelRotation.x, 1.0f))
        {
            UpdateModelMatrix();
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(65.0f);
        if (ImGui::DragFloat("Scale", &mModelScale, 0.1f))
        {
            UpdateModelMatrix();
        }
        ImGui::PopItemWidth();

        if (ImGui::Button("Reset"))
        {
            ResetModelTransform();
        }

        ImGui::Separator();

        if (ImGui::Checkbox("Float16 Vertices", &mUseFloat16Vertices))
        {
            LoadCurrentModelAndSet(false);
        }
    }

    SharedPtr<Scene> ModelLoaderSystem::LoadModel(const ModelPathInfo& pathInfo)
    {
        for (const UniquePtr<IModelLoader>& loader : mLoaders)
        {
            if (loader->GetModelType() == pathInfo.type)
            {
                return loader->LoadModel(pathInfo, GetMeshMetadata());
            }
        }

        NOT_IMPLEMENTED();
        return nullptr;
    }

    void ModelLoaderSystem::LoadModelList()
    {
        mModelPathList.clear();

        for (const UniquePtr<IModelLoader>& loader : mLoaders)
        {
            const Vector<ModelPathInfo>& list = loader->GetModelList();
            mModelPathList.insert(mModelPathList.end(), list.begin(), list.end());
        }
    }

    void ModelLoaderSystem::LoadCurrentModelAndSet(bool resetTransform)
    {
        if (resetTransform)
        {
            ResetModelTransform();
        }

        if (mCurrentSelectModelIndex != -1)
        {
            const ModelPathInfo& info = mModelPathList[mCurrentSelectModelIndex];

            Engine::SetScene(LoadModel(info));
        }
        else
        {
            // Create default scene.
            SharedPtr<Mesh> boxMesh = std::make_shared<Mesh>(MeshHelper::GenerateBoxMeshData(), GetMeshMetadata());
            UniquePtr<SceneObject> obj = std::make_unique<SceneObject>(CUBE_T("DefaultBox"), boxMesh);

            SharedPtr<Scene> scene = std::make_shared<Scene>();
            scene->AddSceneObject(std::move(obj));

            Engine::SetScene(scene);
        }
    }

    MeshMetadata ModelLoaderSystem::GetMeshMetadata()
    {
        MeshMetadata meshMeta;
        meshMeta.useFloat16 = mUseFloat16Vertices;

        return meshMeta;
    }

    void ModelLoaderSystem::UpdateModelMatrix()
    {
        Vector3 position(mModelPosition.x, mModelPosition.y, mModelPosition.z);
        Vector3 rotation(
            Math::Deg2Rad(mModelRotation.x),
            Math::Deg2Rad(mModelRotation.y),
            Math::Deg2Rad(mModelRotation.z));
        Vector3 scale(mModelScale, mModelScale, mModelScale);
        Engine::GetRenderer()->SetObjectModelMatrix(position, rotation, scale);
    }

    void ModelLoaderSystem::ResetModelTransform()
    {
        if (mCurrentSelectModelIndex != -1)
        {
            const ModelPathInfo& info = mModelPathList[mCurrentSelectModelIndex];

            mModelPosition = info.position.GetFloat3();
            mModelRotation = info.rotation.GetFloat3();
            mModelScale = info.scale.GetFloat3().x;
        }
        else
        {
            mModelPosition = { 0.0f, 0.0f, 0.0f };
            mModelRotation = { 0.0f, 0.0f, 0.0f };
            mModelScale = 1.0f;
        }
        UpdateModelMatrix();
    }
} // namespace cube
