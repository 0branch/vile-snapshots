/*
 * $Id: vl_alloc.h,v 1.8 2018/10/21 19:16:43 tom Exp $
 *
 * Copyright 2005-2010,2018 Thomas E. Dickey and Paul G. Fox
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, distribute with modifications, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization.
 */

#ifndef VL_ALLOC_H_incl
#define VL_ALLOC_H_incl 1

/* structure-allocate, for appeasing strict compilers */
#define	castalloc(type,nbytes)		(type *)malloc((size_t) nbytes)
#define	castrealloc(type,ptr,nbytes)	(type *)realloc((ptr), (size_t)(nbytes))
#define	typecalloc(type)		(type *)calloc(sizeof(type), (size_t) 1)
#define	typecallocn(type,ntypes)	(type *)calloc(sizeof(type), (size_t) ntypes)
#define	typealloc(type)			(type *)malloc(sizeof(type))
#define	typeallocn(type,ntypes)		(type *)malloc((ntypes)*sizeof(type))
#define	typereallocn(type,ptr,ntypes)	(type *)realloc((ptr),\
							(ntypes)*sizeof(type))
#define	typeallocplus(type,extra)	(type *)calloc((extra)+sizeof(type), (size_t) 1)

#define safe_castrealloc(type,ptr,nbytes) \
	{ \
	    type *safe_ptr = castrealloc(type,ptr,nbytes); \
	    if (safe_ptr == 0) { \
		ptr = 0; \
	    } \
	    ptr = safe_ptr; \
	}

#define safe_typereallocn(type,ptr,ntypes) \
	{ \
	    type *safe_ptr = typereallocn(type,ptr,ntypes); \
	    if (safe_ptr == 0) { \
		free(ptr); \
	    } \
	    ptr = safe_ptr; \
	}

#define	FreeAndNull(p)	if ((p) != 0)	{ free(p); p = 0; }
#define	FreeIfNeeded(p)	if ((p) != 0)	free(p)

#endif /* VL_ALLOC_H_incl */
