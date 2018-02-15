
#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <stdint.h>
#include <assert.h>

#define transferTo(i,o)			_transferTo<__typeof__(i),__typeof__(o)>(i,o)
#define setTransferMaxBufSize(s)	(_transfer_maxsize = (s))

#define USTREAMBUFRNDBITS		4 // round bufsize to multiple of 2^4
#define USTREAMBUFRND			(1<<USTREAMBUFRNDBITS)
#define USTREAMBUFSZ(x)			(((x) + USTREAMBUFRND - 1) & ~(USTREAMBUFRND - 1))

size_t _transfer_maxsize __attribute__((weak)) = TCP_MSS; // default
size_t _transfer_bufsize __attribute__((weak)) = 0;
char*  _transfer_buf     __attribute__((weak)) = 0;

size_t _transferTo_size (size_t availr, size_t availw) __attribute__((weak));
size_t _transferTo_size (size_t availr, size_t availw)
{
	if (availr > availw)
		availr = availw;
	if (!availr)
		return 0;
	if (_transfer_maxsize && availr > _transfer_maxsize)
		availr = _transfer_maxsize;
	if (availr > _transfer_bufsize)
	{
		size_t newsize = USTREAMBUFSZ(availr);
		char* newbuf = (char*)realloc(_transfer_buf, newsize);
		if (newbuf)
		{
			_transfer_buf = newbuf;
			_transfer_bufsize = newsize;
		}
		else
			availr = _transfer_bufsize;
		//os_printf(":strm %d\n", _transfer_bufsize);
		if (!_transfer_bufsize)
			return 0;
	}
	return availr;
}

template <class SI, class SO>
size_t _transferTo (SI& in, SO& out)
{
	size_t size = _transferTo_size(in.available(), out.availableForWrite());
	if (!size)
		return 0;
	size = in.readBytes(_transfer_buf, size);
	size_t wsize = out.write((const uint8_t*)_transfer_buf, size);
	//assert(wsize == size);
	return wsize;
}

#endif // __TRANSFER_H
