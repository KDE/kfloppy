#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#define KFAREA		(2002)

#define DEBUGSETUP	kdDebug(KFAREA) << (__PRETTY_FUNCTION__)
#define DEBUGS(a)	kdDebug(KFAREA) << "  " << a
#define DEBUGSZ(s)	kdDebug(KFAREA) << QString("  Size %1x%2.").arg(s.width()).arg(s.height());
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

