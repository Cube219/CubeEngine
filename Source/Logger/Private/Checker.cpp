#include "Checker.h"

#include "Logger.h"

namespace cube
{
#ifdef CUBE_USE_CHECK
    Vector<UniquePtr<ICheckerExtension>> Checker::mExtensions;

    void Checker::ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg)
    {
        for (auto& extension : mExtensions)
        {
            extension->ProcessFailedCheck(fullFileName, lineNum, exprAndMsg);
        }
    }
#endif // CUBE_USE_CHECK
} // namespace cube
