#ifndef HM_POOL_H
#define HM_POOL_H

#include <hm_types.h>
#include "list.h"

/* This is the per session memory allocator
 * there are 1+ pool(s) for one session
 */

/* allocated */
typedef struct hm_hdr_s {
	u32 pool_idx:HM_BITS;		/* pool index, 0: freed */
	u32 zone_idx:32-HM_BITS;	/* zone index, 0: external */
} hm_hdr;

/* freed */
typedef struct hm_obj_s {
	u32 pool_idx:HM_BITS;		/* should be 0 after freed */
	u32 unused:32-HM_BITS;
	list_t list;
} hm_obj;

#define hm_obj_init(obj) do { \
	list_init(&(obj)->list); \
} whlie(0)

typedef struct hm_zone_s {
	u16 unit; /* the actual memory size is unit*HM_OBJ_MIN_SIZE */
	u16 count;
	list_t list;
} hm_zone;

#define hm_zone_init(zone, sz) do { \
	list_init(&(zone)->list); \
	(zone)->count = 0; \
	(zone)->size = (sz); \
} while(0)

#define hm_zone_insert(zone, obj) do { \
	list_add(&(obj)->list, &(zone)->list); \
	(zone)->count ++; \
} while(0)

typedef struct hm_pool_ops_s {
	void* (*alloc)(size_t);
	void (*free)(void*);
} hm_pool_ops;

typedef struct hm_pool_s {
	u32 magic;
	u16 pos;
	u16 size;
	hm_zone zone[HM_ZONE_MAX];
} hm_pool;

#define hm_pool_init(pool) do { \
	(pool)->magic = HM_POOL_MAGIC; \
} while(0)

#ifdef __cplusplus
extern "C" {
#endif

void hm_pool_fini(hm_pool* pool);

void* hm_pool_alloc(hm_pool* pool, size_t size);
void hm_pool_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif

