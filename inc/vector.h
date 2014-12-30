/*************************************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			29/6/2014
Last modofied date: 6/8/2014
Description: 				sequence of pointers, direct access, grows on demand
*************************************************************************/
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "adterr.h" 	/* ERROR for abstract data type */
#include "my_types.h" /* Item */

typedef struct Vector Vector;

/* _initSize = the initial allocation size
   _extSize = the extention size
	 returns NULL if creation fails */
Vector* VectorCreate(size_t _initSize, size_t _extSize);

/* destroys vector only if all pointer items are NULL 
	NOTE: please delete objects first and use VectorSetNull */
void VectorDestroy(Vector*);

/* adds item (pointer) to vector's end (reallocation on demand) */
ADTErr VectorAdd(Vector* _vec, Item);

/* remove item (pointer) from vector's end and store his value in Item */
ADTErr VectorRemove(Vector*, Item*);

/* returns item (pointer) at _index or NULL if failed */
Item VectorGet(Vector*, size_t _index);

/* vector[_index] will be set with item (pointer) _pItem */
ADTErr VectorSet(Vector*, size_t _index, Item);

/* returns the number of items (pointers) currently in the vector */
size_t VectorCount(Vector*);

/* excecute DoFunc (with supplied Param) for each item in Vector
	 until DoFunc return 0 and the Vector index is returned
	 or -1 if CompFunc never returns 0 */
int VectorForEach(Vector*, DoFunc, Param); 

#ifdef __DEBUG__

/* prints vector's information */
void VectorPrintInfo(Vector*);

#endif /* #ifdef __DEBUG__ */


#endif /* #ifndef __VECTOR_H__ */


