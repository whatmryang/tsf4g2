#if 0

#include "comm/comm.h"
#include "tlist_i.h"
#include "tmempool_i.h"

#define TLISTINIT()

#define TLISTNEXT(pstlistNode) ((LPTLISTNODE)((size_t)(pstlistNode)+(pstlistNode->stNext.iOffset)))
#define TLISTSETNEXT(pstListNode,pstNext) 

#define TLISTPREV(pstlistNode) ((LPTLISTNODE)((size_t)(pstlistNode)+(pstlistNode->stPrev.iOffset)))
#define TLISTSETPREV(pstListNode,pstPrev) (pstListNode->stPrev.iOffset)

int tlist_new(LPTLIST *ppstList,void *pvBase)
{
	if(!ppstList)
	{
		return -1;
	}
	
	*ppstList = (LPTLIST)calloc(1,sizeof(TLIST));
	if(*ppstList)
	{
		((*ppstList)->iNeedFree) = 1;
		INITNODE((*ppstList),(*ppstList)->stNode);
		return 0;
	}
	return -1;
}

size_t tlist_get_size(void *pvUnused)
{
	return sizeof(TLIST);
}

int tlist_init_from_mem(LPTLIST pstList,void *pstPool)
{
	if(pstList)
	{
		(pstList->iNeedFree) = 0;
//		(pstList)->pvBase = pstPool;
		INITNODE(pstList,(pstList)->stNode);
		return 0;
	}
	return -1;
}

int tlist_fini(LPTLIST *ppstList)
{
	if(ppstList)
	{
		if(*ppstList)
		{
			free(*ppstList);
			return 0;
		}
		return 0;
	}
	return -1;
}

int tlist_is_empty(LPTLIST pstList)
{
	if(((char *)(&pstList->stNode) ==((char *)pstList+pstList->stNode.stNext.iOffset) ) \
		&& ((char *)(&pstList->stNode) ==((char *)pstList+pstList->stNode.stPrev.iOffset)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

LPTLISTNODE tlist_addr2node(LPTLIST pstList,LPADDR pstAddr)
{
	return (LPTLISTNODE)((char *)pstList + pstAddr->iOffset);
}

ADDR tlist_node2addr(LPTLIST pstList,LPTLISTNODE pstNode)
{
	ADDR stAddr;
	stAddr.iOffset = (intptr_t)pstNode - (intptr_t)(pstList);
	return stAddr;
}

int tlist_insert_next(LPTLIST pstList,LPTLISTNODE pstNode)
{
	ADDR stData,stNext,stHead;
	LPTLISTNODE pstData,pstNext,pstHead;
	int iRet;

	pstHead = &(pstList->stNode);
	stHead = tlist_node2addr(pstList,pstHead);
	pstNext = tlist_addr2node(pstList,&(pstHead->stNext));
	stNext = tlist_node2addr(pstList,pstNext);
	pstData = pstNode;
	stData = tlist_node2addr(pstList,pstData);	
	pstData->stNext = stNext;
	pstData->stPrev = stHead;
	pstNext->stPrev = stData;
	pstHead->stNext = stData;
	
	return 0;
}

int tlist_insert_prev(LPTLIST pstList,LPTLISTNODE pstNode)
{
	ADDR stData,stPrev,stHead;
	LPTLISTNODE pstPrev,pstHead;
	int iRet;

	pstHead = &(pstList->stNode);
	stHead = tlist_node2addr(pstList,pstHead);
	pstPrev = tlist_addr2node(pstList,&(pstHead->stPrev));
	stPrev = tlist_node2addr(pstList,pstPrev);
	stData = tlist_node2addr(pstList,pstNode);	

	pstNode->stNext = stHead;
	pstNode->stPrev = stPrev;

	pstPrev->stNext = stData;
	pstHead->stPrev = stData;
	
	return 0;
}

int tlist_remove(LPTLIST pstList,LPTLISTNODE pstNode)
{
	LPTLISTNODE pstData,pstPrev,pstNext;
	pstData = pstNode;
	pstPrev = tlist_addr2node(pstList,&(pstData->stPrev));
	pstNext = tlist_addr2node(pstList,&(pstData->stNext));
	pstPrev->stNext = pstData->stNext;
	pstNext->stPrev = pstData->stPrev;
	INITNODE(pstList,*pstNode);
	return 0;
}

int tlist_for_each(LPTLIST pstList,TLIST_CALLBACK callback,void *data)
{
	ADDR stCurr,stNext;
	LPTLISTNODE pstCurr;
	int iRet;
	void *pvNodeData;
	stCurr = pstList->stNode.stNext;

	while(((char *)pstList + stCurr.iOffset ) != (char *)&pstList->stNode)
	{
		pstCurr = tlist_addr2node( pstList, &stCurr);
		stNext = pstCurr->stNext;
		if(pstCurr)
		{
			if(NULL != callback)
			{
				iRet = callback(pstList,pstCurr,data);
				if(iRet)
				{
					return iRet;
				}
			}
		}
		stCurr = stNext;
	}
	return 0;
}

LPTLISTNODE tlist_get_tail(LPTLIST pstList)
{
	ADDR stTail;
	if(tlist_is_empty(pstList))
	{
		return NULL;
	}

	stTail = pstList->stNode.stPrev;
	return tlist_addr2node(pstList,&stTail);
}

LPTLISTNODE tlist_get_head(LPTLIST pstList)
{
	ADDR stTail;
	if(tlist_is_empty(pstList))
	{
		return NULL;
	}

	stTail = pstList->stNode.stNext;
	return tlist_addr2node(pstList,&stTail);
}

#if 0
int tlist_check_list(LPTLIST pstList)
{
	//I have to check the list.
	ADDR stCurr,stNext;
	LPTLISTNODE pstCurr;
	
	stCurr = pstList->stNode.stNext;

	while(stCurr.idx > 0)
	{
		pstCurr = tlist_addr2node( pstList, &stCurr);
		stNext = pstCurr->stNext;

		pstCurr->iCheckFlag = 0;
		stCurr = stNext;
	}

	//Make sure there are no overlayed element.
	stCurr = pstList->stNode.stNext;

	while(stCurr.idx > 0)
	{
		pstCurr = tlist_addr2node( pstList, &stCurr);
		stNext = pstCurr->stNext;

		pstCurr->iCheckFlag ++;
		stCurr = stNext;
	}

	stCurr = pstList->stNode.stNext;

	while(stCurr.idx > 0)
	{
		pstCurr = tlist_addr2node( pstList, &stCurr);
		stNext = pstCurr->stNext;

		if(pstCurr->iCheckFlag != 1)
		{
			return -1;
		}
		stCurr = stNext;
	}

	return 0;
}
#endif

#if 0
/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct tlist_head *list,
				  struct tlist_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct tlist_head *list,
				const struct tlist_head *head)
{
	return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct tlist_head *head)
{
	return head->next == head;
}

/**
 * list_empty_careful - tests whether a list is empty and not being modified
 * @head: the list to test
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
static inline int list_empty_careful(const struct tlist_head *head)
{
	struct tlist_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int list_is_singular(const struct tlist_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct tlist_head *list,
		struct tlist_head *head, struct tlist_head *entry)
{
	struct tlist_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct tlist_head *list,
		struct tlist_head *head, struct tlist_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

static inline void __list_splice(const struct tlist_head *list,
				 struct tlist_head *prev,
				 struct tlist_head *next)
{
	struct tlist_head *first = list->next;
	struct tlist_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice(const struct tlist_head *list,
				struct tlist_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void list_splice_tail(struct tlist_head *list,
				struct tlist_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head->prev, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct tlist_head *list,
				    struct tlist_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct tlist_head *list,
					 struct tlist_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct tlist_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct tlist_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; prefetch(pos->next), pos != (head); \
        	pos = pos->next)

/**
 * __list_for_each	-	iterate over a list
 * @pos:	the &struct tlist_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct tlist_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
        	pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct tlist_head to use as a loop cursor.
 * @n:		another &struct tlist_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct tlist_head to use as a loop cursor.
 * @n:		another &struct tlist_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     prefetch(pos->prev), pos != (head); \
	     pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head); 	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);	\
	     prefetch(pos->member.prev), &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member) 			\
	for (; prefetch(pos->member.next), &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member), 		\
		n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node *n,
					struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void hlist_add_after(struct hlist_node *n,
					struct hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

/*
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move_list(struct hlist_head *old,
				   struct hlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos && ({ prefetch(pos->next); 1; }); \
	     pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(tpos, pos, member)		 \
	for (pos = (pos)->next;						 \
	     pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(tpos, pos, member)			 \
	for (; pos && ({ prefetch(pos->next); 1;}) &&			 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct hlist_node to use as a loop cursor.
 * @n:		another &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(tpos, pos, n, head, member) 		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) && 				 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)

#endif


#endif
