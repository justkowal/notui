#ifdef _WIN32
#include <cwchar>
#include <ctime>

#undef stat64
#undef fstat64

extern "C" {
    int MSVCRT_stat64(const char* path, void* buffer) __asm__("_stat64");
    int MSVCRT_fstat64(int fd, void* buffer) __asm__("_fstat64");

    extern char* tiparm(const char* format, ...);
    extern char* tigetstr(const char* capname);
    extern int tigetnum(const char* capname);
    extern int del_curterm(void* sp);
    extern int tigetflag(const char* capname);
    extern const char* curses_version(void);
    extern size_t wcsrtombs(char* dst, const wchar_t** src, size_t len, mbstate_t* ps);
    extern size_t mbrtowc(wchar_t* pwc, const char* s, size_t n, mbstate_t* ps);
    extern size_t wcrtomb(char* s, wchar_t wc, mbstate_t* ps);
    extern int wctob(wint_t c);
    extern wint_t btowc(int c);

    extern void* cur_term;

    // NOLINTBEGIN(bugprone-reserved-identifier,cert-dcl51-cpp,readability-identifier-naming,cppcoreguidelines-pro-type-reinterpret-cast,google-readability-casting)
    void* __imp_tiparm = (void*)&tiparm;
    void* __imp_tigetstr = (void*)&tigetstr;
    void* __imp_tigetnum = (void*)&tigetnum;
    void* __imp_del_curterm = (void*)&del_curterm;
    void* __imp_tigetflag = (void*)&tigetflag;
    void* __imp_curses_version = (void*)&curses_version;
    void* __imp_wcsrtombs = (void*)&wcsrtombs;
    void* __imp_mbrtowc = (void*)&mbrtowc;
    void* __imp_wcrtomb = (void*)&wcrtomb;
    void* __imp_wctob = (void*)&wctob;
    void* __imp_btowc = (void*)&btowc;
    void* __imp_cur_term = (void*)&cur_term;
    // NOLINTEND(bugprone-reserved-identifier,cert-dcl51-cpp,readability-identifier-naming,cppcoreguidelines-pro-type-reinterpret-cast,google-readability-casting)

    int clock_gettime64(int clock_id, struct timespec* tp) {
        extern int clock_gettime(clockid_t, struct timespec*);
        return clock_gettime(static_cast<clockid_t>(clock_id), tp);
    }
    int clock_nanosleep64(int clock_id, int flags, const struct timespec* rqtp, struct timespec* rmtp) {
        extern int clock_nanosleep(clockid_t, int, const struct timespec*, struct timespec*);
        return clock_nanosleep(static_cast<clockid_t>(clock_id), flags, rqtp, rmtp);
    }
    int pthread_cond_timedwait64(void* cond, void* mutex, const struct timespec* abstime) {
        extern int pthread_cond_timedwait(void*, void*, const void*);
        return pthread_cond_timedwait(cond, mutex, abstime);
    }

    int stat64(const char* path, void* buffer) {
        return MSVCRT_stat64(path, buffer);
    }
    int fstat64(int fd, void* buffer) {
        return MSVCRT_fstat64(fd, buffer);
    }
}
#endif
