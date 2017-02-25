#ifndef LIST_H_INCLUDED_
#define LIST_H_INCLUDED_

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
#include "stddef.h"

#ifdef CONFIG_DEBUG_LIST
 /*
  * These are non-NULL pointers that will result in page faults
  * under normal circumstances, used to verify that nobody uses
  * non-initialized list entries.
  */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)
#else
#define LIST_POISON1  NULL
#define LIST_POISON2  NULL
#endif

typedef struct list_s {
	struct list_s* next;
	struct list_s* prev;
} list_t;

#define LIST_INIT(name) { &(name), &(name) }

#define LIST_DEF(name) \
	list_t name = LIST_INIT(name)

static inline void INIT_LIST(list_t *list)
{
	WRITE_ONCE(list->next, list);
	list->prev = list;
}

#ifdef CONFIG_DEBUG_LIST
extern bool __list_add_valid(list_t *entry, list_t *prev, list_t *next);
extern bool __list_del_entry_valid(list_t *entry);
#else
static inline bool __list_add_valid(list_t *entry, list_t *prev,
	list_t *next)
{
	return true;
}
static inline bool __list_del_entry_valid(list_t *entry)
{
	return true;
}
#endif

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(list_t *entry, list_t *prev, list_t *next)
{
	if (!__list_add_valid(entry, prev, next))
		return;

	next->prev = entry;
	entry->next = next;
	entry->prev = prev;
	WRITE_ONCE(prev->next, entry);
}

/**
 * list_add - add a new entry
 * @entry: new entry to be added
 * @list: list to add it after
 *
 * Insert a new entry after the specified list.
 * This is good for implementing stacks.
 */
static inline void list_add(list_t *entry, list_t *list)
{
	__list_add(entry, list, list->next);
}


/**
 * list_add_tail - add a new entry
 * @entry: new entry to be added
 * @list: list to add it before
 *
 * Insert a new entry before the specified list.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(list_t *entry, list_t *list)
{
	__list_add(entry, list->prev, list);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(list_t *prev, list_t *next)
{
	next->prev = prev;
	WRITE_ONCE(prev->next, next);
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(list_t *entry)
{
	if (!__list_del_entry_valid(entry))
		return;

	__list_del(entry->prev, entry->next);
}

static inline void list_del(list_t *entry)
{
	__list_del_entry(entry);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @entry : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(list_t *old, list_t *entry)
{
	entry->next = old->next;
	entry->next->prev = entry;
	entry->prev = old->prev;
	entry->prev->next = entry;
}

static inline void list_replace_init(list_t *old, list_t *entry)
{
	list_replace(old, entry);
	INIT_LIST(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(list_t *entry)
{
	__list_del_entry(entry);
	INIT_LIST(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @entry: the entry to move
 * @list: the list that will precede our entry
 */
static inline void list_move(list_t *entry, list_t *list)
{
	__list_del_entry(entry);
	list_add(entry, list);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @entry: the entry to move
 * @list: the list that will follow our entry
 */
static inline void list_move_tail(list_t *entry, list_t *list)
{
	__list_del_entry(entry);
	list_add_tail(entry, list);
}

/**
 * list_is_last - tests whether @entry is the last entry in @list
 * @entry: the entry to test
 * @list: the list
 */
static inline int list_is_last(const list_t *entry, const list_t *list)
{
	return entry->next == list;
}

/**
 * list_empty - tests whether a list is empty
 * @list: the list to test.
 */
static inline int list_empty(const list_t *list)
{
	return READ_ONCE(list->next) == list;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @list: the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is list_del_init(). Eg. it cannot be used
 * if another CPU could re-list_add() it.
 */
static inline int list_empty_careful(const list_t *list)
{
	list_t *next = list->next;
	return (next == list) && (next == list->prev);
}

/**
 * list_rotate_left - rotate the list to the left
 * @list: the list
 */
static inline void list_rotate_left(list_t *list)
{
	list_t *first;

	if (!list_empty(list)) {
		first = list->next;
		list_move_tail(first, list);
	}
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @list: the list to test.
 */
static inline int list_is_singular(const list_t *list)
{
	return !list_empty(list) && (list->next == list->prev);
}

static inline void __list_cut_position(list_t *new_list, list_t *list,
	list_t *entry)
{
	list_t *new_first = entry->next;
	new_list->next = list->next;
	new_list->next->prev = new_list;
	new_list->prev = entry;
	entry->next = new_list;
	list->next = new_first;
	new_first->prev = list;
}

/**
 * list_cut_position - cut a list into two
 * @new_list: a new list to add all removed entries
 * @list: a list with entries
 * @entry: an entry within list, could be the list itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @list, up to and
 * including @entry, from @list to @new_list. You should
 * pass on @entry an element you know is on @list. @new_list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(list_t *new_list, list_t *list,
	list_t *entry)
{
	if (list_empty(list))
		return;
	if (list_is_singular(list) &&
		(list->next != entry && list != entry))
		return;
	if (entry == list)
		INIT_LIST(new_list);
	else
		__list_cut_position(new_list, list, entry);
}

static inline void __list_splice(const list_t *list, list_t *prev, list_t *next)
{
	list_t *first = list->next;
	list_t *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @old_list: the old list to add.
 * @list: the place to add it.
 */
static inline void list_splice(const list_t *old_list, list_t *list)
{
	if (!list_empty(old_list))
		__list_splice(old_list, list, list->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @old_list: the old list to add.
 * @list: the place to add it.
 */
static inline void list_splice_tail(list_t *old_list, list_t *list)
{
	if (!list_empty(old_list))
		__list_splice(old_list, list->prev, list);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @old_list: the old list to add.
 * @list: the place to add it.
 *
 * The old list at @old_list is reinitialised
 */
static inline void list_splice_init(list_t *old_list, list_t *list)
{
	if (!list_empty(old_list)) {
		__list_splice(old_list, list, list->next);
		INIT_LIST(old_list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @old_list: the old list to add.
 * @list: the place to add it.
 *
 * Each of the lists is a queue.
 * The old list at @old_list is reinitialised
 */
static inline void list_splice_tail_init(list_t *old_list, list_t *list)
{
	if (!list_empty(old_list)) {
		__list_splice(old_list, list->prev, list);
		INIT_LIST(old_list);
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &list_t pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list entry to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list entry to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list entry to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
	list_t *head__ = (ptr); \
	list_t *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})

/**
 * list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_head within the struct.
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &list_t to use as a loop cursor.
 * @list:	the list.
 */
#define list_for_each(pos, list) \
	for (pos = (list)->next; pos != (list); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &list_t to use as a loop cursor.
 * @list:	the list.
 */
#define list_for_each_prev(pos, list) \
	for (pos = (list)->prev; pos != (list); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &list_t to use as a loop cursor.
 * @n:		another &list_t to use as temporary storage
 * @list:	the list.
 */
#define list_for_each_safe(pos, n, list) \
	for (pos = (list)->next, n = pos->next; pos != (list); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &list_t to use as a loop cursor.
 * @n:		another &list_t to use as temporary storage
 * @list:	the list.
 */
#define list_for_each_prev_safe(pos, n, list) \
	for (pos = (list)->prev, n = pos->prev; \
	     pos != (list); \
	     pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, list, member)				\
	for (pos = list_first_entry(list, typeof(*pos), member);	\
	     &pos->member != (list);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, list, member)			\
	for (pos = list_last_entry(list, typeof(*pos), member);		\
	     &pos->member != (list); 					\
	     pos = list_prev_entry(pos, member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @list:	the list of the list
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, list, member) \
	((pos) ? : list_entry(list, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, list, member) 		\
	for (pos = list_next_entry(pos, member);			\
	     &pos->member != (list);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, list, member)		\
	for (pos = list_prev_entry(pos, member);			\
	     &pos->member != (list);					\
	     pos = list_prev_entry(pos, member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, list, member) 			\
	for (; &pos->member != (list);					\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, list, member)			\
	for (pos = list_first_entry(list, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     &pos->member != (list); 					\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, list, member) 		\
	for (pos = list_next_entry(pos, member), 				\
		n = list_next_entry(pos, member);				\
	     &pos->member != (list);						\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, list, member) 			\
	for (n = list_next_entry(pos, member);					\
	     &pos->member != (list);						\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @list:	the list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, list, member)		\
	for (pos = list_last_entry(list, typeof(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     &pos->member != (list); 					\
	     pos = n, n = list_prev_entry(n, member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:	the loop cursor used in the list_for_each_entry_safe loop
 * @n:		temporary storage used in list_for_each_entry_safe
 * @member:	the name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)				\
	n = list_next_entry(pos, member)

#define list_init(list) INIT_LIST(list)
#define list_insert(list, n) list_add(n, list)
#define list_erase(n) list_del(n)

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list node is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

typedef struct hlist_node_s {
	struct hlist_node_s* next;
	struct hlist_node_s** pprev;
} hlist_node_t;

typedef struct hlist_s {
	struct hlist_node_s* first;
} hlist_t;

#define HLIST_INIT { .first = NULL }
#define HLIST_DEF(name) hlist_t name = {  .first = NULL }
#define INIT_HLIST(ptr) ((ptr)->first = NULL)
static inline void INIT_hlist_node_t(hlist_node_t *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const hlist_node_t *h)
{
	return !h->pprev;
}

static inline int hlist_empty(const hlist_t *h)
{
	return !READ_ONCE(h->first);
}

static inline void __hlist_del(hlist_node_t *n)
{
	hlist_node_t *next = n->next;
	hlist_node_t **pprev = n->pprev;

	WRITE_ONCE(*pprev, next);
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(hlist_node_t *n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlist_del_init(hlist_node_t *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_hlist_node_t(n);
	}
}

static inline void hlist_add_head(hlist_node_t *n, hlist_t *h)
{
	hlist_node_t *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	WRITE_ONCE(h->first, n);
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(hlist_node_t *n, hlist_node_t *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	WRITE_ONCE(*(n->pprev), n);
}

static inline void hlist_add_behind(hlist_node_t *n, hlist_node_t *prev)
{
	n->next = prev->next;
	WRITE_ONCE(prev->next, n);
	n->pprev = &prev->next;

	if (n->next)
		n->next->pprev  = &n->next;
}

/* after that we'll appear to be on some hlist and hlist_del will work */
static inline void hlist_add_fake(hlist_node_t *n)
{
	n->pprev = &n->next;
}

static inline bool hlist_fake(hlist_node_t *h)
{
	return h->pprev == &h->next;
}

/*
 * Check whether the node is the only node of the list without
 * accessing list:
 */
static inline bool
hlist_is_singular(hlist_node_t *n, hlist_t *h)
{
	return !n->next && n->pprev == &h->first;
}

/*
 * Move a list from one list to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move(hlist_t *old_list, hlist_t *list)
{
	list->first = old_list->first;
	if (list->first)
		list->first->pprev = &list->first;
	old_list->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, list) \
	for (pos = (list)->first; pos ; pos = pos->next)

#define hlist_for_each_safe(pos, n, list) \
	for (pos = (list)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

#define hlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
	   ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
	})

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @list:	the list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(pos, list, member)				\
	for (pos = hlist_entry_safe((list)->first, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(pos, member)			\
	for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
	     pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(pos, member)				\
	for (; pos;							\
	     pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another &hlist_node_t to use as temporary storage
 * @list:	the list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(pos, n, list, member) 		\
	for (pos = hlist_entry_safe((list)->first, typeof(*pos), member);\
	     pos && ({ n = pos->member.next; 1; });			\
	     pos = hlist_entry_safe(n, typeof(*pos), member))

#endif

