#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {

constexpr int kSafeConsoleWidth = 30;
char gLastTitle[64]{};
bool gClearRequested = false;

}  // namespace

namespace std {

int citadelConsolePrintf(const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);
    int result = 0;

    if (std::strcmp(format, "\x1b[2J\x1b[1;1H") == 0) {
        gClearRequested = true;
        va_end(arguments);
        return 0;
    }

    if (std::strcmp(format, "\x1b[%d;1H\x1b[36m%-39.39s\x1b[0m") == 0) {
        const int row = va_arg(arguments, int);
        const char* title = va_arg(arguments, const char*);
        const char* safeTitle = title != nullptr ? title : "";
        if (gClearRequested && std::strncmp(gLastTitle, safeTitle, sizeof(gLastTitle) - 1U) != 0) {
            ::printf("\x1b[2J\x1b[1;1H");
            std::snprintf(gLastTitle, sizeof(gLastTitle), "%s", safeTitle);
        }
        gClearRequested = false;
        result = ::printf("\x1b[%d;1H\x1b[36m%-30.30s\x1b[0m", row, safeTitle);
        va_end(arguments);
        return result;
    }

    if (std::strcmp(format, "\x1b[%d;1H%-39.39s") == 0) {
        const int row = va_arg(arguments, int);
        const char* text = va_arg(arguments, const char*);
        result = ::printf("\x1b[%d;1H%-30.30s", row, text != nullptr ? text : "");
        va_end(arguments);
        return result;
    }

    result = ::vprintf(format, arguments);
    va_end(arguments);
    return result;
}

}  // namespace std
