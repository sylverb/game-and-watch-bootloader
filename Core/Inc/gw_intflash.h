#pragma once

#include "stdbool.h"

typedef void (*flash_progress_callback_t)(unsigned int percentage);

bool update_bank2_flash(flash_progress_callback_t progress_callback);
