/*****************************************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			7/7/2014
Last modofied date: 19/8/2014
Description: 				Doubly linked list of pointers (Items), grows on demand.
Notes:						  ListEnd is outside of the list (STL style).
*****************************************************************************/
#ifndef __LIST_H__
#define __LIST_H__

#include "my_types.h" 

typedef struct List List;
typedef void*  ListItr;

/* creates list. return NULL on failure */
List* ListCreate();

/* destroys list (not including Items) */
void ListDestroy(List*);

/* return ListItr to _list first item */ 
ListItr ListBegin(List*);

/* return ListItr _list end --> outside of the list (STL style) */ 
ListItr ListEnd(List*);

/* insert _item before _listItr location */ 
ListItr ListInsert(ListItr, ConstItem);

/* removes _item from _listItr location and return it, or NULL on failure
 * WARNING: ListEnd is outside of the List and not the last item ! */ 
Item ListRemove(ListItr);

/* return the Item located at _listItr or NULL on failure */
Item ListGetItem(ListItr);

/* return iterator for next item */ 
ListItr ListNext(ListItr);

/* return iterator for previous item */ 
ListItr ListPrev(ListItr);

/* return TRUE if the 2 iterators is the same and FALSE if not */  
int ListIsSame(ListItr, ListItr);

/* push _item to head of the list */
ListItr ListPushHead(List*, ConstItem);

/* push _item to tail of the list */
ListItr ListPushTail(List*, ConstItem);

/* pop item from head of the list
 * return the popped Item, or NULL on failure */
Item ListPopHead(List*);

/* pop item from tail of the list
 * return the popped Item, or NULL on failure */
Item ListPopTail(List*);

/* return List number of items */
size_t ListCountItems(List*);

/* return 1 if List is empty and 0 otherwise */
int ListIsEmpty(List*);

/* return ListItr to the first item in list
 * for which CompFunc returns 0 */
ListItr ListFindFirst(ListItr _from, ListItr _to, CompFunc, ConstItem); 

/* perform specified DoFunc for each item in the
 * collection until error is returned from the DoFunc
 * returns iterator to last visited node */
ListItr	ListForEach(ListItr _from, ListItr _to, DoFunc, Param);

#endif /* #ifndef __LIST_H__ */

