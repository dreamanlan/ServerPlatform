// Header-only helper for normalizing text file buffers to UTF-8.
//
// EnsureUtf8(buffer, size) takes a heap-allocated buffer (allocated with
// operator new[]) containing the raw bytes of a text file and converts it
// to UTF-8 in place (or reallocates if a wider encoding must be decoded).
//
// Detection order:
//   1. UTF-8 BOM (EF BB BF)       -> strip BOM, keep bytes.
//   2. UTF-16 LE BOM (FF FE)      -> decode to UTF-8, reallocate buffer.
//   3. UTF-16 BE BOM (FE FF)      -> decode to UTF-8, reallocate buffer.
//   4. No BOM, valid UTF-8 bytes  -> keep bytes.
//   5. No BOM, invalid UTF-8      -> convert from system local ANSI / locale
//                                    encoding to UTF-8, reallocate buffer.
//
// On success the buffer is guaranteed to be null-terminated UTF-8 bytes
// and `size` holds the new byte length (not counting the terminator).
// If a conversion fails the original buffer is preserved and the function
// returns false so callers can fall back to their own handling.
//
// Platform notes:
//   - Windows uses MultiByteToWideChar / WideCharToMultiByte.
//   - Linux and macOS use iconv with the encoding name reported by
//     nl_langinfo(CODESET). macOS requires linking -liconv.
#ifndef COMMON_ENCODING_ENCODING_UTIL_H
#define COMMON_ENCODING_ENCODING_UTIL_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#if defined(_WIN32) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#include <locale.h>
#include <errno.h>
#endif

namespace encoding_util {

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

    // Convert UTF-16 (LE or BE) byte buffer to a newly allocated UTF-8 buffer.
    // The returned buffer is allocated with operator new[] and null-terminated.
    // `outSize` receives the UTF-8 byte length (not counting the terminator).
    // Returns NULL on failure.
    inline char* Utf16ToUtf8New(const char* src, size_t srcBytes, bool bigEndian, long& outSize)
    {
        outSize = 0;
        if (src == 0 || srcBytes < 2)
            return 0;
        size_t units = srcBytes / 2;
#if defined(_WIN32) || defined(__WINDOWS__)
        wchar_t* wbuf = new wchar_t[units + 1];
        if (wbuf == 0)
            return 0;
        const unsigned char* u = reinterpret_cast<const unsigned char*>(src);
        for (size_t i = 0; i < units; ++i) {
            unsigned char b0 = u[i * 2];
            unsigned char b1 = u[i * 2 + 1];
            wbuf[i] = bigEndian
                ? static_cast<wchar_t>((b0 << 8) | b1)
                : static_cast<wchar_t>((b1 << 8) | b0);
        }
        wbuf[units] = 0;
        int need = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int>(units), 0, 0, 0, 0);
        if (need <= 0) {
            delete[] wbuf;
            return 0;
        }
        char* out = new char[need + 1];
        if (out == 0) {
            delete[] wbuf;
            return 0;
        }
        int got = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int>(units), out, need, 0, 0);
        delete[] wbuf;
        if (got <= 0) {
            delete[] out;
            return 0;
        }
        out[got] = 0;
        outSize = got;
        return out;
#else
        const char* fromCode = bigEndian ? "UTF-16BE" : "UTF-16LE";
        iconv_t cd = iconv_open("UTF-8", fromCode);
        if (cd == (iconv_t)-1)
            return 0;
        size_t cap = srcBytes * 2 + 4;
        char* out = new char[cap + 1];
        if (out == 0) {
            iconv_close(cd);
            return 0;
        }
        char* inPtr = const_cast<char*>(src);
        size_t inLeft = srcBytes;
        char* outPtr = out;
        size_t outLeft = cap;
        while (inLeft > 0) {
            size_t r = iconv(cd, &inPtr, &inLeft, &outPtr, &outLeft);
            if (r == (size_t)-1) {
                if (errno == E2BIG) {
                    size_t used = outPtr - out;
                    size_t newCap = cap * 2 + 4;
                    char* bigger = new char[newCap + 1];
                    if (bigger == 0) {
                        delete[] out;
                        iconv_close(cd);
                        return 0;
                    }
                    memcpy(bigger, out, used);
                    delete[] out;
                    out = bigger;
                    outPtr = out + used;
                    outLeft = newCap - used;
                    cap = newCap;
                    continue;
                }
                delete[] out;
                iconv_close(cd);
                return 0;
            }
        }
        iconv_close(cd);
        size_t got = outPtr - out;
        out[got] = 0;
        outSize = static_cast<long>(got);
        return out;
#endif
    }

    // Convert system local (Windows ANSI / POSIX nl_langinfo) bytes to a
    // newly allocated UTF-8 buffer. Returns NULL on failure.
    inline char* LocalToUtf8New(const char* src, size_t srcBytes, long& outSize)
    {
        outSize = 0;
        if (src == 0 || srcBytes == 0) {
            char* empty = new char[1];
            if (empty == 0)
                return 0;
            empty[0] = 0;
            outSize = 0;
            return empty;
        }
#if defined(_WIN32) || defined(__WINDOWS__)
        int wlen = MultiByteToWideChar(CP_ACP, 0, src, static_cast<int>(srcBytes), 0, 0);
        if (wlen <= 0)
            return 0;
        wchar_t* wbuf = new wchar_t[wlen];
        if (wbuf == 0)
            return 0;
        int gotW = MultiByteToWideChar(CP_ACP, 0, src, static_cast<int>(srcBytes), wbuf, wlen);
        if (gotW <= 0) {
            delete[] wbuf;
            return 0;
        }
        int need = WideCharToMultiByte(CP_UTF8, 0, wbuf, gotW, 0, 0, 0, 0);
        if (need <= 0) {
            delete[] wbuf;
            return 0;
        }
        char* out = new char[need + 1];
        if (out == 0) {
            delete[] wbuf;
            return 0;
        }
        int got = WideCharToMultiByte(CP_UTF8, 0, wbuf, gotW, out, need, 0, 0);
        delete[] wbuf;
        if (got <= 0) {
            delete[] out;
            return 0;
        }
        out[got] = 0;
        outSize = got;
        return out;
#else
        // Initialize locale once so nl_langinfo returns the real codeset
        // instead of the default "ANSI_X3.4-1968".
        static bool s_localeInited = false;
        if (!s_localeInited) {
            setlocale(LC_CTYPE, "");
            s_localeInited = true;
        }
        const char* codeset = nl_langinfo(CODESET);
        if (codeset == 0 || codeset[0] == 0)
            codeset = "UTF-8";
        // If the system locale is already UTF-8 there is nothing to do but
        // the caller has already determined the bytes are not valid UTF-8,
        // so fall through to iconv which will likely fail; that is fine.
        iconv_t cd = iconv_open("UTF-8", codeset);
        if (cd == (iconv_t)-1)
            return 0;
        size_t cap = srcBytes * 4 + 4;
        char* out = new char[cap + 1];
        if (out == 0) {
            iconv_close(cd);
            return 0;
        }
        char* inPtr = const_cast<char*>(src);
        size_t inLeft = srcBytes;
        char* outPtr = out;
        size_t outLeft = cap;
        while (inLeft > 0) {
            size_t r = iconv(cd, &inPtr, &inLeft, &outPtr, &outLeft);
            if (r == (size_t)-1) {
                if (errno == E2BIG) {
                    size_t used = outPtr - out;
                    size_t newCap = cap * 2 + 4;
                    char* bigger = new char[newCap + 1];
                    if (bigger == 0) {
                        delete[] out;
                        iconv_close(cd);
                        return 0;
                    }
                    memcpy(bigger, out, used);
                    delete[] out;
                    out = bigger;
                    outPtr = out + used;
                    outLeft = newCap - used;
                    cap = newCap;
                    continue;
                }
                delete[] out;
                iconv_close(cd);
                return 0;
            }
        }
        iconv_close(cd);
        size_t got = outPtr - out;
        out[got] = 0;
        outSize = static_cast<long>(got);
        return out;
#endif
    }

    // Normalize `buffer` so its contents are UTF-8 bytes. `buffer` must be a
    // pointer to a heap block allocated with operator new[] and holding at
    // least `size`+1 bytes (the extra byte is used as null terminator).
    // On reallocation the old buffer is freed with operator delete[] and
    // `buffer` is updated to point at the new heap block (also allocated
    // with operator new[]). `size` is updated to the new byte length.
    // Returns true on success, false if conversion failed (buffer unchanged).
    inline bool EnsureUtf8(char*& buffer, long& size)
    {
        if (buffer == 0 || size < 0)
            return false;
        // UTF-8 BOM: strip in place.
        if (size >= 3 &&
            static_cast<unsigned char>(buffer[0]) == 0xEF &&
            static_cast<unsigned char>(buffer[1]) == 0xBB &&
            static_cast<unsigned char>(buffer[2]) == 0xBF) {
            memmove(buffer, buffer + 3, static_cast<size_t>(size - 3) + 1);
            size -= 3;
            return true;
        }
        // UTF-16 LE BOM.
        if (size >= 2 &&
            static_cast<unsigned char>(buffer[0]) == 0xFF &&
            static_cast<unsigned char>(buffer[1]) == 0xFE) {
            long outSize = 0;
            char* converted = Utf16ToUtf8New(buffer + 2, static_cast<size_t>(size - 2), false, outSize);
            if (converted == 0)
                return false;
            delete[] buffer;
            buffer = converted;
            size = outSize;
            return true;
        }
        // UTF-16 BE BOM.
        if (size >= 2 &&
            static_cast<unsigned char>(buffer[0]) == 0xFE &&
            static_cast<unsigned char>(buffer[1]) == 0xFF) {
            long outSize = 0;
            char* converted = Utf16ToUtf8New(buffer + 2, static_cast<size_t>(size - 2), true, outSize);
            if (converted == 0)
                return false;
            delete[] buffer;
            buffer = converted;
            size = outSize;
            return true;
        }
        // No BOM: keep if already valid UTF-8.
        if (IsValidUtf8(buffer, static_cast<size_t>(size)))
            return true;
        // Otherwise treat as system local encoding and convert.
        long outSize = 0;
        char* converted = LocalToUtf8New(buffer, static_cast<size_t>(size), outSize);
        if (converted == 0)
            return false;
        delete[] buffer;
        buffer = converted;
        size = outSize;
        return true;
    }

} // namespace encoding_util

#endif // COMMON_ENCODING_ENCODING_UTIL_H
