
#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <stdint.h>
#include <assert.h>

#define transferTo(i,o)			_transferTo<__typeof__(i),__typeof__(o)>(i,o)
#define transferToKw(i,o,kw)		_transferTo<__typeof__(i),__typeof__(o)>(i,o,kw)
#define setTransferMaxBufSize(s)	(_transfer_maxsize = (s))

#define USTREAMBUFRNDBITS		4 // round bufsize to multiple of 2^4
#define USTREAMBUFRND			(1<<USTREAMBUFRNDBITS)
#define USTREAMBUFSZ(x)			(((x) + USTREAMBUFRND - 1) & ~(USTREAMBUFRND - 1))

#ifndef TCP_MSS
#warning TCP_MSS undefined
#define TCP_MSS 1460
#endif

size_t _transferTo_maxsize __attribute__((weak)) = TCP_MSS; // default
size_t _transferTo_bufsize __attribute__((weak)) = 0;
char*  _transferTo_buf     __attribute__((weak)) = 0;

size_t _transferTo_size (size_t availr, size_t availw) __attribute__((weak, warn_unused_result));
size_t _transferTo_size (size_t availr, size_t availw)
{
	if (availr > availw)
		availr = availw;
	if (!availr)
		return 0;
	if (_transferTo_maxsize && availr > _transferTo_maxsize)
		availr = _transferTo_maxsize;
	if (availr > _transferTo_bufsize)
	{
		size_t newsize = USTREAMBUFSZ(availr);
		char* newbuf = (char*)realloc(_transferTo_buf, newsize);
		if (newbuf)
		{
			_transferTo_buf = newbuf;
			_transferTo_bufsize = newsize;
		}
		else
			availr = _transferTo_bufsize;
		//os_printf(":strm %d\n", _transferTo_bufsize);
		if (!_transferTo_bufsize)
			return 0;
	}
	return availr;
}

void _transferTo_alloc (size_t size) __attribute__((weak));
void _transferTo_alloc (size_t size)
{
	size_t ign = _transferTo_size(size, size);
	(void)sizeof(ign);
}

template <class SI, class SO>
size_t _transferTo (SI& in, SO& out, size_t keepw = 0)
{
	size_t availw = out.availableForWrite();
	availw = (availw < keepw)? 0: availw - keepw;
	size_t size = _transferTo_size(in.available(), availw);
	if (!size)
		return 0;
	size = in.readBytes(_transferTo_buf, size);
	size_t wsize = out.write((const uint8_t*)_transferTo_buf, size);
	//assert(wsize == size);
	return wsize;
}

#endif // __TRANSFER_H
