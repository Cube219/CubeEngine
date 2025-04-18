if(CUBE_FETCH_GLTF_SAMPLES)
    message(STATUS "CubeEngine: Fetching glTF sample assets...")

    include(FetchContent)
    FetchContent_Declare(
        gltfSampleAssets
        GIT_REPOSITORY https://github.com/KhronosGroup/glTF-Sample-Assets
        GIT_TAG        68ce9c54868923879ed2615a772795ee64a999e1
        GIT_SHALLOW TRUE
        SOURCE_DIR ${RESOURCES_DIR}/Models/glTFSampleAssets
    )
    FetchContent_MakeAvailable(gltfSampleAssets)

    message(STATUS "CubeEngine: glTF sample assets fetch complete.")
endif()
