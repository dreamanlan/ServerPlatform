#include <stdio.h>
#include <string.h>

#define BASE64_PAD64 '='

static char s_base64_alphabet[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a',
                                 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                                 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
                                 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1',
                                 '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static char s_base64_suffix_map[256] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
    -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};


static char cmove_bits(unsigned char src, unsigned lnum, unsigned rnum)
{
    src <<= lnum;
    src >>= rnum;
    return src;
}

int base64_encode(const char* data, int dlen, char* ret, int space)
{
    if (NULL == data || dlen <= 0)
        return 0;
    int m = 0, padnum = 0, retsize = 0;

    /* Account the result buffer size. */
    if ((dlen % 3) != 0)
        padnum = 3 - dlen % 3;
    retsize = (dlen + padnum) + ((dlen + padnum) * 1 / 3) + 1;
    if (NULL != ret && space >= retsize) {
        char* retpos = ret;
        for (m = 0; m < (dlen + padnum); m += 3) {
            *(retpos) = s_base64_alphabet[cmove_bits(*data, 0, 2)];

            if (m == dlen + padnum - 3 && padnum != 0 && padnum != 2) {
                /* Whether the last bits-group suffice 24 bits. */
                if (padnum == 1) {
                    /* 16bit need pad one '='. */
                    *(retpos + 1) = s_base64_alphabet[cmove_bits(*data, 6, 2) + cmove_bits(*(data + 1), 0, 4)];
                    *(retpos + 2) = s_base64_alphabet[cmove_bits(*(data + 1), 4, 2)];
                    *(retpos + 3) = BASE64_PAD64;
                }

            }
            else if (padnum == 2 && ((dlen == 1) || (m + 3 == (dlen + padnum)))) {
                /* 8bit need pad two'='. */
                *(retpos + 1) = s_base64_alphabet[cmove_bits(*data, 6, 2)];
                *(retpos + 2) = BASE64_PAD64;
                *(retpos + 3) = BASE64_PAD64;
            }
            else {
                /* 24bit normal. */
                *(retpos + 1) = s_base64_alphabet[cmove_bits(*data, 6, 2) + cmove_bits(*(data + 1), 0, 4)];
                *(retpos + 2) = s_base64_alphabet[cmove_bits(*(data + 1), 4, 2) + cmove_bits(*(data + 2), 0, 6)];
                *(retpos + 3) = s_base64_alphabet[*(data + 2) & 0x3f];
            }

            retpos += 4;
            data += 3;
        }
        ret[retsize - 1] = 0;
    }
    return retsize;
}

int base64_decode(const char* bdata, char* ret, int space)
{
    if (NULL == bdata)
        return 0;
    int m = 0, padnum = 0, retsize = 0, bdlen = static_cast<int>(strlen(bdata));
    if (bdlen == 0)
        return 0;
    if (bdlen % 4 != 0)
        return 0;

    /* Whether the data have invalid base-64 characters? */
    for (m = 0; m < bdlen; ++m) {
        if ((bdata[m] != BASE64_PAD64) && (s_base64_suffix_map[bdata[m]] == 255))
            return 0;
    }

    /* Account the output size. */
    if (bdata[bdlen - 1] == '=')
        padnum = 1;

    if (bdata[bdlen - 1] == '=' && bdata[bdlen - 2] == '=')
        padnum = 2;
    retsize = (bdlen - 4) - (bdlen - 4) / 4 + (3 - padnum) + 1;
    if (NULL != ret && space >= retsize) {
        char* retpos = ret;
        /* Begging to decode. */
        for (m = 0; m < bdlen; m += 4) {
            *retpos = cmove_bits(s_base64_suffix_map[*bdata], 2, 0) + cmove_bits(s_base64_suffix_map[*(bdata + 1)], 0, 4);

            if (m == bdlen - 4 && padnum != 0) {
                /* Only deal with last four bits. */
                if (padnum == 1)/* Have one pad characters, only two availability characters. */
                    *(retpos + 1) = cmove_bits(s_base64_suffix_map[*(bdata + 1)], 4, 0) + cmove_bits(s_base64_suffix_map[*(bdata + 2)], 0, 2);
                retpos += 3 - padnum;
            }
            else {
                *(retpos + 1) = cmove_bits(s_base64_suffix_map[*(bdata + 1)], 4, 0) + cmove_bits(s_base64_suffix_map[*(bdata + 2)], 0, 2);
                *(retpos + 2) = cmove_bits(s_base64_suffix_map[*(bdata + 2)], 6, 0) + s_base64_suffix_map[*(bdata + 3)];
                retpos += 3;
            }
            bdata += 4;
        }

        ret[retsize - 1] = 0;
    }
    return retsize;
}

