#include "ObjModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "stb_image.h" // Implementation defined in GLTFModelLoader.cpp

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "CubeString.h"
#include "Engine.h"
#include "Logger.h"
#include "FileSystem.h"
#include "GAPI_Texture.h"
#include "RenderCore/Material.h"
#include "RenderCore/Mesh.h"
#include "RenderCore/MeshHelper.h"
#include "RenderCore/Texture.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"

namespace cube
{
    const Vector<ModelPathInfo>& ObjModelLoader::GetModelList()
    {
        mModelList.clear();

        struct ModelLoadInfo
        {
            const Character* name;
            Vector3 position = Vector3::Zero();
            Vector3 rotation = Vector3::Zero();
            Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
        };

        platform::FilePath objBasePath = Engine::GetRootDirectoryPath() / CUBE_T("Resources/Models/DefaultModels");
        static const ModelLoadInfo objLoadModels[] = {
            {
                .name = CUBE_T("CornellBox"),
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
                    mModelList.push_back({
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

        return mModelList;
    }

    SharedPtr<Scene> ObjModelLoader::LoadModel(const ModelPathInfo& pathInfo, const MeshMetadata& meshMetadata)
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
            CUBE_LOG(Error, ObjModelLoader, "No .obj files found in folder: {0}", modelName);
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
                    CUBE_LOG(Error, ObjModelLoader, "Failed to load obj file: {0}", reader.Error());
                }
                continue;
            }

            if (!reader.Warning().empty())
            {
                CUBE_LOG(Warning, ObjModelLoader, "Warning while loading obj: {0}", reader.Warning());
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
                        SharedPtr<TextureResource> texture = TextureResource::Create(createInfo);
                        stbi_image_free(imageData);

                        return texture;
                    }
                    else
                    {
                        CUBE_LOG(Warning, ObjModelLoader, "Failed to load texture: {0}", texturePath);
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
                            CUBE_LOG(Warning, ObjModelLoader, "Only 3 vertices supported. ({0}) Ignore that face.", faceVertexCount);
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
                        CUBE_LOG(Info, ObjModelLoader, "No normal data found in obj shape '{0}'. Calculating normals.", objShape.name);
                        MeshHelper::SetNormalVector(ArrayView(vertices.begin() + vertexOffset, submeshVertexCount), ArrayView(indices.begin() + indexOffset, numIndices));
                    }
                    MeshHelper::CalculateTangentVectors(ArrayView(vertices.begin() + vertexOffset, submeshVertexCount), ArrayView(indices.begin() + indexOffset, numIndices));
                }
            }

            // Make scene object.
            SharedPtr<Mesh> mesh = std::make_shared<Mesh>(
                std::make_shared<MeshData>(vertices, indices, subMeshes, objFile),
                meshMetadata
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
} // namespace cube
