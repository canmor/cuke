
#ifndef CUKE_UNICODE_H_
#define CUKE_UNICODE_H_

#include <locale>
#include <codecvt>

namespace cuke {
    static std::string to_utf8(std::wstring_view ws) {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
        return utf8_conv.to_bytes(ws.data());
    }

    static std::wstring to_wstring(std::string_view u8) {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        return converter.from_bytes(u8.data());
    }
}

#endif //CUKE_UNICODE_H_
