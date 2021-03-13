cmake_minimum_required (VERSION 3.6)

set(THIRD_LIB
    fmt/core.h
    fmt/format.cc
    fmt/format.h
    fmt/format-inl.h

    rapidjson/error/en.h
    rapidjson/error/error.h
    rapidjson/internal/biginteger.h
    rapidjson/internal/clzll.h
    rapidjson/internal/diyfp.h
    rapidjson/internal/dtoa.h
    rapidjson/internal/ieee754.h
    rapidjson/internal/itoa.h
    rapidjson/internal/meta.h
    rapidjson/internal/pow10.h
    rapidjson/internal/regex.h
    rapidjson/internal/stack.h
    rapidjson/internal/strfunc.h
    rapidjson/internal/strtod.h
    rapidjson/internal/swap.h
    rapidjson/msinttypes/inttypes.h
    rapidjson/msinttypes/stdint.h
    rapidjson/allocators.h
    rapidjson/cursorstreamwrapper.h
    rapidjson/document.h
    rapidjson/encodedstream.h
    rapidjson/filereadstream.h
    rapidjson/filewritestream.h
    rapidjson/fwd.h
    rapidjson/istreamwrapper.h
    rapidjson/memorybuffer.h
    rapidjson/memorystream.h
    rapidjson/ostreamwrapper.h
    rapidjson/pointer.h
    rapidjson/prettywriter.h
    rapidjson/rapidjson.h
    rapidjson/reader.h
    rapidjson/schema.h
    rapidjson/stream.h
    rapidjson/stringbuffer.h
    rapidjson/writer.h
)

set(INCLUDE
    MatrixImpl/MatrixArray.h
    MatrixImpl/MatrixSSE.h
    MatrixImpl/MatrixUtilityArray.h
    MatrixImpl/MatrixUtilitySSE.h
    VectorImpl/VectorArray.h
    VectorImpl/VectorSSE.h
    Event.h
    Flags.h
    Format.h
    Json.h
    KeyCode.h
    Math.h
    Matrix.h
    MatrixUtility.h
    String.h
    Types.h
    Vector.h
)

set(SOURCE
    CubeFormat.cpp
    Math.cpp
    String.cpp
)

# Target
add_library(CE-Utility STATIC ${SOURCE} ${INCLUDE} ${THIRD_LIB})
