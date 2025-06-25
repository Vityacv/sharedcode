#include "pch.h"
#include <cstdint>
#include "shared/string_utils.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef WIN32
StringUtil& StringUtil::instance() {
    thread_local StringUtil util;
    return util;
}

int StringUtil::toNative(const char *s, wchar_t *m, size_t sz) {

    return MultiByteToWideChar(CP_UTF8, 0, s, -1, m, static_cast<int>(sz));
}

const wchar_t* StringUtil::toNative(const char *s)
{
    size_t srcSize = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if(srcSize < convertBufferSize)
        MultiByteToWideChar(CP_UTF8, 0, s, -1, m_buffer, static_cast<int>(srcSize));
    return static_cast<wchar_t*>(m_buffer);
}

int StringUtil::fromNative(const wchar_t *s, char *m, size_t sz)
{
    return WideCharToMultiByte(CP_UTF8, 0, s, -1, m, static_cast<int>(sz), nullptr, nullptr);
}

const char* StringUtil::fromNative(const wchar_t *s)
{
    size_t srcSize = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
    if(srcSize < convertBufferSize)
        WideCharToMultiByte(CP_UTF8, 0, s, -1, reinterpret_cast<char*>(m_buffer), static_cast<int>(srcSize), nullptr, nullptr);
    return reinterpret_cast<char*>(m_buffer);
}

char * regcall StringUtil::copyFromNative(const wchar_t *wb, char *mb, size_t sz)
{
    size_t i = 0;
    do {
        i++;
        *(char *)((char *)mb + i) = *(char *)(wb + i);
    } while (i != sz);
    return mb+sz;
}

size_t regcall StringUtil::getUtf8Size(char *s)
{
    return MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
}

size_t regcall StringUtil::getUtf16Size(wchar_t *s)
{
    return WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
}

wchar_t * StringUtil::copyToNative(const char *mb, wchar_t *wb, size_t sz)
{
    int i = sz;
//    do {
//        i++;
//    } while (mb[i]);
    do {
        i--;
        *(wchar_t *)((wchar_t *)wb + i) = *(char *)(mb + i);
    } while (i);
    return wb+sz;
}
#endif
// run-time hash
uint32_t regcall StringUtil::hash_rta(char * str)
{
    unsigned hash = 0;
    while (*str)
    {
        hash = hash * 31;
        hash += *str++;
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtai(char * str)
{
    unsigned hash = 0;
    while (*str)
    {
        hash = hash * 31;
        hash += (*str > 'Z' ? *str++ : (*str++ | 0x20));
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtas(char * str,uintptr_t sz)
{
    unsigned hash = 0;
    unsigned i=0;
    while (sz>i)
    {
        hash = hash * 31;
        hash += *str++;
        i++;
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtasi(char * str,uintptr_t sz)
{
    unsigned hash = 0;
    unsigned i=0;
    while (sz>i)
    {
        hash = hash * 31;
        hash += (*str > 'Z' ? *str++ : (*str++ | 0x20));
        i++;
    }

    return hash;
}


uint32_t regcall StringUtil::hash_rtw(wchar_t * str)
{
    unsigned hash = 0;
    while (*str)
    {
        hash = hash * 31;
        hash += *str++;
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtwi(wchar_t * str)
{
    unsigned hash = 0;
    while (*str)
    {
        hash = hash * 31;
        hash += (*str > 'Z' ? *str++ : (*str++ | 0x20));
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtws(wchar_t * str,uintptr_t sz)
{
    unsigned hash = 0;
    unsigned i=0;
    while (sz>i)
    {
        hash = hash * 31;
        hash += *str++;
        i++;
    }

    return hash;
}

uint32_t regcall StringUtil::hash_rtwsi(wchar_t * str,uintptr_t sz)
{
    unsigned hash = 0;
    unsigned i=0;
    while (sz>i)
    {
        hash = hash * 31;
        hash += (*str > 'Z' ? *str++ : (*str++ | 0x20));
        i++;
    }

    return hash;
}


#ifndef MAXULONG
#define MAXULONG  0xffffffff
#endif
#ifndef max
#define max(a,b)   (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)   (((a) < (b)) ? (a) : (b))
#endif

// Replacement for WinAPI IsTextUnicode from react os source code

bool MyRtlIsTextUnicode(CONST VOID* buf, INT len, INT* pf)
{

    const WCHAR *s = reinterpret_cast<const wchar_t*>(buf);
    int i;
    unsigned int flags = MAXULONG, out_flags = 0;
    UCHAR last_lo_byte = 0;
    UCHAR last_hi_byte = 0;
    ULONG hi_byte_diff = 0;
    ULONG lo_byte_diff = 0;
    ULONG weight = 3;
    ULONG lead_byte = 0;

    if (len < sizeof(WCHAR))
    {
        /* FIXME: MSDN documents IS_TEXT_UNICODE_BUFFER_TOO_SMALL but there is no such thing... */
        if (pf) *pf = 0;

        return FALSE;
    }

    if (pf)
        flags = *pf;

    if (((char *)buf)[len - 1] == 0)
        len--;  /* Windows seems to do something like that to avoid e.g. false IS_TEXT_UNICODE_NULL_BYTES  */

    len /= sizeof(WCHAR);

    /* Windows only checks the first 256 characters */
    if (len > 256) len = 256;

    for (i = 0; i < len; i++)
    {
        UCHAR lo_byte = LOBYTE(s[i]);
        UCHAR hi_byte = HIBYTE(s[i]);

        lo_byte_diff += max(lo_byte, last_lo_byte) - min(lo_byte, last_lo_byte);
        hi_byte_diff += max(hi_byte, last_hi_byte) - min(hi_byte, last_hi_byte);

        last_lo_byte = lo_byte;
        last_hi_byte = hi_byte;

        switch (s[i])
        {
            case 0xFFFE: /* Reverse BOM */
            case UNICODE_NULL:
            case 0x0A0D: /* ASCII CRLF (packed into one word) */
            case 0xFFFF: /* Unicode 0xFFFF */
                out_flags |= IS_TEXT_UNICODE_ILLEGAL_CHARS;
                break;
        }
    }

    if (pf)
    {
        out_flags &= *pf;
        *pf = out_flags;
    }

    return FALSE;
}
