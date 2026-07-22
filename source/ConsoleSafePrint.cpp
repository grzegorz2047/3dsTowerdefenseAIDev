#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {

constexpr int kPhysicalConsoleWidth = 30;

const char* compactKnownLine(const char* text) {
    if (text == nullptr) return "";
    if (std::strcmp(text, "D-PAD WYBOR  A GRAJ  START WYJSCIE") == 0) {
        return "D-PAD wybor A graj START wyj";
    }
    return text;
}

}  // namespace

namespace std {

int citadelConsolePrintf(const char* format, ...) {
    va_list arguments;
    va_start(arguments, format);

    int result = 0;
    if (std::strcmp(format, "\x1b[%d;1H%-39.39s") == 0) {
        const int row = va_arg(arguments, int);
        const char* text = compactKnownLine(va_arg(arguments, const char*));
        result = ::printf("\x1b[%d;1H%-30.30s", row, text);
    } else if (std::strcmp(format, "\x1b[%d;1H\x1b[36m%-39.39s\x1b[0m") == 0) {
        const int row = va_arg(arguments, int);
        const char* title = va_arg(arguments, const char*);
        result = ::printf("\x1b[%d;1H\x1b[36m%-30.30s\x1b[0m", row, title != nullptr ? title : "");
    } else {
        result = ::vprintf(format, arguments);
    }

    va_end(arguments);
    return result;
}

}  // namespace std
