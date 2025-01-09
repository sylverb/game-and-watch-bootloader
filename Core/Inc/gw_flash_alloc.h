#include <stdint.h>
#include <stdbool.h>

void clear_flash_alloc_metadata();
uint8_t *store_file_in_flash(const char *file_path, uint32_t *file_size_p, bool byte_swap);
