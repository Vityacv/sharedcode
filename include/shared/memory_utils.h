#ifndef _MEMORY_UTILS_H
#define _MEMORY_UTILS_H
#include "shared/common_macro.h"
#define X86_PAGE_SIZE 0x1000

#ifdef _WIN32

#define GetProcAddr(hMod, name) GetProcAddressByHash(hMod, StringUtil::hash_ct(name))

#define GetModuleSize(x)                                                 \
    (((PIMAGE_NT_HEADERS)((BYTE *)x + ((PIMAGE_DOS_HEADER)x)->e_lfanew)) \
         ->OptionalHeader.SizeOfImage)
void *regcall GetProcAddressByHash(void * hMod, unsigned hashName);
uint8_t regcall isMemoryExist(void *adr);
uint8_t regcall isModuleExist(uintptr_t mod);
void regcall unprotectCode(uint8_t *adr, uint32_t sz = 100);
void regcall unprotectMem(uint8_t *adr, uint32_t sz = 100);
void *regcall memmem(uint8_t *haystack, size_t haystack_len, uint8_t *needle,
                     size_t needle_len);
#endif
void regcall memswap(uint8_t *src, uint8_t *dst, unsigned len);
void *regcall memcpyl(void *d, const void *s, size_t n);
inline int regcall floatToInt(float x) { return int(x); }
inline uint32_t regcall floatToUInt32(float x) { return uint32_t(x); }
int regcall GetRandomInt(int range);
int regcall GetRandomIntRng(int lo, int hi);
float regcall GetRandomFloat(float min, float max);
uint8_t *regcall searchBytesDbg(uint8_t *pBuff, uintptr_t pBuffSize,
                             uint8_t *pPattBuf);
uint8_t *regcall scanBytesImpl(uint8_t *pBuff, uintptr_t pBuffSize,
                           uint8_t *pPattBuf);
#ifdef _FORCE_DBGLOG
#define scanBytes scanBytesDbg
#else
#define scanBytes scanBytesImpl
#endif

uint8_t * searchSkipBytes(uint8_t *pBuff, uintptr_t pBuffSize, ...);
#define bytes_search
#ifdef bytes_search
namespace bytes_search {
template <uintptr_t X>
struct EnsureCompileTime {
    enum : uintptr_t { Value = X };
};
#ifndef _FORCE_INLINE
#ifdef _MSC_VER /* Visual Studio */
#define _FORCE_INLINE __forceinline
#else
#if defined(__cplusplus) || \
    defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L /* C99 */
#ifdef __GNUC__
#define _FORCE_INLINE inline __attribute__((always_inline))
#else
#define _FORCE_INLINE inline
#endif
#else
#define _FORCE_INLINE
#endif /* __STDC_VERSION__ */
#endif /* _MSC_VER */
#endif /* _FORCE_INLINE */

template <unsigned... Pack>
struct IndexList {};

template <typename IndexList, unsigned Right>
struct Append;
template <unsigned... Left, unsigned Right>
struct Append<IndexList<Left...>, Right> {
    typedef IndexList<Left..., Right> Result;
};

template <unsigned N>
struct ConstructIndexList {
    typedef typename Append<typename ConstructIndexList<N - 1>::Result,
                            N - 1>::Result Result;
};
template <>
struct ConstructIndexList<0> {
    typedef IndexList<> Result;
};
inline const unsigned char XORKEY =
    static_cast<char>(EnsureCompileTime<0>::Value);
_FORCE_INLINE constexpr unsigned char obfuscateChar(uint8_t *mask,
                                                    size_t masksz,
                                                    const char c1,
                                                    const char c2,
                                                    unsigned Index) {
    if (!Index) {
        memset(mask, 0, masksz);
    }
    mask[(unsigned)(Index / 8)] &= ~(1 << (Index % 8));
    return (c1 | c2) + XORKEY;
}
_FORCE_INLINE constexpr unsigned char obfuscateCharQ(uint8_t *mask,
                                                     unsigned Index) {
    mask[(unsigned)(Index / 8)] |= 1 << (Index % 8);
    return 0 + XORKEY;
}
template <typename IndexList>
class format;
template <unsigned... Index>
class format<IndexList<Index...>> {
   private:
    unsigned short sz = sizeof...(Index);
    unsigned char Value[sizeof...(Index)];
    unsigned char Mask[((unsigned)(sizeof...(Index) / 8) + 1)];

   public:
    _FORCE_INLINE constexpr format(const char *const String)
        : Value{String[Index * 2] == '?'
                    ? obfuscateCharQ(Mask, Index)
                    : obfuscateChar(
                          Mask, ((unsigned)(sizeof...(Index) / 8) + 1),
                          (String[Index * 2] >= 'A' ? String[Index * 2] - '7'
                                                    : String[Index * 2] - '0')
                              << 4,
                          String[(Index * 2) + 1] >= 'A'
                              ? String[(Index * 2) + 1] - '7'
                              : String[(Index * 2) + 1] - '0',
                          Index)...} {}
    _FORCE_INLINE uint8_t *formatGet() { return (uint8_t *)&sz; }
};
}  // namespace bytes_search
#define BYTES_SEARCH_FORMAT(x)                                   \
    (bytes_search::format<bytes_search::ConstructIndexList<      \
         static_cast<uint32_t>((sizeof(x) - 1) / 2)>::Result>(x) \
         .formatGet())
#include <array>
// Converts a single hex character to its integer value
constexpr uint8_t hexCharToByte(char c) {
    return (c >= '0' && c <= '9') ? (c - '0') :
           (c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
           (c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0;
}

// Converts a string of hex digits to a single byte
constexpr uint8_t hexPairToByte(char high, char low) {
    return (hexCharToByte(high) << 4) | hexCharToByte(low);
}

// Converts a string of hex digits to a constexpr array of bytes and generates a bit-level mask
template<std::size_t N>
constexpr auto hexStringToByteArrayWithBitMask(const char(&str)[N]) {
    static_assert(N % 2 == 1, "Hex string must have an even number of characters.");

    constexpr std::size_t byteSize = (N - 1) / 2; // Total bytes
    std::array<uint8_t, byteSize> bytes = {};
    std::array<uint8_t, (byteSize + 7) / 8> bitMask = {};  // Bit mask array (1 bit per byte)

    for (std::size_t i = 0; i < N - 1; i += 2) {
        std::size_t byteIndex = i / 2;

        // Determine if the current byte is part of a wildcard pattern
        bool isWildcard = (str[i] == '?') || (str[i + 1] == '?');

        if (isWildcard) {
            // Set the bit corresponding to this byte to 1 in the bit mask
            bitMask[byteIndex / 8] |= (1 << ( (byteIndex % 8)));
        } else {
            bytes[byteIndex] = hexPairToByte(str[i], str[i + 1]);
        }
    }

    return std::make_pair(bytes, bitMask);
}

// Define a structure to hold size, pattern, and bit mask
template<std::size_t PatternSize, std::size_t MaskSize>
struct PackedPattern {
    uint16_t size;
    std::array<uint8_t, PatternSize> pattern;
    std::array<uint8_t, MaskSize> bitMask;
};

// Define a constexpr function to create PackedPattern and return it
template<std::size_t N>
constexpr PackedPattern<((N - 1) / 2), (((N - 1) / 2 + 7) / 8)>
createPackedPattern(const char(&pattern)[N]) {
    auto result = hexStringToByteArrayWithBitMask(pattern);
    return PackedPattern<((N - 1) / 2), (((N - 1) / 2 + 7) / 8)>{
        static_cast<uint16_t>(result.first.size()),
        result.first,
        result.second
    };
}

// Define macro to directly create PackedPattern and use it in printPackedData
#define BSF(pattern) (createPackedPattern(pattern))

// #define BSF BYTES_SEARCH_FORMAT
#endif
#endif
