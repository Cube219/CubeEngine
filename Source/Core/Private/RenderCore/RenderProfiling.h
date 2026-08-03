#pragma once

#include "CoreHeader.h"

#include "GAPI_CommandList.h"

namespace cube
{
	class GPUEventScope
	{
	public:
		GPUEventScope(gapi::CommandList& commandList, StringView name)
		    : mCurrentCommandList(commandList)
		{
		    mCurrentCommandList.BeginEvent(name);
		}
		~GPUEventScope()
		{
		    mCurrentCommandList.EndEvent();
		}

		GPUEventScope(const GPUEventScope& other) = delete;
		GPUEventScope& operator=(const GPUEventScope& rhs) = delete;

	private:
		gapi::CommandList& mCurrentCommandList;
	};

#define GPU_EVENT_SCOPE(commandList, name) GPUEventScope CUBE_MACRO_JOIN(_eventScope, __LINE__)(commandList, name)
} // namespace cube
