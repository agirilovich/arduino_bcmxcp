int snprintfcat(char *dst, size_t size, const char *fmt, ...);
void *xmalloc(size_t size);
ssize_t select_read(const int fd, void *buf, const size_t buflen, const time_t d_sec, const suseconds_t d_usec);
