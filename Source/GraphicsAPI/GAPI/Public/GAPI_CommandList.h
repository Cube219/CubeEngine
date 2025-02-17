#pragma once

#include "GAPIHeader.h"

namespace cube
{
    namespace gapi
    {
        struct CommandListCreateInfo
        {
        };

        class CommandList
        {
        public:
            CommandList() = default;
            virtual ~CommandList() = default;
        };
    } // namespace gapi
} // namespace cube
