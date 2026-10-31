module RayTracingCommon;

#if CUBE_SLANG_METAL

namespace internal
{
    public struct MetalRayTracingInfo
    {
        public uint3 dispatchRaysIndex;
        public uint3 dispatchRaysDimensions;

        public uint hitGroupBaseIndex;
        public uint missShaderBaseIndex;

        public __init(uint threadIdX, uint3 dispatchRaysDimensions, uint hitGroupBaseIndex, uint missShaderBaseIndex)
        {
            this.dispatchRaysDimensions = dispatchRaysDimensions;

            dispatchRaysIndex.z = threadIdX / (dispatchRaysDimension.x * dispatchRaysDimension.y);
            dispatchRaysIndex.y = threadIdX % (dispatchRaysDimension.x * dispatchRaysDimension.y) / dispatchRaysDimension.x;
            dispatchRaysIndex.x = threadIdX % dispatchRaysDimension.x;

            this.hitGroupBaseIndex = hitGroupBaseIndex;
            this.missShaderBaseIndex = missShaderBaseIndex;
        }

        public bool IsOutOfBounds()
        {
            return dispatchRaysIndex.z >= dispatchRaysDimensions.z;
        }
    };

    __target_intrinsic(metal, "visible_function_table<void(void*, void*, void*)>")
    struct MetalHitGroupVisibleFunctionTable
    {
        public void GetFunction(uint index)
        {
            __intrinsic_asm ".operator[]";
        }

        public void CallFunction(uint index)
        {
        __requirePrelude(R"

        ");
        }
    };

    __target_intrinsic(metal, "visible_function_table<void(void*, void*)>")
    struct MetalMissShaderVisibleFunctionTable
    {
        public void GetFunction(uint index)
        {
            __intrinsic_asm ".operator[]";
        }
    };

    public void TraceRayImpl<payload_t>(
        MetalRayTracingInfo metalRayTracingInfo,
        MetalHitGroupVisibleFunctionTable metalHitGroupVisibleFunctionTable,
        MetalMissShaderVisibleFunctionTable metalMissShaderVisibleFunctionTable,

        RaytracingAccelerationStructure accelerationStructure,
        uint rayFlags,
        uint instanceInclusionMask,
        uint rayContributionToHitGroupIndex,
        uint multiplierForGeometryContributionToHitGroupIndex,
        uint missShaderIndex,
        RayDesc rayDesc,
        inout payload_t payLoad)
    {
        // TODO: Support any hit shader.
        RayQuery<RAY_FLAG_NONE> rayQuery;
        rayQuery.TraceRayInline(params.tlas, rayFlags, instanceInclusionMask, ray);
        rayQuery.Proceed();
        if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
        {
            const uint hitGroupIndex = instanceContributionToHitGroupIndex
                + rayQuery.CommittedGeometryIndex() * multiplierForGeometryContributionToHitGroupIndex
                + rayContributionToHitGroupIndex;

            __intrinsic_asm R"
            ";
        }
        else if (rayQuery.CommittedStatus() == COMMITTED_NOTHING)
        {
        }
    }
}

#define CUBE_RAY_TRACING_RAY_GEN_ENTRY(name, paramsType, paramsName) \
    [shader("compute")] \
    [numthreads(64, 1, 1)] \
    void name(uint3 threadId : SV_DispatchThreadID, \
        paramsType paramsName, \
        MetalHitGroupVisibleFunctionTable internal_hitGroupVisibleFunctionTable,
        MetalMissShaderVisibleFunctionTable internal_missShaderVisibleFunctionTable,
        uniform uint3 internal_DispatchRaysDimensions,
        uniform uint internal_hitGroupBaseIndex,
        uniform uint internal_missShaderBaseIndex)

#define CUBE_RAY_TRACING_RAY_GEN_INIT() \
    internal::MetalRayTracingInfo internal_metalRayTracingInfo = internal::MetalRayTracingInfo( \
        threadId.x, internal_DispatchRaysDimensions, \
        internal_hitGroupBaseIndex, internal_missShaderBaseIndex); \
    if (internal_metalRayTracingInfo.IsOutOfBounds()) return;

#define CUBE_RAY_TRACING_CLOSEST_HIT_ENTRY(name, payloadType, payloadName, attributeType, attributeName) \
    void name(inout payLoadType payloadName, attributeType attributeName, internal::MetalRayTracingInfo internal_metalRayTracingInfo)

#define CUBE_RAY_TRACING_ANY_HIT_ENTRY(name, payloadType, payloadName, attributeType, attributeName) \
    void name(inout payLoadType payloadName, attributeType attributeName, internal::MetalRayTracingInfo internal_metalRayTracingInfo)

#define CUBE_RAY_TRACING_MISS_ENTRY(name, payloadType, payloadName) \
    void name(inout payLoadType payloadName, internal::MetalRayTracingInfo internal_metalRayTracingInfo)

// Functions used in ray tracing shaders

#define DispatchRaysIndex() internal_metalRayTracingInfo.dispatchRaysIndex
#define DispatchRaysDimensions() internal_metalRayTracingInfo.dispatchDimensions
#define TraceRay(accelerationStructure, rayFlags, instanceInclusionMask, rayContributionToHitGroupIndex, multiplierForGeometryContributionToHitGroupIndex, missShaderIndex, ray, payload) \
    internal::TraceRayImpl( \
        internal_metalRayTracingInfo, internal_hitGroupVisibleFunctionTable, internal_missShaderVisibleFunctionTable, \
        accelerationStructure, rayFlags, instanceInclusionMask, rayContributionToHitGroupIndex, multiplierForGeometryContributionToHitGroupIndex, missShaderIndex, ray, payload)

#else

#define CUBE_RAY_TRACING_RAY_GEN_ENTRY(name, paramsType, paramsName) \
    [shader("raygeneration")] \
    void name(paramsType, paramsName)

#define CUBE_RAY_TRACING_RAY_GEN_INIT()

#define CUBE_RAY_TRACING_CLOSEST_HIT_ENTRY(name, payloadType, payloadName, attributeType, attributeName) \
    [shader("closesthit")] \
    void name(inout payLoadType payloadName, attributeType attributeName)

#define CUBE_RAY_TRACING_ANY_HIT_ENTRY(name, payloadType, payloadName, attributeType, attributeName) \
    [shader("anyhit")] \
    void name(inout payLoadType payloadName, attributeType attributeName)

#define CUBE_RAY_TRACING_ANY_HIT_END()

#define CUBE_RAY_TRACING_MISS_ENTRY(name, payloadType, payloadName) \
    [shader("miss")] \
    void name(inout payLoadType payloadName)

#endif
