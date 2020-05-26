#pragma once

namespace cube
{
    namespace rapi
    {
        class RenderAPI
        {
        public:
            RenderAPI() {}
            virtual ~RenderAPI() {}

            virtual void Initialize() = 0;
            virtual void Shutdown() = 0;
        };
    } // namespace rapi
} // namespace cube
