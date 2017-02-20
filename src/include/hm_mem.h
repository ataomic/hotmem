#ifndef RS_HM_MEM_H
#define RS_HM_MEM_H

#include <rtl/types.h>
#include <rtl/list/dlist.h>

typedef struct hm_memctl_s {
	dlist_t list;
	ulong id;
}hm_memctl;

#define HM_MEM_MASK 0xfffful
#define HM_MEM_MAX (HM_MEM_MASK+1)

typedef struct hm_mem_s {
	dlist_t* ctls;
	int count;
}hm_mem;

#define hm_mem_head(mem, id) (&(mem)->ctls[id&HM_MEM_MASK].list)

static __inline ulong hm_mem_hashcode(const hm_memctl* ctl)
{ 
	return (ulong)ctl;
}

#ifdef __cplusplus
extern "C" {
#endif

int hm_mem_init(hm_mem * mem);
void hm_mem_fini(hm_mem * mem);

void* hm_malloc(ulong id, size_t size);
void hm_free(void* v_objp);

#ifdef __cplusplus
}
#endif

#endif

