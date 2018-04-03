
#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <stdint.h>
#include <assert.h>

#define transferTo(i,o)			_transferTo<__typeof__(i),__typeof__(o)>(i,o)
#define transferToMax(i,o,ms)		_transferTo<__typeof__(i),__typeof__(o)>(i,o,ms)
#define setTransferMaxBufSize(s)	(_transfer_maxsize = (s))

#define USTREAMBUFRNDBITS		4 // round bufsize to multiple of 2^4
#define USTREAMBUFRND			(1<<USTREAMBUFRNDBITS)
#define USTREAMBUFSZ(x)			(((x) + USTREAMBUFRND - 1) & ~(USTREAMBUFRND - 1))

size_t _transferTo_bufsize __attribute__((weak)) = 0;
char*  _transferTo_buf     __attribute__((weak)) = 0;

size_t _transferTo_size (size_t availr, size_t availw, ssize_t maxsize = -1) __attribute__((weak, warn_unused_result));
size_t _transferTo_size (size_t availr, size_t availw, ssize_t maxsize)
{
	if (availr > availw)
		availr = availw;
	if (maxsize >= 0 && availr > (size_t)maxsize)
		availr = maxsize;
	if (!availr)
		return 0;
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
size_t _transferTo (SI& in, SO& out, ssize_t maxsize = -1)
{
	size_t size = _transferTo_size(in.available(), out.availableForWrite(), maxsize);
	if (!size)
		return 0;
	size = in.readBytes(_transferTo_buf, size);
	size_t wsize = out.write((const uint8_t*)_transferTo_buf, size);
	//assert(wsize == size);
	return wsize;
}

#endif // __TRANSFER_H
