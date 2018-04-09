
#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <stdint.h>
#include <assert.h>

#define transferTo(i,o)			_transferTo<__typeof__(i),__typeof__(o)>(i,o)
#define transferToMax(i,o,maxsize)	_transferTo<__typeof__(i),__typeof__(o)>(i,o,maxsize)
#define setTransferMaxBufSize(s)	(_transfer_maxsize = (s))

#define TRANSFERBUFRNDBITS		4 // round bufsize to multiple of 2^4
#define TRANSFERBUFRND			(1<<TRANSFERBUFRNDBITS)
#define TRANSFERBUFSZ(x)		(((x) + (TRANSFERBUFRND) - 1) & ~((TRANSFERBUFRND) - 1))

#ifndef TRANSFERBUFABSMAXSIZE
#ifndef TCP_MSS
#warning TCP_MSS is undefined
#define TRANSFERBUFABSMAXSIZE		(1460)
#else // defined(TCP_MSS)
#define TRANSFERBUFABSMAXSIZE		(TCP_MSS)
#endif // defined(TCP_MSS)
#endif // !defined(TRANSFERBUFABSMAXSIZE)

size_t _transferTo_bufsize __attribute__((weak)) = 0;
char*  _transferTo_buf     __attribute__((weak)) = 0;

size_t transferTo_alloc (size_t size1, size_t size2, size_t maxsize = 0) __attribute__((weak, warn_unused_result));
size_t transferTo_alloc (size_t size1, size_t size2, size_t maxsize)
{
	if (size1 > size2)
		size1 = size2;
	if (maxsize && size1 > maxsize)
		size1 = maxsize;
	if (size1 > TRANSFERBUFABSMAXSIZE)
		size1 = TRANSFERBUFABSMAXSIZE;
	if (!size1)
		return 0;
	if (size1 > _transferTo_bufsize)
	{
		size_t newsize = TRANSFERBUFSZ(size1);
		char* newbuf = (char*)realloc(_transferTo_buf, newsize);
		if (newbuf)
		{
			_transferTo_buf = newbuf;
			_transferTo_bufsize = newsize;
		}
		else
			size1 = _transferTo_bufsize;
		//os_printf(":strm %d\n", _transferTo_bufsize);
		if (!_transferTo_bufsize)
			return 0;
	}
	return size1;
}

template <class SI, class SO>
size_t _transferTo (SI& in, SO& out, ssize_t maxsize = 0)
{
	size_t size = transferTo_alloc(in.available(), out.availableForWrite(), maxsize);
	if (!size)
		return 0;
	size = in.readBytes(_transferTo_buf, size);
	size_t wsize = out.write((const uint8_t*)_transferTo_buf, size);
	//assert(wsize == size);
	return wsize;
}

#endif // __TRANSFER_H
