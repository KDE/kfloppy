/* debug.h
**
** Copyright (C) 2003 Adriaan de Groot <groot@kde.org>
**
** Debugging definitions for KFloppy. It also tries to map
** OSsen into families, so you can use ANY_LINUX or ANY_BSD
** in the code to differentiate those families. What happens
** on other systems is anyone's guess.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/


#ifndef DEBUG_H
#define DEBUG_H


#define KFAREA		(2002)

#ifndef NDEBUG
#define DEBUGSETUP	kdDebug(KFAREA) << (__PRETTY_FUNCTION__) << endl
#define DEBUGS(a)	kdDebug(KFAREA) << "  " << a << endl
#define DEBUGSZ(s)	kdDebug(KFAREA) << QString("  Size %1x%2.\n").arg(s.width()).arg(s.height());
#else
#define DEBUGSETUP
#define DEBUGS(a)
#define DEBUGSZ(s)
#endif


// Detect vaguely what OS we're working with. Map variants
// to one known kind.
//
//
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define ANY_BSD	(1)
#else
#if defined(linux) || defined(LINUX) || defined (__linux) || defined(__linux__)
#define ANY_LINUX (1)
#endif
#endif



#endif

