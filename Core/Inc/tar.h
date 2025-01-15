#ifndef TAR_H
#define TAR_H
#include <stdbool.h>

typedef void (*progress_callback_t)(unsigned int percentage, const char *file_name);

bool extract_tar(const char *tar_path, const char *extract_path, progress_callback_t progress_callback);

#endif // TAR_H