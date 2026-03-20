// ── ppd_host.h ────────────────────────────────────────────────────────────────
// C helpers for the ppd host runtime (boot.ppd path discovery).

#ifndef PPD_HOST_H
#define PPD_HOST_H

#include <string.h>
#include <stdint.h>

// ── boot.ppd path ─────────────────────────────────────────────────────────────

static char _ppd_boot_buf[512];

// Compute boot.ppd path: same directory as argv[0].
static void _ppd_init_boot_path(const char* argv0) {
    strncpy(_ppd_boot_buf, argv0, 511);
    _ppd_boot_buf[511] = '\0';
    char* slash = (char*)strrchr(_ppd_boot_buf, '/');
    if (slash) {
        *(slash + 1) = '\0';
    } else {
        _ppd_boot_buf[0] = '.';
        _ppd_boot_buf[1] = '/';
        _ppd_boot_buf[2] = '\0';
    }
    strncat(_ppd_boot_buf, "boot.ppd", 511 - strlen(_ppd_boot_buf));
}

static const char*   _ppd_boot_path(void)     { return _ppd_boot_buf; }
static uint32_t      _ppd_boot_path_len(void) { return (uint32_t)strlen(_ppd_boot_buf); }

#endif // PPD_HOST_H
