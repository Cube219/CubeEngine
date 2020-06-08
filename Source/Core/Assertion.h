#pragma once

#ifdef _DEBUG

#include <exception>

#include "LogWriter.h"
#include "Platform/PlatformDebug.h"
#include "Utility/Format.h"
#include "Allocator/FrameAllocator.h"

namespace cube
{
	template <typename ...Args>
	void AssertionFailedFormatting(const char* fileName, int lineNum, StringView msg, Args&&... args)
	{
		FrameString str = FrameFormat(msg.data(), std::forward<Args>(args)...);
		LogWriter::WriteLog(LogType::Error, fileName, lineNum, str);
		platform::PlatformDebug::AssertionFailed(str, nullptr, fileName, lineNum);
	}
} // namespace cube

#define ASSERTION_FAILED(msg, ...)                                                  \
	cube::AssertionFailedFormatting(__FILE__, __LINE__, CUBE_T(msg), ##__VA_ARGS__);

#define CHECK(expr, msg, ...)                \
	if(!(expr)){                             \
		ASSERTION_FAILED(msg, ##__VA_ARGS__) \
	}

#define GLOBAL_TRY_CATCH_BEGIN try {

#define GLOBAL_TRY_CATCH_END                               \
} catch(const std::exception& e) {                         \
    ASSERTION_FAILED("An exception occured. {}", e.what()) \
}

#else // _DEBUG

#define ASSERTION_FAILED(msg, ...)
#define CHECK(expr, msg, ...)

#define GLOBAL_TRY_CATCH_BEGIN

#define GLOBAL_TRY_CATCH_END

#endif // _DEBUG
