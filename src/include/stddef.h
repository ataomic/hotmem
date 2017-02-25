#ifndef STDDEF_H_INCLUDED_
#define STDDEF_H_INCLUDED_

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type, member) );})
#endif

#ifndef __cplusplus

typedef enum {
	false = 0,
	true,
} bool;

#endif

#define WRITE_ONCE(x, y) ((x) = (y))
#define READ_ONCE(x) ((x))

#define RET_WAIT		0xff
#define RET_OK			0
#define RET_ERROR		-1
#define RET_DECLINED	-2
#define RET_DUP			-3

#define flag_bit(x) (1u<<(x))

#define flag_set(o, f) ((o)->flags |= (f))
#define flag_unset(o, f) ((o)->flags &= ~(f))
#define flag_test(o, f) ((o)->flags & (f))

#define array_size(array) (sizeof(array)/sizeof((array)[0]))

#endif

