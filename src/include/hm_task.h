#ifndef RS_HM_TASK_H
#define RS_HM_TASK_H

typedef struct hm_task_s {
	list list;
	hm_atom id;
	hm_mem mem;
}hm_task;

#define HM_TASK_MASK 0x3fful
#define HM_TASK_MAX (HM_TASK_MASK+1)

#define hm_task_head(tasks, hashcode) ((tasks)+((hashcode)&HM_TASK_MASK))

/* for debugging */
static __inline hm_task* hm_task_current() 
{
	return hm_task_search(hm_atom_current());
}

#ifdef __cplusplus
extern "C" {
#endif

int hm_task_register();
hm_task* hm_task_search(hm_atom atom);

#ifdef __cplusplus
}
#endif

#endif

