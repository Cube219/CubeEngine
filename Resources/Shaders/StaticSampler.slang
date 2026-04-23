module StaticSampler;

#if CUBE_SLANG_HLSL

SamplerState StaticPointClampSampler : register(s1000);
SamplerState StaticPointWrapSampler : register(s1001);
SamplerState StaticLinearClampSampler : register(s1002);
SamplerState StaticLinearWrapSampler : register(s1003);

public SamplerState GetStaticPointClampSampler()
{
    return StaticPointClampSampler;
}

public SamplerState GetStaticPointWrapSampler()
{
    return StaticPointWrapSampler;
}

public SamplerState GetStaticLinearClampSampler()
{
    return StaticLinearClampSampler;
}

public SamplerState GetStaticLinearWrapSampler()
{
    return StaticLinearWrapSampler;
}

#elif CUBE_SLANG_METAL

public SamplerState GetStaticPointClampSampler()
{
    __requirePrelude(R"(
        sampler GetStaticPointClampSamplerImpl()
        {
            constexpr sampler staticPointClampSampler(
                mag_filter::nearest,
                min_filter::nearest,
                mip_filter::nearest,
                address::clamp_to_edge
            );
            return staticPointClampSampler;
        }
    )");
    __intrinsic_asm "GetStaticPointClampSamplerImpl()";
}

public SamplerState GetStaticPointWrapSampler()
{
    __requirePrelude(R"(
        sampler GetStaticPointWrapSamplerImpl()
        {
            constexpr sampler staticPointWrapSampler(
                mag_filter::nearest,
                min_filter::nearest,
                mip_filter::nearest,
                address::repeat
            );
            return staticPointWrapSampler;
        }
    )");
    __intrinsic_asm "GetStaticPointWrapSamplerImpl()";
}

public SamplerState GetStaticLinearClampSampler()
{
    __requirePrelude(R"(
        sampler GetStaticLinearClampSamplerImpl()
        {
            constexpr sampler staticLinearClampSampler(
                mag_filter::linear,
                min_filter::linear,
                mip_filter::nearest,
                address::clamp_to_edge
            );
            return staticLinearClampSampler;
        }
    )");
    __intrinsic_asm "GetStaticLinearClampSamplerImpl()";
}

public SamplerState GetStaticLinearWrapSampler()
{
    __requirePrelude(R"(
        sampler GetStaticLinearWrapSamplerImpl()
        {
            constexpr sampler staticLinearWrapSampler(
                mag_filter::linear,
                min_filter::linear,
                mip_filter::nearest,
                address::repeat
            );
            return staticLinearWrapSampler;
        }
    )");
    __intrinsic_asm "GetStaticLinearWrapSamplerImpl()";
}

#else

#endif
