#include "GLTFModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "CubeString.h"
#include "Engine.h"
#include "Logger.h"
#include "FileSystem.h"
#include "GAPI_Texture.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/MeshHelper.h"
#include "Renderer/Texture.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

namespace cube
{
    const Vector<ModelPathInfo>& GLTFModelLoader::GetModelList()
    {
        mModelList.clear();

        struct ModelLoadInfo
        {
            const Character* name;
            Vector3 position = Vector3::Zero();
            Vector3 rotation = Vector3::Zero();
            Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
        };

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
                    mModelList.push_back({
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

        return mModelList;
    }

    SharedPtr<Scene> GLTFModelLoader::LoadModel(const ModelPathInfo& pathInfo, const MeshMetadata& meshMetadata)
    {
        tinygltf::Model model;
        AnsiString error;
        AnsiString warning;
        tinygltf::TinyGLTF loader;

        AnsiString pathStr = pathInfo.path.ToAnsiString();
        bool res = loader.LoadASCIIFromFile(&model, &error, &warning, pathStr);

        if (!warning.empty())
        {
            CUBE_LOG(Warning, GLTFModelLoader, "There's some warning while loading from glTF: {}", warning);
        }

        if (!error.empty())
        {
            CUBE_LOG(Error, GLTFModelLoader, "There's some error while loading from glTF: {}", error);
        }

        if (!res)
        {
            CUBE_LOG(Error, GLTFModelLoader, "Failed to load the model from glTF");
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
                    CUBE_LOG(Warning, GLTFModelLoader, "Cannot load {0}: invalid texture index", debugName);
                    return nullptr;
                }
                int imageIndex = model.textures[textureIndex].source;
                if (imageIndex == -1)
                {
                    CUBE_LOG(Warning, GLTFModelLoader, "Cannot load {0}: invalid image index", debugName);
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
                    CUBE_LOG(Warning, GLTFModelLoader, "Cannot load {0}: empty image data", debugName);
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
                    CUBE_LOG(Warning, GLTFModelLoader, "Cannot load {0}: Unsupported element format (component: {1}, pixel_type: {2})", debugName, image.component, image.pixel_type);
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
                            CUBE_LOG(Warning, GLTFModelLoader, "Mismatch count in vertices ({0} != {1}). Use the greater one.", numVertices, count);
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
                    CUBE_LOG(Info, GLTFModelLoader, "No normal data found in the model. Calculate normal from position and index.");
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

                // Generate tangent vectors if missing.
                if (tangentAccessor == NONE)
                {
                    CUBE_LOG(Info, GLTFModelLoader, "No tangent data found in the model. Calculating tangents.");
                    MeshHelper::CalculateTangentVectors(ArrayView(vertices.begin() + vertexOffset, numVertices), ArrayView(indices.begin() + indexOffset, numIndices));
                }
            }

            SharedPtr<MeshData> meshData = std::make_shared<MeshData>(vertices, indices, subMeshes, String_Convert<String>(mesh.name));
            meshes.push_back(std::make_shared<Mesh>(meshData, meshMetadata));
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
} // namespace cube
