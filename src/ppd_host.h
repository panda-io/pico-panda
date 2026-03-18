// ── ppd_host.h ────────────────────────────────────────────────────────────────
// C helpers for the ppd host runtime (timing, SIGINT, boot.ppd path).

#ifndef PPD_HOST_H
#define PPD_HOST_H

#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>

// ── Timing ────────────────────────────────────────────────────────────────────

// Returns elapsed microseconds since first call (never wraps in practice).
static int64_t _ppd_time_us(void) {
    static int _ppd_time_init = 0;
    static struct timeval _ppd_time_start;
    if (!_ppd_time_init) {
        gettimeofday(&_ppd_time_start, NULL);
        _ppd_time_init = 1;
    }
    struct timeval now;
    gettimeofday(&now, NULL);
    return (int64_t)(now.tv_sec - _ppd_time_start.tv_sec) * 1000000LL
         + (now.tv_usec - _ppd_time_start.tv_usec);
}

// ── SIGINT handling ───────────────────────────────────────────────────────────

static volatile int _ppd_exit_flag_val = 0;
static void _ppd_sigint_handler(int sig) { (void)sig; _ppd_exit_flag_val = 1; }
static void _ppd_install_sigint(void) { signal(SIGINT, _ppd_sigint_handler); }
static int  _ppd_exit_flag(void) { return _ppd_exit_flag_val; }

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
