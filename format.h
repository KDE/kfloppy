/*

    $Id$

    Copyright (C) 2002 Adriaan de Groot
                       groot@kde.org


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

/*
** This file defines a hierarchy of classes that
** can run a sequence of external programs (like
** fdformat, mkisofs, etc.) in sequence. Stdout
** and stderr of those programs can be captured
** and analyzed in order to provide feedback to
** the user.
**
** KFAction            Base class, just for performing some action.
**   KFActionQueue     Provides sequencing of KFActions
**   FloppyAction      Weird name; handles running a program,
**                     understands FD device names. This can be
**                     considered the "useful" base class of
**                     programming actions.
**     FDFormat        Runs fdformat(1) under BSD or Linux
**     FATFilesystem   Creates an msdos (FAT) filesystem
**     Ext2Filesystem  Creates ext2 filesystems, under Linux.
**     UFSFilesystem   Creates UFS filesystem, under BSD.
**
**
** Maybe this is overkill, since for floppies all you need is 
** fdformat(1) and some create-filesystem program like newfs(1)
** or mke2fs(1). However, for Zip disks, should they ever be supported,
** this is quite useful since you need to dd, fdisk, disklabel, and
** newfs them.
*/

#include "debug.h"
#include <qobject.h>

/*
** Abstract base class of actions to be undertaken.
** Basically you can create a KFActionStack (See below)
** and push a bunch of actions on it, then run exec()
** on the stack and wait for the done() signal.
*/
class KFAction : public QObject
{
Q_OBJECT

public:
	KFAction(QObject *parent = 0L);
	virtual ~KFAction();
	
public slots:
	/*
	** Exec() should return quickly to ensire that the GUI
	** thread stays alive. quit() should abort the action.
	*/
	virtual void exec();
	/*
	** Quit aborts the action. No done() signal should
	** be emitted.
	*/
	virtual void quit();
	
signals:
	/*
	** done() should always be emitted with this as first
	** parameter, to avoid sender() magic and the like.
	** @p success indicates whether the action was
	** succesful.
	*/
	void done(KFAction *me,bool success);
	
	/*
	** Emit this signal to inform the user of interesting
	** changes; setting msg to an empty string doesn't
	** change any visible user message. @p progress
	** indicates the action's progress (if that can be determined)
	** and sending -1 leaves the visible indicator unchanged.
	**
	** error(), on the other hand, displays a box. It interrupts
	** the user's work and should be used with care.
	*/
	void status(const QString &msg, int progress);
	void error(const QString &msg, const QString &details);
} ;


/*
** Acts as a queue and executes the actions in the
** queue in FIFO order.
*/

class KFActionQueue : public KFAction
{
Q_OBJECT

public:
	KFActionQueue(QObject *parent = 0L);
	virtual ~KFActionQueue();
	
	/*
	** Add a KFAction to the queue. When exec() is called,
	** the actions are called one after the other (if each
	** action is succesful; if any action fails, the whole
	** queue fails and the unsuccesful action is the last
	** one run.) Actions become the property of the queue
	** action. Note that queues can be nested.
	*/
	void queue(KFAction *);
	
	virtual void exec();

protected slots:
	void actionDone(KFAction *,bool);
	
private:
	class KFActionQueue_p *d;
} ;


/*
** The remainder of the Actions are concrete ones and
** might better go off to live in another header file,
** but this is only needed if the number of supported
** formats grows enormously.
*/

/*
** Description structure for floppy devices.
** devices is a list of possible device names (yay
** /dev/ consistency) while drive,blocks denotes
** fd0 or fd1, and the size of the disk (ought to
** be 1440, 1200, 720 or 260. I've never seen a 2880
** floppy drive).
**
** Tracks is pretty bogus; see the internal code for its use.
** Similarly, flags are internal too.
*/

typedef struct { const char **devices;
	int drive;
	int blocks;
	int tracks;
	int flags; } fdinfo;

/*
** Concrete action for running a single external program.
**
** Not a Q_OBJECT because everything slotlike is
** overridden through virtual slots instead of
** through Qt mechanisms.
*/

class KProcess;

class FloppyAction : public KFAction
{
Q_OBJECT

public:
	FloppyAction(QObject *parent = 0L);
	
	/*
	** Kills the running process, if one exists.
	*/
	virtual void quit();
	
	/*
	** ConfigureDevice() needs to be called prior to exec()
	** or exec() will fail; this indicates which drive and
	** density to use. This same function needs to be
	** called on all subclasses in order to configure them
	** for which drive to use, _along_ with their local
	** configuration functions.
	*/
	
	bool configureDevice(int driveno /* 0 or 1 */, int density /* in kb */ );

protected:
	fdinfo *deviceInfo;      // Configuration info
	const char *deviceName;  // Ptr. into list of "/dev/..." entries

protected slots:
	virtual void processDone(KProcess *);
	/* Provide separate handling of stdout and stderr.
	** The default implementation just sends stderr on
	** to processStdOut(), so you need reimplement only
	** one if you choose.
	*/
	virtual void processStdOut(KProcess *, char *, int);
	virtual void processStdErr(KProcess *, char *, int);
	
protected:
	KProcess *theProcess;
	QString theProcessName;  // human-readable

	/*
	** Sets up connections, calls KProcess::run(). 
	** You need to *theProcess << program << args ; first.
	*/
		
	bool startProcess();
} ;

/*
** Concrete class that runs fdformat(1)
*/

class FDFormat : public FloppyAction
{
public:
	FDFormat(QObject *parent = 0L);
	
	virtual void exec();

	
	/*
	** Concrete classes can provide a runtimeCheck
	** function (heck, this is static, so the name
	** is up to you) that checks if the required 
	** applications are available. This way, the
	** calling application can decide not to use
	** actions whose prerequisites are absent anyway.
	*/
	static bool runtimeCheck();
		
	/* @p verify instructs fdformat(1) to verify the
	** medium as well.
	*/
	
	bool configure(bool verify);
	
	virtual void processStdOut(KProcess *, char *,int);

protected:
	static QString fdformatName;    // path to executable.
	int formatTrackCount;    // How many tracks formatted.
	bool doVerify;
} ;



/*
** Create an msdos (FAT) filesystem on the floppy.
*/
class FATFilesystem : public FloppyAction
{
public:
	FATFilesystem(QObject *parent = 0L);
	
	virtual void exec();
	
	static bool runtimeCheck();

	/*
	** newfs_msdos(1) doesn't support an additional verify,
	** but Linux mkdosfs(1) does. Enable additional medium
	** verify with @p verify. Disks can be labeled with the
	** remaining parameters.
	*/	
	bool configure(bool verify, bool label, const QString &l);
	
protected:
	static QString newfs_fat;
	
	bool doVerify,doLabel;
	QString label;
	
} ;

/*
** The remaining "native" filesystems only get
** compiled on systems where they seem reasonable.
** In particular, Linux gets ext2 and BSD UFS.
**
** There is no real reason to forbid ext2 on BSD,
** the code is the same and only one additional
** #ifdef is needed in the code to look for newfs_ext2(1)
** instead of mke2fs(1).
*/

#ifdef ANY_LINUX
class Ext2Filesystem : public FloppyAction
{
public:
	Ext2Filesystem(QObject *parent = 0L);
	
	virtual void exec();
	
	static bool runtimeCheck();
	
	/* Same args as FATFilesystem. */
	bool configure(bool verify, bool label, const QString &l);
	
protected:
	static QString newfs;
	
	bool doVerify,doLabel;
	QString label;
} ;
#endif

#ifdef ANY_BSD
class UFSFilesystem : public FloppyAction
{
public:
	UFSFilesystem(QObject *parent = 0L);
	
	virtual void exec();
	
	static bool runtimeCheck();
	
protected:
	static QString newfs;
	
	bool doVerify,doLabel;
	QString label;
} ;
#endif


/*
** Utility function that looks for executables in $PATH
** and in /sbin and /usr/sbin.
*/

QString findExecutable(const QString &);

