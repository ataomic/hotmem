
#include "hm_def.h"
#include "hm_osi.h"
#include "hm_mem.h"

#define HM_MEM_SIZE(size) (sizeof(hm_memctl)+(size))
#define HM_MEM_ADDR(ctl) ((size_t)(ctl)+sizeof(hm_memctl))
#define HM_MEM_CTL(objp) ((hm_memctl* )((size_t)(objp)-sizeof(hm_memctl)))

int hm_mem_init(hm_mem* mem)
{
	int index;
		
	mem->count = 0;
	mem->ctls = k_malloc(HM_MEM_MAX*sizeof(dlist_t));
	if(!mem)
		return -1;

	for(index = 0; index < HM_MEM_MAX; index ++)
		dlist_init_head(&mem->ctls[index]);
		
	return 0;
}

void hm_mem_fini(hm_mem* mem)
{
	int index = 0;
	dlist_t *pos, *next;

	if(mem->count > 0) {
		for(index = 0; index < HM_MEM_MAX; index ++) {
			dlist_for_each_safe(pos, next, &mem->ctls[index]) {
				dlist_del(&((hm_memctl* )pos)->list);
				u_free(pos);
				mem->count --;
				if(mem->count == 0)
					break;
			}
		}
	}
	
	k_free(mem->ctls);
}

static __inline hm_memctl* hm_mem_search(const hm_mem* mem, unsigned int id)
{
	dlist_t *pos, *head;

	head = hm_mem_head(mem);
	dlist_for_each(pos, head) {
		if(((hm_memctl* )pos)->id == id)
			return (hm_memctl* )pos;
	}

	return 0;
}

void* hm_malloc(unsigned int id, size_t size)
{
	hm_task* task;
	hm_memctl* ctl;
		
	task = hm_task_current();
	if(task) {
		ctl = hm_mem_search(&task->mem, id);
		if(ctl)
			return HM_MEM_ADDR(ctl);
	}
	else if(!(task = hm_task_register()))
			return 0;

	ctl = (hm_memctl* )u_malloc(HM_MEM_SIZE(size));
	if(ctl) {
		ctl->id = id;
		
		dlist_add((dlist_t* )ctl, &hm_mem_head(&task->mem, id));
		
		return HM_MEM_ADDR(ctl);
	}
	else
		return 0;
}

void hm_free(void* objp)
{
	if(objp) {
		hm_memctl* ctl = HM_MEM_CTL(objp);
		dlist_del((dlist_t* )ctl);
		u_free(ctl);
	}
}

