#pragma once

#include "CoreHeader.h"

#include "Event.h"
#include "GAPI.h"

namespace cube
{
    class Material;
    class MeshData;
    struct MeshMetadata;
    class Renderer;

    class Engine
    {
    public:
        Engine() = delete;
        ~Engine() = delete;

        struct EngineInitializeInfo
        {
            int argc;
            const char** argv;
            GAPIName gapi;
            bool drawImGUI = true;
            // Some platform cannot execute loop logic in the main thread. (Ex. MacOS)
            // So, run initialize and shutdown logic in platform loop function which runs another thread.
            bool runInitializeAndShutdownInLoopFunction = false;
        };
        CUBE_CORE_EXPORT static void Initialize(const EngineInitializeInfo& initInfo);
        CUBE_CORE_EXPORT static void StartLoop();
        CUBE_CORE_EXPORT static void Shutdown();

        static Renderer* GetRenderer() { return mRenderer.get(); }

        CUBE_CORE_EXPORT static const String& GetRootDirectoryPath() { return mRootDirectoryPath; }
        CUBE_CORE_EXPORT static const String& GetShaderDirectoryPath() { return mShaderDirectoryPath; }

        static void SetMesh(SharedPtr<MeshData> meshData, const MeshMetadata& meshMeta);
        static void SetMaterials(const Vector<SharedPtr<Material>>& materials);

    private:
        static void OnLoop();
        static void OnClosing();
        static void OnResize(Uint32 width, Uint32 height);

        static void LoopImGUI();

        static Uint64 GetNow();

        static void SearchAndSetRootDirectory();
        static void SetOtherDirectories();

        static EventFunction<void()> mOnLoopEventFunc;
        static EventFunction<void()> mOnClosingEventFunc;
        static EventFunction<void(Uint32, Uint32)> mOnResizeEventFunc;

        static UniquePtr<Renderer> mRenderer;
        static bool mDrawImGUI;

        static ImGUIContext mImGUIContext;
        static bool mImGUIShowDemoWindow;

        static String mRootDirectoryPath;
        static String mShaderDirectoryPath;

        static Uint64 mStartTime;
        static Uint64 mLastTime;
        static Uint64 mCurrentTime;
    };
} // namespace cube
