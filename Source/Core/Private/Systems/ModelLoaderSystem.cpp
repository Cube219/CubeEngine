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
#include "CubeString.h"
#include "Engine.h"
#include "FileSystem.h"
#include "GAPI_Texture.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/MeshHelper.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture.h"

namespace cube
{
    Vector<ModelPathInfo> ModelLoaderSystem::mModelPathList;
    int ModelLoaderSystem::mCurrentSelectModelIndex;

    float ModelLoaderSystem::mModelScale;

    void ModelLoaderSystem::Initialize()
    {
        mCurrentSelectModelIndex = -1;
        ResetModelScale();
    }

    void ModelLoaderSystem::Shutdown()
    {
    }

    void ModelLoaderSystem::OnLoopImGUI()
    {
        ImGui::Begin("Model Loader", 0, ImGuiWindowFlags_AlwaysAutoResize);

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
        switch (pathInfo.type)
        {
        case ModelType::glTF:
            return LoadModel_glTF(pathInfo);
        default:
            NOT_IMPLEMENTED();
        }

        return {};
    }
    

    void ModelLoaderSystem::LoadModelList()
    {
        mModelPathList.clear();

        FrameString resourceBasePath = FrameString(Engine::GetRootDirectoryPath()) + CUBE_T("/Resources/Models/glTFSampleAssets/Models/");
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
            bool contained = false;
            for (const Character* modelName : gltfLoadModels)
            {
                if (e == modelName)
                {
                    contained = true;
                    break;
                }
            }

            if (contained)
            {
                mModelPathList.push_back({
                    .type = ModelType::glTF,
                    .name = String_Convert<AnsiString>(e),
                    .path = Format<String>(CUBE_T("{0}/{1}/glTF/{1}.gltf"), resourceBasePath, e)
                });
            }
        }
    }

    void ModelLoaderSystem::LoadCurrentModelAndSet()
    {
        const ModelPathInfo& info = mModelPathList[mCurrentSelectModelIndex];

        ModelResources resources = LoadModel(info);
        Engine::SetMesh(resources.mesh);
        Engine::SetMaterials(resources.materials);
    }

    ModelResources ModelLoaderSystem::LoadModel_glTF(const ModelPathInfo& pathInfo)
    {
        tinygltf::Model model;
        AnsiString error;
        AnsiString warning;
        tinygltf::TinyGLTF loader;

        AnsiString pathStr;
        String_ConvertAndAppend(pathStr, pathInfo.path);
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

        FrameString pathInfoName = String_Convert<FrameString>(pathInfo.name);

        // Load meshes
        FrameVector<Vertex> vertices;
        FrameVector<Index> indices;
        FrameVector<SubMesh> subMeshes;

        for (const tinygltf::Mesh& mesh : model.meshes)
        {
            constexpr int NONE = -1;
            
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
                    .materialIndex = prim.material,
                    .debugName = String_Convert<String>(mesh.name)
                });

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
        }

        // Load materials
        Vector<SharedPtr<Material>> materials;

        for (const tinygltf::Material& gltfMaterial : model.materials)
        {
            auto LoadTexture = [&model, &pathInfoName](const Character* textureName, int textureIndex) -> SharedPtr<TextureResource>
            {
                FrameString debugName = Format<FrameString>(CUBE_T("[{0}({1})] Texture"), pathInfoName, textureName);

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
                tinygltf::Image& image = model.images[imageIndex];
                if (image.image.empty())
                {
                    CUBE_LOG(Warning, ModelLoaderSystem, "Cannot load {0}: empty image data", debugName);
                    return nullptr;
                }
                // Append file name
                debugName = Format<FrameString>(CUBE_T("{0} ({1})"), debugName, image.uri);

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
                    .type = gapi::TextureType::Texture2D,
                    .format = format,
                    .width = static_cast<Uint32>(image.width),
                    .height = static_cast<Uint32>(image.height),
                    .data = BlobView(image.image.data(), image.image.size()),
                    .bytesPerElement = static_cast<Uint32>(image.component * image.bits / 8),
                    .generateMipMaps = true,
                    .debugName = debugName
                };
                SharedPtr<TextureResource> texture = std::make_shared<TextureResource>(createInfo);

                return texture;
            };

            FrameString materialName = Format<FrameString>(CUBE_T("{0}({1})"), pathInfoName, gltfMaterial.name);
            materials.push_back(std::make_shared<Material>(materialName));

            // TODO: Remove duplication - check the image is used first
            SharedPtr<Material> material = materials.back();

            FrameString channelMappingCode;
            if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index != -1)
            {
                material->SetTexture(0, LoadTexture(CUBE_T("baseColorTexture"), gltfMaterial.pbrMetallicRoughness.baseColorTexture.index));
                channelMappingCode += CUBE_T("value.albedo = materialData.textureSlot0.Sample(input.uv).rgb;\n");
            }
            if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
            {
                material->SetTexture(1, LoadTexture(CUBE_T("metallicRoughnessTexture"), gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index));
                channelMappingCode += CUBE_T("float3 t1 = materialData.textureSlot1.Sample(input.uv).rgb;\n");
                channelMappingCode += CUBE_T("value.metallic = t1.g;\n");
                channelMappingCode += CUBE_T("value.roughness = t1.b;\n");
            }
            if (gltfMaterial.normalTexture.index != -1)
            {
                material->SetTexture(2, LoadTexture(CUBE_T("normalTexture"), gltfMaterial.normalTexture.index));
                channelMappingCode += CUBE_T("float3 t2 = normalize(materialData.textureSlot2.Sample(input.uv).rgb * 2.0f - 1.0f);\n");
                channelMappingCode += CUBE_T("value.normal = t2;\n");
            }

            material->SetChannelMappingCode(channelMappingCode);
        }

        ModelResources loadedResources = {
            .mesh = std::make_shared<MeshData>(vertices, indices, subMeshes, pathInfoName),
            .materials = std::move(materials)
        };

        return loadedResources;
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
