#ifndef FILE_INCLUDED_6162485A_7F38_4B43_95D8_0BCFC314195F
#define FILE_INCLUDED_6162485A_7F38_4B43_95D8_0BCFC314195F


/*
 *  Copys the contents of a file into a string.
 *  out_len is optional.
 *
 *  You have to free the buffer.
 *
 *  Returns 1 for success and 0 for failure.
 */
int
lib_copy_file(
        const char *filename,
        char **out_file,
        unsigned *out_len);


#endif /* inc guard */
