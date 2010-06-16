/* share/inc/type.h - NMF Definition.
 *
 * Copyright (c) 2006, 2007, 2008 STMicroelectronics. All Rights Reserved.
 *
 * This file is part of the ST Nomadik Multiprocessing Framework.
 *
 * The NMF Definition is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published by 
 * the Free Software Foundation; version 2.
 * 
 * The NMF Definition is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License 
 * along with the NMF Definition; see the file lgpl-2.1.txt.  If not, write to the Free
 * Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 * 
 * Written by NMF team.
 */
#ifndef _COMMON_TYPE_H_
#define _COMMON_TYPE_H_ 

/*
 * Definition of type that are used by interface.
 */
typedef unsigned int t_uword;
typedef signed int t_sword;

#ifdef __flexcc2__

typedef unsigned char t_bool;

#ifdef __mode16__

typedef signed char t_sint8;
typedef signed int t_sint16;
typedef signed long t_sint24;
typedef signed long t_sint32;
typedef signed long long t_sint40;
// bigger type are not handle on this mode

typedef unsigned char t_uint8;
typedef unsigned int t_uint16;
typedef unsigned long t_uint24;
typedef unsigned long t_uint32;
typedef unsigned long long t_uint40;
// bigger type are not handle on this mode

// shared addr type definition
//typedef __SHARED16 t_uint16 * t_shared_addr;
typedef void * t_shared_field;

#else /* __mode16__ -> __mode24__ */

typedef signed char t_sint8;
typedef signed short t_sint16;
typedef signed int t_sint24;
typedef signed long t_sint32;
typedef signed long t_sint40;
typedef signed long t_sint48;
typedef signed long long t_sint56;

typedef unsigned char t_uint8;
typedef unsigned short t_uint16;
typedef unsigned int t_uint24;
typedef unsigned long t_uint32;
typedef unsigned long t_uint40;
typedef unsigned long t_uint48;
typedef unsigned long long t_uint56;

// shared addr type definition
//typedef __SHARED16 t_uint16 * t_shared_addr;
typedef t_uint24 t_shared_field;

#endif /* MMDSP mode24 */

// shared register (ARM world) type definition
#if 0
typedef struct {
    t_uint16 lsb;
    t_uint16 msb;
} t_shared_reg;
#endif
typedef t_uint32 t_shared_reg;

typedef t_uint32 t_physical_address;

#include <stwdsp.h>

#else /* __flexcc2__ -> RISC 32 Bits */

#ifndef _HCL_DEFS_H
typedef unsigned char t_bool;

typedef unsigned char t_uint8;
typedef signed char t_sint8;
typedef unsigned short t_uint16;
typedef signed short t_sint16;
typedef unsigned long t_uint32;
typedef signed long t_sint32;

typedef unsigned long long t_uint64;
typedef signed long long t_sint64;

typedef t_uint32 t_physical_address;
#endif /* _HCL_DEFS_H */

typedef unsigned long t_uint24;
typedef signed long t_sint24;

// shared addr type definition
typedef t_uint32 t_shared_addr;

// shared register (ARM world) type definition
typedef t_uint32 t_shared_reg;
typedef t_uint32 t_shared_field;

#endif /* RISC 32 Bits */

#endif /* _COMMON_TYPE_H_ */
/* End of file type.h */

