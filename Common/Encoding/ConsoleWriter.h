// Header-only helper for writing UTF-8/local text to console.
// On Windows, call InitConsole() once at program start to switch the
// console code page to UTF-8. If that succeeds, ConsolePrintf / ConsoleWrite
// emit the raw UTF-8 bytes directly. If InitConsole() is called but the
// code-page switch fails, bytes that look like valid UTF-8 are converted
// to the local ANSI code page before being written, so that Chinese and
// other non-ASCII characters still display correctly in legacy consoles.
// If InitConsole() is NEVER called (e.g. when this library is embedded in
// a Node.js addon, Python extension, GUI process, etc.), the writers emit
// raw bytes without any conversion and let the host decide how to interpret
// them. On non-Windows platforms all calls simply pass through.
#ifndef COMMON_ENCODING_CONSOLE_WRITER_H
#define COMMON_ENCODING_CONSOLE_WRITER_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace console_writer {

    // Internal tri-state flag describing how writers should handle bytes:
    //   0  = uninitialized (InitConsole not called) -> emit raw bytes
    //   1  = InitConsole succeeded switching console to UTF-8 -> emit raw UTF-8
    //  -1  = InitConsole was called but UTF-8 switch failed  -> AutoToLocal
    inline int& _ConsoleMode()
    {
        static int s_mode = 0;
        return s_mode;
    }

    // Validate whether the given byte sequence is well-formed UTF-8.
    inline bool IsValidUtf8(const char* s, size_t len)
    {
        if (s == 0)
            return false;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
        size_t i = 0;
        while (i < len) {
            unsigned char c = p[i];
            if (c < 0x80) {
                i += 1;
                continue;
            }
            size_t need = 0;
            unsigned int cp = 0;
            if ((c & 0xE0) == 0xC0) {
                need = 1;
                cp = c & 0x1F;
                if (c < 0xC2)
                    return false;
            }
            else if ((c & 0xF0) == 0xE0) {
                need = 2;
                cp = c & 0x0F;
            }
            else if ((c & 0xF8) == 0xF0) {
                need = 3;
                cp = c & 0x07;
            }
            else {
                return false;
            }
            if (i + need >= len)
                return false;
            for (size_t k = 1; k <= need; ++k) {
                unsigned char cc = p[i + k];
                if ((cc & 0xC0) != 0x80)
                    return false;
                cp = (cp << 6) | (cc & 0x3F);
            }
            if (need == 2 && cp < 0x800)
                return false;
            if (need == 3 && cp < 0x10000)
                return false;
            if (cp > 0x10FFFF)
                return false;
            if (cp >= 0xD800 && cp <= 0xDFFF)
                return false;
            i += need + 1;
        }
        return true;
    }

    // Convert UTF-8 bytes to the Windows local ANSI code page.
    // On non-Windows platforms returns the original bytes.
    inline std::string Utf8ToLocal(const char* s, size_t len)
    {
#if defined(_WIN32) || defined(__WINDOWS__)
        if (s == 0 || len == 0)
            return std::string();
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(len), 0, 0);
        if (wlen <= 0)
            return std::string(s, len);
        std::vector<wchar_t> wbuf(static_cast<size_t>(wlen));
        MultiByteToWideChar(CP_UTF8, 0, s, static_cast<int>(len), &wbuf[0], wlen);
        int alen = WideCharToMultiByte(CP_ACP, 0, &wbuf[0], wlen, 0, 0, 0, 0);
        if (alen <= 0)
            return std::string(s, len);
        std::string out(static_cast<size_t>(alen), '\0');
        WideCharToMultiByte(CP_ACP, 0, &wbuf[0], wlen, &out[0], alen, 0, 0);
        return out;
#else
        return std::string(s ? s : "", len);
#endif
    }

    // Auto-detect: if input is valid UTF-8 convert to local ANSI,
    // otherwise return the bytes unchanged.
    inline std::string AutoToLocal(const char* s, size_t len)
    {
#if defined(_WIN32) || defined(__WINDOWS__)
        if (s == 0)
            return std::string();
        if (IsValidUtf8(s, len))
            return Utf8ToLocal(s, len);
        return std::string(s, len);
#else
        return std::string(s ? s : "", len);
#endif
    }

    // Initialize console. On Windows attempt to set code pages to UTF-8
    // so that raw UTF-8 byte output shows correctly; record the result
    // in the internal mode flag. On non-Windows platforms this is a no-op.
    // MUST be called explicitly by EXE hosts; library-only hosts that do
    // NOT call this will get raw byte pass-through (no conversion).
    inline void InitConsole()
    {
#if defined(_WIN32) || defined(__WINDOWS__)
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        // Use the real console code page rather than SetConsoleOutputCP's
        // return value: some hosts (e.g. Node.js) report success but the
        // actual console code page stays unchanged. Falling back to
        // AutoToLocal keeps Chinese readable on legacy GBK consoles.
        UINT cp = GetConsoleOutputCP();
        _ConsoleMode() = (cp == CP_UTF8) ? 1 : -1;
#endif
    }

    // Write a buffer of length len to stdout with auto encoding handling.
    inline void ConsoleWriteN(const char* s, size_t len)
    {
        if (s == 0 || len == 0)
            return;
#if defined(_WIN32) || defined(__WINDOWS__)
        int mode = _ConsoleMode();
        if (mode == -1) {
            std::string out = AutoToLocal(s, len);
            fwrite(out.data(), 1, out.size(), stdout);
        }
        else {
            fwrite(s, 1, len, stdout);
        }
#else
        fwrite(s, 1, len, stdout);
#endif
    }

    // Write a null-terminated string to stdout with auto encoding handling.
    inline void ConsoleWrite(const char* s)
    {
        if (s == 0)
            return;
        ConsoleWriteN(s, strlen(s));
    }

    // printf-like writer with auto encoding handling.
    inline int ConsolePrintf(const char* fmt, ...)
    {
        if (fmt == 0)
            return 0;
        char stackBuf[2048];
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(stackBuf, sizeof(stackBuf), fmt, ap);
        va_end(ap);
        if (n < 0)
            return n;
        if (static_cast<size_t>(n) < sizeof(stackBuf)) {
            ConsoleWriteN(stackBuf, static_cast<size_t>(n));
            return n;
        }
        std::vector<char> heap(static_cast<size_t>(n) + 1);
        va_start(ap, fmt);
        int n2 = vsnprintf(&heap[0], heap.size(), fmt, ap);
        va_end(ap);
        if (n2 < 0)
            return n2;
        ConsoleWriteN(&heap[0], static_cast<size_t>(n2));
        return n2;
    }

    // fprintf-like writer that only wraps console streams (stdout/stderr).
    // For other FILE* it behaves like normal vfprintf with no conversion.
    inline int ConsoleFprintf(FILE* fp, const char* fmt, ...)
    {
        if (fp == 0 || fmt == 0)
            return 0;
        va_list ap;
        if (fp != stdout && fp != stderr) {
            va_start(ap, fmt);
            int n = vfprintf(fp, fmt, ap);
            va_end(ap);
            return n;
        }
        char stackBuf[2048];
        va_start(ap, fmt);
        int n = vsnprintf(stackBuf, sizeof(stackBuf), fmt, ap);
        va_end(ap);
        if (n < 0)
            return n;
        const char* data = stackBuf;
        std::vector<char> heap;
        if (static_cast<size_t>(n) >= sizeof(stackBuf)) {
            heap.resize(static_cast<size_t>(n) + 1);
            va_start(ap, fmt);
            int n2 = vsnprintf(&heap[0], heap.size(), fmt, ap);
            va_end(ap);
            if (n2 < 0)
                return n2;
            n = n2;
            data = &heap[0];
        }
#if defined(_WIN32) || defined(__WINDOWS__)
        int mode = _ConsoleMode();
        if (mode == -1) {
            std::string out = AutoToLocal(data, static_cast<size_t>(n));
            fwrite(out.data(), 1, out.size(), fp);
        }
        else {
            fwrite(data, 1, static_cast<size_t>(n), fp);
        }
#else
        fwrite(data, 1, static_cast<size_t>(n), fp);
#endif
        return n;
    }

} // namespace console_writer

#endif // COMMON_ENCODING_CONSOLE_WRITER_H
