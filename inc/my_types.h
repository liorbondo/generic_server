/**************************************
Author: 			 			Lior Bondorevsky
Creation date: 			8/8/2014
Last modofied date: 31/8/2014
Description: 				My typedefs
**************************************/
#ifndef __MY_TYPES_H__
#define __MY_TYPES_H__


/** GENERAL **/

typedef unsigned int UInt;
typedef void* 			 Item;
typedef const void*  ConstItem;
typedef const void*	 Param;


/** FUNCTIONS **/

/* return value: =0 -> equal
 *							 >0 -> left argument bigger 
 *							 <0 -> right argument bigger */
typedef int	(*CompFunc) (ConstItem, ConstItem);  
/* return value: 0 --> error, else success */
typedef int	(*DoFunc) (Item, Param);	
	
typedef void (*PrintFunc)(ConstItem);			 

#endif /* #ifndef __MY_TYPES_H__ */


