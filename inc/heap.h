/***********************************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			10/7/2014
Last modofied date: 28/8/2014
Description: 				Heap of Items, grows on demand.
Notes:              Policy is determined by CompFunc supplied by user.		
***********************************************************************/
#ifndef __HEAP_H__
#define __HEAP_H__

#include "adterr.h" 	/* ADTErr */
#include "my_types.h" /* Item */

typedef struct Heap Heap;

/* Builds Heap from already created Vector & his items (can be empty)
 * according to heap policy (determined by CompFunc), returns NULL if failed 
 * NOTE: Vector will be used & changed by the heap (Items remains as given) */
Heap* HeapBuild(Vector*, CompFunc);

/* Destroys Heap (not including given Vector & Items)  */
void HeapDestroy(Heap*);

/* Return Heap max Item */
Item HeapGetMax(Heap*);

/* Remove Heap max and store his value in Item 
 * Maintains the Heap policy */
ADTErr HeapRemoveMax(Heap*, Item*);

/* Insert new Item to Heap and maintains the Heap policy */
ADTErr HeapInsert(Heap*, Item);

/* Returns Heap number of items */
size_t HeapCount(Heap*);

#ifdef __DEBUG__

/* DEBUG ONLY: Prints the heap (int Items only) */
void HeapPrint(Heap*);

#endif /* #ifdef __DEBUG__ */

#endif /* #ifndef __HEAP_H__ */


