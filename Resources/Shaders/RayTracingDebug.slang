import Common;
import Global;

ParameterBlock<GlobalShaderParameterList> globalParams;

struct RayTracingDebugViewShaderParameterList
{
    float4 bufferSizeAndInvSize;
    Bindless<RaytracingAccelerationStructure> tlas;
    Bindless<RWTexture2D> outColorTexture;
};

[shader("compute")]
[numthreads(8, 8, 1)]
void RayTracingDebugViewCS(
    uint3 threadId : SV_DispatchThreadID,
    ParameterBlock<RayTracingDebugViewShaderParameterList> params
)
{
    const uint2 bufferPixel = threadId.xy;
    const float2 bufferUV = (float2(bufferPixel) + 0.5f) * params.bufferSizeAndInvSize.zw;
    const float2 ndc = (bufferUV - 0.5f) * float2(2.0f, -2.0f);

    float4 rayStartNDC = float4(ndc.xy, 0.0f, 1.0f);
    float4 rayEndNDC = float4(ndc.xy, 1.0f, 1.0f);

    float4 rayStartPosH = mul(rayStartNDC, globalParams.clipToWorld);
    float4 rayEndPosH = mul(rayEndNDC, globalParams.clipToWorld);

    float3 rayStartPos = rayStartPosH.xyz / rayStartPosH.w;
    float3 rayEndPos = rayEndPosH.xyz / rayEndPosH.w;

    RayDesc ray;
    ray.Origin = rayStartPos;
    ray.Direction = normalize(rayEndPos - rayStartPos);
    ray.TMin = 0.001f;
    ray.TMax = length(rayEndPos - rayStartPos);

    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;
    rayQuery.TraceRayInline(params.tlas, 0, 0xFF, ray);
    rayQuery.Proceed();
    if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        params.outColorTexture[bufferPixel] = { 1.0f, 0.0f, 0.0f, 1.0f };
    }
    else
    {
        params.outColorTexture[bufferPixel] = { 0.0f, 0.0f, 0.0f, 1.0f };
    }
}
