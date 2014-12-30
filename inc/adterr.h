/**********************************************************************
Author: 			 			Lior Bondorevsky
Creation date: 			29/6/2014
Last modofied date: 28/8/2014
Description: 				ERROR for abstract data types
***********************************************************************/
#ifndef __ADTERR_H__
#define __ADTERR_H__


typedef enum ADTErr 
{
	ERR_OK,
	ERR_ALLOC,
	ERR_INVALID_INPUT,
	ERR_UNDERFLOW,
	ERR_OVERFLOW,
	ERR_DUPLICATION
} ADTErr;

#endif /* #ifndef __ADTERR_H__ */
