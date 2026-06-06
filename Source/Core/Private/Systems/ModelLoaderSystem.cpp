#include "ModelLoaderSystem.h"

#include "imgui.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "CubeMath.h"
#include "CubeString.h"
#include "Engine.h"
#include "Logger.h"
#include "FileSystem.h"
#include "GAPI_Texture.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/MeshHelper.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

namespace cube
{
    Vector<ModelPathInfo> ModelLoaderSystem::mModelPathList;
    int ModelLoaderSystem::mCurrentSelectModelIndex;

    Float3 ModelLoaderSystem::mModelPosition;
    Float3 ModelLoaderSystem::mModelRotation;
    float ModelLoaderSystem::mModelScale;
    bool ModelLoaderSystem::mUseFloat16Vertices = true;

    void ModelLoaderSystem::Initialize()
    {
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
        switch (pathInfo.type)
        {
        case ModelType::glTF:
            return LoadModel_glTF(pathInfo);
        case ModelType::Obj:
            return LoadModel_Obj(pathInfo);
        default:
            NOT_IMPLEMENTED();
        }

        return nullptr;
    }

    void ModelLoaderSystem::LoadModelList()
    {
        mModelPathList.clear();

        struct ModelLoadInfo
        {
            const Character* name;
            Vector3 position = Vector3::Zero();
            Vector3 rotation = Vector3::Zero();
            Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
        };

        // glTF models.
        platform::FilePath resourceBasePath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Models/glTFSampleAssets/Models");
        static const ModelLoadInfo gltfLoadModelInfos[] = {
            {
                .name = CUBE_T("DamagedHelmet"),
                .rotation = Vector3(0.0f, -90.0f, 90.0f),
                .scale = Vector3(2.5f)
            },
            {
                .name = CUBE_T("FlightHelmet"),
                .rotation = Vector3(0, -90.0f, 0.0f),
                .scale = Vector3(7.0f)
            },
            {
                .name = CUBE_T("MetalRoughSpheres"),
                .position = Vector3(-1.5f, 0.0f, 0.0f),
                .rotation = Vector3(0, 90.0f, 90.0f),
                .scale = Vector3(0.5f)
            },
            {
                .name = CUBE_T("Sponza"),
            },
            {
                .name = CUBE_T("Suzanne"),
                .rotation = Vector3(0, -90.0f),
                .scale = Vector3(2.0f)
            },
            {
                .name = CUBE_T("AlphaBlendModeTest"),
            },
            {
                .name = CUBE_T("CompareAmbientOcclusion"),
            },
        };
        Vector<String> gltfList = platform::FileSystem::GetList(resourceBasePath);
        for (const ModelLoadInfo& modelInfo : gltfLoadModelInfos)
        {
            for (const String& e : gltfList)
            {
                if (e == modelInfo.name)
                {
                    mModelPathList.push_back({
                        .type = ModelType::glTF,
                        .name = String_Convert<AnsiString>(e),
                        .path = resourceBasePath / Format<FrameString>(CUBE_T("{0}/glTF/{0}.gltf"), e),
                        .position = modelInfo.position,
                        .rotation = modelInfo.rotation,
                        .scale = modelInfo.scale
                    });
                    break;
                }
            }
        }

        // obj models.
        platform::FilePath objBasePath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Models/DefaultModels");
        static const ModelLoadInfo objLoadModels[] = {
            {
                .name = CUBE_T("CornellBox"),
                .rotation = Vector3(0.0f, -90.0f, 0.0f),
                .scale = Vector3(2.0f)
            },
            {
                .name = CUBE_T("FireplaceRoom"),
                .position = Vector3(-5.8f, 0.0f, 5.1f),
                .scale = Vector3(3.0f)
            },
            {
                .name = CUBE_T("LivingRoom"),
                .position = Vector3(0.0f, 0.0f, -12.0f),
                .scale = Vector3(3.0f)
            }
        };
        Vector<String> objList = platform::FileSystem::GetList(objBasePath);
        for (const ModelLoadInfo& modelInfo : objLoadModels)
        {
            for (const String& e : objList)
            {
                if (e == modelInfo.name)
                {
                    mModelPathList.push_back({
                        .type = ModelType::Obj,
                        .name = String_Convert<AnsiString>(e),
                        .path = objBasePath / e,
                        .position = modelInfo.position,
                        .rotation = modelInfo.rotation,
                        .scale = modelInfo.scale
                    });
                    break;
                }
            }
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

    SharedPtr<Scene> ModelLoaderSystem::LoadModel_glTF(const ModelPathInfo& pathInfo)
    {
        tinygltf::Model model;
        AnsiString error;
        AnsiString warning;
        tinygltf::TinyGLTF loader;

        AnsiString pathStr = pathInfo.path.ToAnsiString();
        bool res = loader.LoadASCIIFromFile(&model, &error, &warning, pathStr);

        if (!warning.empty())
        {
            CUBE_LOG(Warning, ModelLoaderSystem, "There's some warning while loading from glTF: {}", warning);
        }

        if (!error.empty())
        {
            CUBE_LOG(Error, ModelLoaderSystem, "There's some error while loading from glTF: {}", error);
        }

        if (!res)
        {
            CUBE_LOG(Error, ModelLoaderSystem, "Failed to load the model from glTF");
            return {};
        }

        FrameString modelName = String_Convert<FrameString>(pathInfo.name);

        // Load materials.
        Vector<SharedPtr<Material>> materials;
        HashMap<int, SharedPtr<TextureResource>> loadedImageCache;

        for (const tinygltf::Material& gltfMaterial : model.materials)
        {
            auto LoadTexture = [&model, &loadedImageCache](StringView materialName, const Character* textureName, int textureIndex) -> SharedPtr<TextureResource>
            {
                FrameString debugName = Format<FrameString>(CUBE_T("[{0}] {1}"), materialName, textureName);

                if (textureIndex == -1)
                {
                    CUBE_LOG(Warning, ModelLoaderSystem, "Cannot load {0}: invalid texture index", debugName);
                    return nullptr;
                }
                int imageIndex = model.textures[textureIndex].source;
                if (imageIndex == -1)
                {
                    CUBE_LOG(Warning, ModelLoaderSystem, "Cannot load {0}: invalid image index", debugName);
                    return nullptr;
                }
                HashMap<int, SharedPtr<TextureResource>>::iterator cacheIt = loadedImageCache.find(imageIndex);
                if (cacheIt != loadedImageCache.end())
                {
                    return cacheIt->second;
                }
                tinygltf::Image& image = model.images[imageIndex];
                if (image.image.empty())
                {
                    CUBE_LOG(Warning, ModelLoaderSystem, "Cannot load {0}: empty image data", debugName);
                    return nullptr;
                }
                // Append file path.
                debugName = Format<FrameString>(CUBE_T("{0}({1})"), debugName, image.uri);

                gapi::ElementFormat format = gapi::ElementFormat::Unknown;
                if (image.component == 4)
                {
                    if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        format = gapi::ElementFormat::RGBA8_UNorm;
                    }
                    else if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                    {
                        format = gapi::ElementFormat::RGBA16_UNorm;
                    }
                }
                if (format == gapi::ElementFormat::Unknown)
                {
                    CUBE_LOG(Warning, ModelLoaderSystem, "Cannot load {0}: Unsupported element format (component: {1}, pixel_type: {2})", debugName, image.component, image.pixel_type);
                    return nullptr;
                }

                TextureResourceCreateInfo createInfo = {
                    .textureInfo = {
                        .format = format,
                        .type = gapi::TextureType::Texture2D,
                        .width = static_cast<Uint32>(image.width),
                        .height = static_cast<Uint32>(image.height),
                    },
                    .data = BlobView(image.image.data(), image.image.size()),
                    .bytesPerElement = static_cast<Uint32>(image.component * image.bits / 8),
                    .generateMipMaps = true,
                    .debugName = debugName
                };
                SharedPtr<TextureResource> texture = std::make_shared<TextureResource>(createInfo);
                loadedImageCache.emplace(imageIndex, texture);

                return texture;
            };

            FrameString materialName = String_Convert<FrameString>(gltfMaterial.name);
            materials.push_back(std::make_shared<Material>(materialName));

            SharedPtr<Material> material = materials.back();

            if (gltfMaterial.alphaMode == "MASK")
            {
                material->SetMode(MaterialMode::Mask);
                material->SetAlphaCutoff(static_cast<float>(gltfMaterial.alphaCutoff));
            }
            // Otherwise use default value (opaque).
            // TODO: Implement BLEND mode.

            FrameString channelMappingCode;
            if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                material->SetTexture(0, LoadTexture(materialName, CUBE_T("baseColorTexture"), gltfMaterial.pbrMetallicRoughness.baseColorTexture.index));
                channelMappingCode += CUBE_T("float4 baseColor = materialData.textureSlot0.Sample(GetStaticLinearWrapSampler(), input.uv).rgba;\n");
                // Encoded in sRGB. Decode to linear.
                channelMappingCode += CUBE_T("value.albedo = GammaCorrection::sRGBToLinear(baseColor.rgb);\n");
                channelMappingCode += CUBE_T("value.alpha = baseColor.a;\n");

                material->AddAdditionalModule(CUBE_T("StaticSampler"));
                material->AddAdditionalModule(CUBE_T("GammaCorrection"));
            }
            if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                material->SetTexture(1, LoadTexture(materialName, CUBE_T("metallicRoughnessTexture"), gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index));
                channelMappingCode += CUBE_T("float3 roughnessAndMetallic = materialData.textureSlot1.Sample(GetStaticLinearWrapSampler(), input.uv).rgb;\n");
                channelMappingCode += CUBE_T("value.metallic = roughnessAndMetallic.b;\n");
                channelMappingCode += CUBE_T("value.roughness = roughnessAndMetallic.g;\n");

                material->AddAdditionalModule(CUBE_T("StaticSampler"));
            }
            if (gltfMaterial.normalTexture.index != -1)
            {
                material->SetTexture(2, LoadTexture(materialName, CUBE_T("normalTexture"), gltfMaterial.normalTexture.index));
                channelMappingCode += CUBE_T("float3 normal = normalize(materialData.textureSlot2.Sample(GetStaticLinearWrapSampler(), input.uv).rgb * 2.0f - 1.0f);\n");
                channelMappingCode += CUBE_T("value.normal = normal;\n");

                material->AddAdditionalModule(CUBE_T("StaticSampler"));
            }
            if (gltfMaterial.emissiveTexture.index != -1)
            {
                material->SetTexture(3, LoadTexture(materialName, CUBE_T("emissiveTexture"), gltfMaterial.emissiveTexture.index));
                // Encoded in sRGB. Decode to linear.
                channelMappingCode += CUBE_T("float3 emissive = materialData.textureSlot3.Sample(GetStaticLinearWrapSampler(), input.uv).rgb;\n");
                channelMappingCode += CUBE_T("value.emissive = GammaCorrection::sRGBToLinear(emissive);\n");

                material->AddAdditionalModule(CUBE_T("StaticSampler"));
                material->AddAdditionalModule(CUBE_T("GammaCorrection"));
            }
            if (gltfMaterial.occlusionTexture.index != -1)
            {
                material->SetTexture(4, LoadTexture(materialName, CUBE_T("occlusionTexture"), gltfMaterial.occlusionTexture.index));
                channelMappingCode += CUBE_T("float occlusion = materialData.textureSlot4.Sample(GetStaticLinearWrapSampler(), input.uv).r;\n");
                channelMappingCode += CUBE_T("value.indirectOcclusion = occlusion;\n");

                material->AddAdditionalModule(CUBE_T("StaticSampler"));
            }
            material->SetChannelMappingCode(channelMappingCode);
        }

        // Load meshes.
        FrameVector<SharedPtr<Mesh>> meshes;
        FrameVector<FrameVector<WeakPtr<Material>>> materialsPerMeshes;

        for (const tinygltf::Mesh& mesh : model.meshes)
        {
            constexpr int NONE = -1;

            FrameVector<Vertex> vertices;
            FrameVector<Index> indices;
            FrameVector<SubMesh> subMeshes;

            FrameVector<WeakPtr<Material>>& materialsPerMesh = materialsPerMeshes.emplace_back();

            for (const tinygltf::Primitive& prim : mesh.primitives)
            {
                int positionAccessor = NONE;
                int normalAccessor = NONE;
                int tangentAccessor = NONE;
                int colorAccessor = NONE;
                int texCoordAccessor = NONE;

                if (auto posIt = prim.attributes.find("POSITION"); posIt != prim.attributes.end())
                {
                    positionAccessor = posIt->second;
                }
                if (auto normalIt = prim.attributes.find("NORMAL"); normalIt != prim.attributes.end())
                {
                    normalAccessor = normalIt->second;
                }
                if (auto tangentIt = prim.attributes.find("TANGENT"); tangentIt != prim.attributes.end())
                {
                    tangentAccessor = tangentIt->second;
                }
                if (auto colorIt = prim.attributes.find("COLOR_0"); colorIt != prim.attributes.end())
                {
                    colorAccessor = colorIt->second;
                }
                if (auto texIt = prim.attributes.find("TEXCOORD_0"); texIt != prim.attributes.end())
                {
                    texCoordAccessor = texIt->second;
                }

                Uint64 numVertices = 0;
                auto UpdateNumVertices = [&model, &numVertices](int accessorIndex)
                {
                    if (accessorIndex != NONE)
                    {
                        const Uint64 count = model.accessors[accessorIndex].count;
                        if (numVertices > 0 && numVertices != count)
                        {
                            CUBE_LOG(Warning, ModelLoaderSystem, "Mismatch count in vertices ({0} != {1}). Use the greater one.", numVertices, count);
                        }
                        numVertices = std::max(numVertices, count);
                    }
                };
                UpdateNumVertices(positionAccessor);
                UpdateNumVertices(normalAccessor);
                UpdateNumVertices(tangentAccessor);
                UpdateNumVertices(colorAccessor);
                UpdateNumVertices(texCoordAccessor);

                const Uint64 vertexOffset = vertices.size();
                const Uint64 indexOffset = indices.size();
                const Uint64 numIndices = model.accessors[prim.indices].count;

                subMeshes.push_back({
                    .vertexOffset = vertexOffset,
                    .indexOffset = indexOffset,
                    .numIndices = numIndices,
                    .materialIndex = static_cast<int>(materialsPerMesh.size()),
                    .debugName = Format<String>(CUBE_T("{0}"), mesh.name)
                });
                materialsPerMesh.emplace_back(prim.material != -1 ? materials[prim.material] : nullptr);

                vertices.insert(vertices.end(), numVertices, {});

                auto ProcessData = [&model](int accessorIndex, std::function<void(const tinygltf::Accessor&)> checker, std::function<void(Uint64, const void*, int, int)> onComponent)
                {
                    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    checker(accessor);

                    const Uint64 count = accessor.count;
                    const Uint64 offset = accessor.byteOffset + bufferView.byteOffset;
                    CHECK(accessor.ByteStride(bufferView) == std::max((Uint64)bufferView.byteStride, (Uint64)tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type)));
                    const Uint64 stride = accessor.ByteStride(bufferView);
                    for (Uint64 i = 0; i < count; ++i)
                    {
                        onComponent(i, (void*)&(buffer.data[offset + stride * i]), accessor.type, accessor.componentType);
                    }
                };
                // POSITION
                if (positionAccessor != NONE)
                {
                    ProcessData(positionAccessor,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_VEC3);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        },
                        [&vertices, vertexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            float xyz[3];
                            memcpy(xyz, pData, sizeof(xyz));

                            vertices[vertexOffset + index].position = { xyz[0], xyz[1], xyz[2], 1.0f };
                        }
                    );
                }
                // NORMAL
                if (normalAccessor != NONE)
                {
                    ProcessData(normalAccessor,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_VEC3);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        },
                        [&vertices, vertexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            float xyz[3];
                            memcpy(xyz, pData, sizeof(xyz));

                            vertices[vertexOffset + index].normal = { xyz[0], xyz[1], xyz[2] };
                        }
                    );
                }
                else
                {
                    CUBE_LOG(Info, ModelLoaderSystem, "No normal data found in the model. Calculate normal from position and index.");
                    MeshHelper::SetNormalVector(ArrayView(vertices.begin() + vertexOffset, numVertices), ArrayView(indices.begin() + indexOffset, numIndices));
                }
                // TANGENT
                if (tangentAccessor != NONE)
                {
                    ProcessData(tangentAccessor,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_VEC4);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        },
                        [&vertices, vertexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            float xyzw[4];
                            memcpy(xyzw, pData, sizeof(xyzw));

                            vertices[vertexOffset + index].tangent = { xyzw[0], xyzw[1], xyzw[2], xyzw[3] };
                        }
                    );
                }
                else
                {
                    CUBE_LOG(Info, ModelLoaderSystem, "No tangent data found in the model. Calculate approximate tangent from normal.");
                    MeshHelper::SetApproxTangentVector(ArrayView(vertices.begin() + vertexOffset, numVertices));
                }
                // TEXCOORD
                if (texCoordAccessor != NONE)
                {
                    ProcessData(texCoordAccessor,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_VEC2);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                        },
                        [&vertices, vertexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            float uv[2];
                            if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                memcpy(uv, pData, sizeof(uv));
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                            {
                                unsigned char uvByte[2];
                                memcpy(uvByte, pData, sizeof(uvByte));
                                uv[0] = static_cast<float>(uvByte[0]) / 255.0f;
                                uv[1] = static_cast<float>(uvByte[1]) / 255.0f;
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            {
                                unsigned short uvShort[2];
                                memcpy(uvShort, pData, sizeof(uvShort));
                                uv[0] = static_cast<float>(uvShort[0]) / 65535.0f;
                                uv[1] = static_cast<float>(uvShort[1]) / 65535.0f;
                            }

                            vertices[vertexOffset + index].uv = { uv[0], uv[1] };
                        }
                    );
                }
                // COLOR
                if (colorAccessor != NONE)
                {
                    ProcessData(colorAccessor,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_VEC3
                                || accessor.type == TINYGLTF_TYPE_VEC4);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                        },
                        [&vertices, vertexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            float rgba[4];
                            rgba[3] = 1.0f;

                            int num = 4;
                            if (type == TINYGLTF_TYPE_VEC3)
                            {
                                num = 3;
                            }

                            if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                memcpy(rgba, pData, sizeof(float) * num);
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                            {
                                unsigned char rgbaByte[4];
                                rgbaByte[3] = 255;
                                memcpy(rgbaByte, pData, sizeof(unsigned char) * num);
                                rgba[0] = static_cast<float>(rgbaByte[0]) / 255.0f;
                                rgba[1] = static_cast<float>(rgbaByte[1]) / 255.0f;
                                rgba[2] = static_cast<float>(rgbaByte[2]) / 255.0f;
                                rgba[3] = static_cast<float>(rgbaByte[3]) / 255.0f;
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            {
                                unsigned short rgbaShort[4];
                                rgbaShort[3] = 65535;
                                memcpy(rgbaShort, pData, sizeof(unsigned short) * num);
                                rgba[0] = static_cast<float>(rgbaShort[0]) / 65535.0f;
                                rgba[1] = static_cast<float>(rgbaShort[1]) / 65535.0f;
                                rgba[2] = static_cast<float>(rgbaShort[2]) / 65535.0f;
                                rgba[3] = static_cast<float>(rgbaShort[3]) / 65535.0f;
                            }

                            vertices[vertexOffset + index].color = { rgba[0], rgba[1], rgba[2], rgba[3] };
                        }
                    );
                }

                // Index
                {
                    CHECK_FORMAT(prim.mode == TINYGLTF_MODE_TRIANGLES, "Currently only support triangle mode.");
                    indices.insert(indices.end(), numIndices, {});
                    ProcessData(prim.indices,
                        [](const tinygltf::Accessor& accessor)
                        {
                            CHECK(accessor.type == TINYGLTF_TYPE_SCALAR);
                            CHECK(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                                || accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        },
                        [&indices, indexOffset](Uint64 index, const void* pData, int type, int componentType)
                        {
                            Uint32 v = 0;
                            if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                            {
                                unsigned char ch;
                                memcpy(&ch, pData, sizeof(ch));
                                v = ch;
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            {
                                unsigned short sh;
                                memcpy(&sh, pData, sizeof(sh));
                                v = sh;
                            }
                            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                            {
                                unsigned int in;
                                memcpy(&in, pData, sizeof(in));
                                v = in;
                            }

                            indices[indexOffset + index] = v;
                        }
                    );
                }
            }

            SharedPtr<MeshData> meshData = std::make_shared<MeshData>(vertices, indices, subMeshes, String_Convert<String>(mesh.name));
            meshes.push_back(std::make_shared<Mesh>(meshData, GetMeshMetadata()));
        }

        // Make scene and scene objects.
        SharedPtr<Scene> scene = std::make_shared<Scene>();

        if (model.defaultScene != -1)
        {
            tinygltf::Scene& gltfScene = model.scenes[model.defaultScene];
            for (int nodeIndex : gltfScene.nodes)
            {
                tinygltf::Node& node = model.nodes[nodeIndex];

                UniquePtr<SceneObject> obj = std::make_unique<SceneObject>(
                    String_Convert<FrameString>(node.name),
                    node.mesh != -1 ? meshes[node.mesh] : nullptr);
                if (node.mesh != -1)
                {
                    obj->SetMaterials(materialsPerMeshes[node.mesh]);
                }

                if (!node.translation.empty())
                {
                    obj->SetPosition({ (float)node.translation[0], (float)node.translation[1], (float)node.translation[2] });
                }
                if (!node.rotation.empty())
                {
                    // TODO: Quat to euler?
                }
                if (!node.scale.empty())
                {
                    obj->SetScale({ (float)node.scale[0], (float)node.scale[1], (float)node.scale[2] });
                }

                scene->AddSceneObject(std::move(obj));
            }

            for (SharedPtr<Material>& material : materials)
            {
                scene->AddMaterial(material);
            }
        }

        return scene;
    }

    SharedPtr<Scene> ModelLoaderSystem::LoadModel_Obj(const ModelPathInfo& pathInfo)
    {
        FrameString modelName = String_Convert<FrameString>(pathInfo.name);

        // Collect all .obj files in the path.
        Vector<String> fileList = platform::FileSystem::GetList(pathInfo.path);
        Vector<String> objFiles;
        for (const String& file : fileList)
        {
            if (file.size() >= 4 && file.substr(file.size() - 4) == CUBE_T(".obj"))
            {
                objFiles.push_back(file);
            }
        }

        if (objFiles.empty())
        {
            CUBE_LOG(Error, ModelLoaderSystem, "No .obj files found in folder: {0}", modelName);
            return {};
        }

        SharedPtr<Scene> scene = std::make_shared<Scene>();

        Vector<SharedPtr<Material>> materials;

        for (const String& objFile : objFiles)
        {
            AnsiString objFilePathAnsi = (pathInfo.path / objFile).ToAnsiString();

            tinyobj::ObjReaderConfig readerConfig;
            tinyobj::ObjReader reader;
            if (!reader.ParseFromFile(objFilePathAnsi, readerConfig))
            {
                if (!reader.Error().empty())
                {
                    CUBE_LOG(Error, ModelLoaderSystem, "Failed to load obj file: {0}", reader.Error());
                }
                continue;
            }

            if (!reader.Warning().empty())
            {
                CUBE_LOG(Warning, ModelLoaderSystem, "Warning while loading obj: {0}", reader.Warning());
            }

            FrameVector<WeakPtr<Material>> materialsPerObject;
            // Load materials.
            const std::vector<tinyobj::material_t>& objMaterials = reader.GetMaterials();

            for (const tinyobj::material_t& objMaterial : objMaterials)
            {
                FrameString materialName = Format<FrameString>(CUBE_T("{0}({1})"), modelName, objMaterial.name);
                SharedPtr<Material> material = std::make_shared<Material>(materialName);

                material->SetBaseColor(Vector4(objMaterial.diffuse[0], objMaterial.diffuse[1], objMaterial.diffuse[2], 1.0f));

                auto LoadTexture = [&modelName, &pathInfo](const Character* textureName, AnsiStringView objTextureName) -> SharedPtr<TextureResource>
                {
                    // Normalize backslashes to forward slashes for cross-platform
                    AnsiString objTextureNameAnsi = AnsiString(objTextureName);
                    for (char& c : objTextureNameAnsi)
                    {
                        if (c == '\\')
                        {
                            c = '/';
                        }
                    }
                    AnsiString texturePath = (pathInfo.path / objTextureNameAnsi).ToAnsiString();

                    int width, height, channels;
                    unsigned char* imageData = stbi_load(texturePath.data(), &width, &height, &channels, 4);
                    if (imageData)
                    {
                        FrameString debugName = Format<FrameString>(CUBE_T("[{0}] {1} ({2})"), modelName, textureName, objTextureNameAnsi);

                        TextureResourceCreateInfo createInfo = {
                            .textureInfo = {
                                .format = gapi::ElementFormat::RGBA8_UNorm,
                                .type = gapi::TextureType::Texture2D,
                                .width = static_cast<Uint32>(width),
                                .height = static_cast<Uint32>(height),
                            },
                            .data = BlobView(imageData, static_cast<Uint64>(width) * height * 4),
                            .bytesPerElement = 4,
                            .generateMipMaps = true,
                            .debugName = debugName
                        };
                        SharedPtr<TextureResource> texture = std::make_shared<TextureResource>(createInfo);
                        stbi_image_free(imageData);

                        return texture;
                    }
                    else
                    {
                        CUBE_LOG(Warning, ModelLoaderSystem, "Failed to load texture: {0}", texturePath);
                        return nullptr;
                    }
                };

                bool isPBR = !objMaterial.metallic_texname.empty() || !objMaterial.roughness_texname.empty();
                material->SetIsPBR(isPBR);

                FrameString channelMappingCode;
                if (isPBR)
                {
                    if (!objMaterial.diffuse_texname.empty())
                    {
                        material->SetTexture(0, LoadTexture(CUBE_T("baseColorTexture"), objMaterial.diffuse_texname));
                        channelMappingCode += CUBE_T("value.albedo = materialData.textureSlot0.Sample(GetStaticLinearWrapSampler(), input.uv).rgb;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                    if (!objMaterial.metallic_texname.empty())
                    {
                        material->SetTexture(1, LoadTexture(CUBE_T("metallicTexture"), objMaterial.metallic_texname));
                        channelMappingCode += CUBE_T("float t1 = materialData.textureSlot1.Sample(GetStaticLinearWrapSampler(), input.uv).r;\n");
                        channelMappingCode += CUBE_T("value.metallic = t1;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                    if (!objMaterial.roughness_texname.empty())
                    {
                        material->SetTexture(2, LoadTexture(CUBE_T("roughnessTexture"), objMaterial.roughness_texname));
                        channelMappingCode += CUBE_T("float t2 = materialData.textureSlot2.Sample(GetStaticLinearWrapSampler(), input.uv).r;\n");
                        channelMappingCode += CUBE_T("value.roughness = t2;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                    if (!objMaterial.normal_texname.empty())
                    {
                        material->SetTexture(3, LoadTexture(CUBE_T("normalTexture"), objMaterial.normal_texname));
                        channelMappingCode += CUBE_T("float3 t3 = normalize(materialData.textureSlot3.Sample(GetStaticLinearWrapSampler(), input.uv).rgb * 2.0f - 1.0f);\n");
                        channelMappingCode += CUBE_T("value.normal = t3;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                }
                else
                {
                    if (!objMaterial.diffuse_texname.empty())
                    {
                        material->SetTexture(0, LoadTexture(CUBE_T("diffuseTexture"), objMaterial.diffuse_texname));
                        channelMappingCode += CUBE_T("value.diffuseColor = materialData.textureSlot0.Sample(GetStaticLinearWrapSampler(), input.uv).rgb;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                    else
                    {
                        material->SetDiffuseColor(Vector4(objMaterial.diffuse[0], objMaterial.diffuse[1], objMaterial.diffuse[2], 1.0f));
                        channelMappingCode += CUBE_T("value.diffuseColor = materialData.diffuseColor.rgb;\n");
                    }
                    if (!objMaterial.specular_texname.empty())
                    {
                        material->SetTexture(1, LoadTexture(CUBE_T("specularTexture"), objMaterial.specular_texname));
                        channelMappingCode += CUBE_T("value.specularColor = materialData.textureSlot1.Sample(GetStaticLinearWrapSampler(), input.uv).rgb;\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                    else
                    {
                        material->SetSpecularColor(Vector4(objMaterial.specular[0], objMaterial.specular[1], objMaterial.specular[2], 1.0f));
                        channelMappingCode += CUBE_T("value.specularColor = materialData.specularColor.rgb;\n");

                    }
                    material->SetShininess(objMaterial.shininess);
                    channelMappingCode += CUBE_T("value.shininess = materialData.shininess;\n");
                    if (!objMaterial.normal_texname.empty())
                    {
                        material->SetTexture(2, LoadTexture(CUBE_T("normalTexture"), objMaterial.normal_texname));
                        channelMappingCode += CUBE_T("value.normal = normalize(materialData.textureSlot2.Sample(GetStaticLinearWrapSampler(), input.uv).rgb * 2.0f - 1.0f);\n");

                        material->AddAdditionalModule(CUBE_T("StaticSampler"));
                    }
                }
                material->SetChannelMappingCode(channelMappingCode);

                materials.push_back(material);
                materialsPerObject.push_back(materials.back());
            }

            const tinyobj::attrib_t& attrib = reader.GetAttrib();
            const std::vector<tinyobj::shape_t>& objShapes = reader.GetShapes();

            // Load meshes.
            FrameVector<Vertex> vertices;
            FrameVector<Index> indices;
            FrameVector<SubMesh> subMeshes;

            // tinyobj loads vertex attributes in each separated buffer. (SoA)
            // To convert AoS, add vertex based on each index keys.
            struct IndexKey
            {
                int vertexIndex;
                int normalIndex;
                int texcoordIndex;

                bool operator<(const IndexKey& rhs) const
                {
                    if (vertexIndex != rhs.vertexIndex) return vertexIndex < rhs.vertexIndex;
                    if (normalIndex != rhs.normalIndex) return normalIndex < rhs.normalIndex;
                    return texcoordIndex < rhs.texcoordIndex;
                }
            };

            for (const tinyobj::shape_t& objShape : objShapes)
            {
                const tinyobj::mesh_t& objMesh = objShape.mesh;

                // Split faces into submeshes by material id.
                const Uint32 numFaces = static_cast<Uint32>(objMesh.num_face_vertices.size());
                Vector<Uint32> faceObjIndexOffsets(numFaces);
                {
                    Uint32 offset = 0;
                    for (Uint32 f = 0; f < numFaces; ++f)
                    {
                        faceObjIndexOffsets[f] = offset;
                        offset += objMesh.num_face_vertices[f];
                    }
                }

                Map<int, Vector<Uint32>> facesByObjMaterial;
                for (Uint32 f = 0; f < numFaces; ++f)
                {
                    int matId = objMesh.material_ids[f];
                    facesByObjMaterial[matId].push_back(f);
                }

                int subMeshIndexPerShape = 0;
                for (const auto& [matId, faces] : facesByObjMaterial)
                {
                    const Uint64 vertexOffset = vertices.size();
                    const Uint64 indexOffset = indices.size();

                    Map<IndexKey, Uint32> vertexMap;
                    bool hasNormals = true;

                    for (Uint32 f : faces)
                    {
                        Uint32 faceVertexCount = objShape.mesh.num_face_vertices[f];
                        Uint32 faceObjIndexOffset = faceObjIndexOffsets[f];

                        if (faceVertexCount != 3)
                        {
                            CUBE_LOG(Warning, ModelLoaderSystem, "Only 3 vertices supported. ({0}) Ignore that face.", faceVertexCount);
                            continue;
                        }

                        for (Uint32 v = 0; v < faceVertexCount; ++v)
                        {
                            const tinyobj::index_t& idx = objShape.mesh.indices[faceObjIndexOffset + v];
                            IndexKey key = { idx.vertex_index, idx.normal_index, idx.texcoord_index };

                            auto findIter = vertexMap.find(key);
                            if (findIter == vertexMap.end())
                            {
                                Vertex vertex = {};

                                if (idx.vertex_index != -1)
                                {
                                    vertex.position = {
                                        attrib.vertices[3 * idx.vertex_index + 0],
                                        attrib.vertices[3 * idx.vertex_index + 1],
                                        attrib.vertices[3 * idx.vertex_index + 2]
                                    };
                                }

                                if (idx.normal_index != -1)
                                {
                                    vertex.normal = {
                                        attrib.normals[3 * idx.normal_index + 0],
                                        attrib.normals[3 * idx.normal_index + 1],
                                        attrib.normals[3 * idx.normal_index + 2]
                                    };
                                }
                                else
                                {
                                    hasNormals = false;
                                }

                                if (idx.texcoord_index != -1)
                                {
                                    vertex.uv = {
                                        attrib.texcoords[2 * idx.texcoord_index + 0],
                                        attrib.texcoords[2 * idx.texcoord_index + 1]
                                    };
                                }

                                findIter = vertexMap.insert({key, static_cast<Uint32>(vertices.size())}).first;
                                vertices.push_back(vertex);
                            }
                            indices.push_back(findIter->second - static_cast<Uint32>(vertexOffset));
                        }
                    }

                    const Uint64 numIndices = indices.size() - indexOffset;

                    subMeshes.push_back({
                        .vertexOffset = vertexOffset,
                        .indexOffset = indexOffset,
                        .numIndices = numIndices,
                        .materialIndex = (matId >= 0) ? matId : -1,
                        .debugName = Format<String>(CUBE_T("[{0}] {1}_{2} ({3})"), modelName, objShape.name, subMeshIndexPerShape, objFile)
                    });
                    subMeshIndexPerShape++;

                    Uint64 submeshVertexCount = vertices.size() - vertexOffset;
                    if (!hasNormals)
                    {
                        CUBE_LOG(Info, ModelLoaderSystem, "No normal data found in obj shape '{0}'. Calculating normals.", objShape.name);
                        MeshHelper::SetNormalVector(ArrayView(vertices.begin() + vertexOffset, submeshVertexCount), ArrayView(indices.begin() + indexOffset, numIndices));
                    }
                    MeshHelper::SetApproxTangentVector(ArrayView(vertices.begin() + vertexOffset, submeshVertexCount));
                }
            }

            // Make scene object.
            SharedPtr<Mesh> mesh = std::make_shared<Mesh>(
                std::make_shared<MeshData>(vertices, indices, subMeshes, objFile),
                GetMeshMetadata()
            );

            UniquePtr<SceneObject> obj = std::make_unique<SceneObject>(objFile, mesh);
            obj->SetMaterials(materialsPerObject);
            scene->AddSceneObject(std::move(obj));
        }

        for (SharedPtr<Material>& material : materials)
        {
            scene->AddMaterial(material);
        }

        return scene;
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
