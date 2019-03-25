#include <lib/encode.h>
#include <lib/assert.h>
#include <lib/fundamental.h>
#include <stdlib.h>


static unsigned char encoding_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
};


static unsigned char *decoding_table = 0;
static int mod_table[] = { 0, 2, 1 };


void build_decoding_table()
{
        decoding_table = (unsigned char*)malloc(256);
        unsigned char i;

        for (i = 0; i < 64; i++) {
                decoding_table[encoding_table[i]] = i;
        }
}


void
lib_base64_encode(
        const unsigned char *data,
        unsigned in_len,
        unsigned char **output_data,
        unsigned *output_length)
{
        /* param check */
        LIB_ASSERT(output_length);

        if(!output_length) {
                return;
        }

        *output_length = 4 * ((in_len + 2) / 3);

        if (!output_data) {
                return;
        }

        unsigned char *encoded_data = *output_data;
        unsigned i, j;

        for (i = 0, j = 0; i < in_len;) {
                uint32_t oct_a = i < in_len ? (unsigned char)data[i++] : 0;
                uint32_t oct_b = i < in_len ? (unsigned char)data[i++] : 0;
                uint32_t oct_c = i < in_len ? (unsigned char)data[i++] : 0;

                uint32_t tri = (oct_a << 0x10) + (oct_b << 0x08) + oct_c;

                unsigned char en_data = 0;

                en_data = encoding_table[(tri >> 3 * 6) & 0x3F];
                encoded_data[j++] = en_data;

                en_data = encoding_table[(tri >> 2 * 6) & 0x3F];
                encoded_data[j++] = en_data;

                en_data = encoding_table[(tri >> 1 * 6) & 0x3F];
                encoded_data[j++] = en_data;

                en_data = encoding_table[(tri >> 0 * 6) & 0x3F];
                encoded_data[j++] = en_data;
        }

        for (i = 0; i < (unsigned)mod_table[in_len % 3]; i++) {
                encoded_data[*output_length - 1 - i] = '=';
        }
        
}


void
lib_base64_decode(
  const unsigned char *data,
  unsigned input_length,
  unsigned char **output_data,
  unsigned *output_length)
{
        /* param check */
        LIB_ASSERT(output_length);

        if (decoding_table == NULL) { build_decoding_table(); }

        if (input_length % 4 != 0) {
                LIB_ASSERT(0);
                return;
        }

        *output_length = input_length / 4 * 3;
        if (data[input_length - 1] == '=') (*output_length)--;
        if (data[input_length - 2] == '=') (*output_length)--;

  if(output_data)
  {
    unsigned char *decoded_data = *output_data;
    unsigned i,j;

    for (i = 0, j = 0; i < input_length;) {

      uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
      uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
      uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
      uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];

      uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

      if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
      if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
      if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
  }
}