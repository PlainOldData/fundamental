#ifndef ENCODE_INCLUDED_9A369505_F1F9_4501_83C3_6EDF6F93DDA7
#define ENCODE_INCLUDED_9A369505_F1F9_4501_83C3_6EDF6F93DDA7


#ifdef __cplusplus
extern "C" {
#endif


void lib_base64_encode(
  const unsigned char *data,
  unsigned input_length,
  unsigned char **output_data,
  unsigned *output_length);


void lib_base64_decode(
  const unsigned char *data,
  unsigned input_length,
  unsigned char **output_data,
  unsigned *output_length);


#ifdef __cplusplus
} /* extern */
#endif


#endif /* inc guard */
