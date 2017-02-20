#include "hm_def.h"
#include "hm_osi.h"

#include "hm_mem.h"
#include "hm_task.h"

static dlist_t hm_tasks[HM_TASK_MAX] = { 0 };

#define HM_TASK_HEAD(i) hm_task_head(hm_tasks, i)

int hm_task_initialize()
{
	int index;

	for(index = 0; index < HM_TASK_MAX; index ++)
		dlist_init_head(HM_TASK_HEAD(index));
}

int hm_task_register()
{
	int index;
	dlist_t *head, *pos;
	hm_atom atom;
	hm_task* task;
	
	atom = hm_atom_current();
	assert(!hm_task_search(atom));

	task = k_malloc(sizeof(hm_task));
	if(task) {
		task->id = atom;
		if(!hm_mem_init(&task->mem)) {
			dlist_add((dlist_t* )task, HM_TASK_HEAD(hm_atom_hashcode(atom)));
			return 0;
		}
		else
			k_free(task);
	}

	return -1;
}

hm_task* hm_task_search(hm_atom atom)
{
	hm_task* task;
	dlist_t *pos, 
		*head = HM_TASK_HEAD(hm_atom_hashcode(atom));
	
	dlist_for_each(pos, head) {
		task = (hm_task* )pos;
		if(!hm_atom_compare(task->atom, atom))
			return task;
	}

	return NULL;
}

