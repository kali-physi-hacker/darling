#include "config.h"
#include "stdio.h"
#include "errno.h"
#include "common/path.h"
#include "common/auto.h"
#include "darwin_errno_codes.h"
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <stdio_ext.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"

//extern "C"
//{
__darwin_FILE* __stdinp  asm("__stdinp") = 0;
__darwin_FILE* __stdoutp asm("__stdoutp") = 0;
__darwin_FILE* __stderrp asm("__stderrp") = 0;
//}

static __darwin_FILE* InitDarwinFILE(FILE* linux_fp)
{
	if (!linux_fp)
		return 0;

	__darwin_FILE* fp = new __darwin_FILE;
	fp->_p = NULL;
	fp->_r = -1;
	fp->_w = -1;
	fp->_flags = 0;
	fp->_file = fileno(linux_fp);
	fp->_lbfsize = 0;
	fp->linux_fp = linux_fp;
	return fp;
}

template<typename RetVal, typename Func, typename... Params> RetVal AutoFileErrno(Func f, __darwin_FILE* file, Params... params)
{
	RetVal rv = f(params..., file->linux_fp);
	if (!retvalOK(rv))
		errnoOut();
	return rv;
}

__darwin_FILE* __darwin_fopen(const char* path, const char* mode)
{
	path = translatePathCI(path);
	
	if (!strchr(mode, 'x'))
		return InitDarwinFILE(fopen(path, mode));
	else // DARWIN_EXTSN
	{
		std::string m = mode;
		size_t pos = m.find('x');
		m.erase(pos, 1);
		
		int flags = O_CREAT | O_EXCL;
		if (m.find('a') != std::string::npos)
			flags |= O_APPEND;
		if (m.find('+') != std::string::npos)
			flags |= O_RDWR;
		else
			flags |= O_WRONLY;
		
		int fd = ::open(path, flags, 0666);
		if (fd == -1)
			return 0;
		else
			return __darwin_fdopen(fd, m.c_str());
	}
}

__darwin_FILE* __darwin_fdopen(int fd, const char* mode)
{
	return InitDarwinFILE(fdopen(fd, mode));
}

__darwin_FILE* __darwin_freopen(const char* path, const char* mode, __darwin_FILE* fp)
{
	return InitDarwinFILE(freopen(translatePathCI(path), mode, fp->linux_fp));
}

int __darwin_fclose(__darwin_FILE* fp)
{
	int r = fclose(fp->linux_fp);
	delete fp;
	return r;
}

size_t __darwin_fread(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp)
{
	return fread(ptr, size, nmemb, fp->linux_fp);
}

size_t __darwin_fwrite(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp)
{
	return fwrite(ptr, size, nmemb, fp->linux_fp);
}

int __darwin_fseek(__darwin_FILE* fp, long offset, int whence)
{
	return fseek(fp->linux_fp, offset, whence);
}

long __darwin_ftell(__darwin_FILE* fp)
{
	return ftell(fp->linux_fp);
}

void __darwin_rewind(__darwin_FILE* fp)
{
	return rewind(fp->linux_fp);
}

int __darwin_getc(__darwin_FILE* fp)
{
	return getc(fp->linux_fp);
}

int __darwin_fgetc(__darwin_FILE* fp)
{
	return fgetc(fp->linux_fp);
}

int __darwin_ungetc(int c, __darwin_FILE* fp)
{
	return ungetc(c, fp->linux_fp);
}

char* __darwin_fgets(char* s, int size, __darwin_FILE* fp)
{
	return fgets(s, size, fp->linux_fp);
}

int __darwin_putc(int c, __darwin_FILE* fp)
{
	return putc(c, fp->linux_fp);
}

int __darwin_fputc(int c, __darwin_FILE* fp)
{
	return fputc(c, fp->linux_fp);
}

int __darwin_fputs(const char* s, __darwin_FILE* fp)
{
	return fputs(s, fp->linux_fp);
}

int __darwin_fprintf(__darwin_FILE* fp, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(fp->linux_fp, fmt, ap);
	va_end(ap);
	return r;
}

int __darwin_vfscanf(__darwin_FILE* fp, const char* fmt, va_list ap)
{
	return vfscanf(fp->linux_fp, fmt, ap);
}

int __darwin_fscanf(__darwin_FILE* fp, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfscanf(fp->linux_fp, fmt, ap);
	va_end(ap);
	return r;
}

int __darwin_vfprintf(__darwin_FILE* fp, const char* fmt, va_list ap)
{
	return vfprintf(fp->linux_fp, fmt, ap);
}

int __darwin_fflush(__darwin_FILE* fp)
{
	if (!fp)
		return fflush(NULL);
	return fflush(fp->linux_fp);
}

void __darwin_setbuf(__darwin_FILE* fp, char* buf)
{
	setbuf(fp->linux_fp, buf);
}

void __darwin_setbuffer(__darwin_FILE* fp, char* buf, size_t size)
{
	setbuffer(fp->linux_fp, buf, size);
}

int __darwin_ferror(__darwin_FILE* fp)
{
	LOGF("ferror: %d\n", ferror(fp->linux_fp));
	return ferror(fp->linux_fp);
}

int __darwin_fileno(__darwin_FILE* fp)
{
	return fileno(fp->linux_fp);
}

__darwin_FILE* __darwin_tmpfile()
{
	return InitDarwinFILE(tmpfile());
}

int __darwin_feof(__darwin_FILE *stream)
{
	if (!stream)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		int rv = feof(stream->linux_fp);
		if (rv == -1)
			errnoOut();
		return rv;
	}
}

void __darwin_clearerr(__darwin_FILE *stream)
{
	if (stream)
		clearerr(stream->linux_fp);
}

int __darwin_fgetpos(__darwin_FILE *stream, fpos_t *pos)
{
	if (!stream)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		int rv = fgetpos(stream->linux_fp, pos);
		if (rv == -1)
			errnoOut();
		return rv;
	}
}

int __darwin_fsetpos(__darwin_FILE *stream, fpos_t *pos)
{
	if (!stream)
    {
        errno = DARWIN_EINVAL;
        return -1;
    }
    else
    {
        int rv = fsetpos(stream->linux_fp, pos);
        if (rv == -1)
            errnoOut();
        return rv;
    }
}

int __darwin_fpurge(__darwin_FILE *stream)
{
	if (!stream)
	{
		errno = DARWIN_EINVAL;
		return -1;
	}
	else
	{
		__fpurge(stream->linux_fp);
		return 0;
	}
}

int __darwin_getw(__darwin_FILE *stream)
{
	return AutoFileErrno<int>(getw, stream);
}

int __darwin_putw(int w, __darwin_FILE *stream)
{
	return AutoFileErrno<int>(putw, stream, w);
}

wint_t __darwin_fgetwc(__darwin_FILE *stream)
{
	return AutoFileErrno<wint_t>(fgetwc, stream);
}

wint_t __darwin_getwc(__darwin_FILE *stream)
{
	return AutoFileErrno<wint_t>(getwc, stream);
}

wint_t __darwin_fputwc(wchar_t wc, __darwin_FILE *stream)
{
	return AutoFileErrno<wint_t>(fputwc, stream, wc);
}

wint_t __darwin_putwc(wchar_t wc, __darwin_FILE *stream)
{
	return AutoFileErrno<wint_t>(putwc, stream, wc);
}

int __darwin_fputws(const wchar_t *ws, __darwin_FILE *stream)
{
	return AutoFileErrno<int>(fputws, stream, ws);
}

wchar_t *__darwin_fgetws(wchar_t *ws, int n, __darwin_FILE *stream)
{
	return AutoFileErrno<wchar_t*>(fgetws, stream, ws, n);
}

wint_t __darwin_ungetwc(wint_t wc, __darwin_FILE *stream)
{
	return AutoFileErrno<wint_t>(ungetwc, stream, wc);
}


__attribute__((constructor)) static void initStdio()
{
	__stderrp = InitDarwinFILE(stderr);
	__stdoutp = InitDarwinFILE(stdout);
	__stdinp = InitDarwinFILE(stdin);
}

int __darwin_remove(const char* path)
{
	return AutoPathErrno<int>(remove, path);
}