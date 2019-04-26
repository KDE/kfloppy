/*

    This file is part of the KFloppy program, part of the KDE project

    Copyright (C) 2002 Adriaan de Groot <groot@kde.org>
    Copyright (C) 2004, 2005 Nicolas GOUTTE <goutte@kde.org>
    Copyright (C) 2015, 2016 Wolfgang Bauer <wbauer@tmo.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "format.h"

#include <unistd.h>
#include <stdlib.h>

#include <QTimer>
#include <QRegExp>
#include <QStandardPaths>
#include "qplatformdefs.h"

#include <KLocalizedString>
#include <KProcess>

static QStringList extPath = QStringList();

/* static */ QString findExecutable(const QString &e)
{
	if (extPath.isEmpty())
	{
            QStringList path = QString::fromLocal8Bit(qgetenv("PATH")).split(QStringLiteral( ":" ));
            path.append(QStringLiteral( "/usr/sbin" ));
            path.append(QStringLiteral( "/sbin" ));
		extPath = path;
	}

	return QStandardPaths::findExecutable(e, extPath);
}


KFAction::KFAction(QObject *parent) :
	QObject(parent)
{
	DEBUGSETUP;
}

KFAction::~KFAction()
{
	DEBUGSETUP;
	quit();
}

/* slot */ void KFAction::quit()
{
	DEBUGSETUP;
}

/* slot */ void KFAction::exec()
{
	DEBUGSETUP;
}

class KFActionQueue_p
{
public:
	QList<KFAction*> list;
} ;

KFActionQueue::KFActionQueue(QObject *parent) :
	KFAction(parent),
	d(new KFActionQueue_p)
{
	DEBUGSETUP;
}

KFActionQueue::~KFActionQueue()
{
	DEBUGSETUP;
	qDeleteAll(d->list);
	d->list.clear();
	delete d;
}

void KFActionQueue::queue(KFAction *p)
{
	DEBUGSETUP;

	d->list.append(p);
	DEBUGS(p->objectName());
}

/* virtual */ void KFActionQueue::exec()
{
	DEBUGSETUP;

    actionDone(nullptr,true);
}

/* slot */ void KFActionQueue::actionDone(KFAction *p,bool success)
{
	DEBUGSETUP;

	if (p)
	{
		if (!d->list.isEmpty() && d->list.first()==p)
		{
			d->list.removeFirst();
			delete p;
		}
		else
		{
			DEBUGS(  "Strange pointer received.");
			emit done(this,false);
			return;
		}
	}
	else
	{
		DEBUGS("Starting action queue.");
	}

	if (!success)
	{
		DEBUGS("Action failed.");
		emit done(this,false);
		return;
	}

	KFAction *next = d->list.isEmpty() ? nullptr : d->list.first();
	if (!next)
	{
		emit done(this,true);
	}
	else
	{
		qCDebug(KFLOPPY_LOG) << "Running action " << next->objectName() ;
		QObject::connect(next,&KFAction::done,
			this,&KFActionQueue::actionDone);
		// Propagate signals
		QObject::connect(next,&KFAction::status,
			this,&KFAction::status);
		QTimer::singleShot(0,next,&KFAction::exec);
	}
}


// Here we have names of devices. The variable
// names are basically the linux device names,
// replace with whatever your OS needs instead.
//
//
#ifdef ANY_LINUX

const char * const fd0H1440[] = { "/dev/fd0u1440", "/dev/floppy/0u1440", "/dev/fd0h1440", "/dev/fd0H1440", "/dev/fd0", nullptr } ;
const char * const fd0D720[] = { "/dev/fd0u720", "/dev/floppy/0u720", "/dev/fd0D720", "/dev/fd0h720", "/dev/fd0", nullptr };
const char * const fd0h1200[] = { "/dev/fd0h1200", "/dev/floppy/0h1200", "/dev/fd0", nullptr };
const char * const fd0h360[] = { "/dev/fd0u360", "/dev/floppy/0u360", "/dev/fd0h360", "/dev/fd0d360", "/dev/fd0", nullptr };

const char * const fd1H1440[] = { "/dev/fd1u1440", "/dev/floppy/1u1440","/dev/fd1h1440", "/dev/fd1H1440", "/dev/fd1", nullptr } ;
const char * const fd1D720[] = { "/dev/fd1u720", "/dev/floppy/1u720", "/dev/fd1D720", "/dev/fd1h720", "/dev/fd1", nullptr };
const char * const fd1h1200[] = { "/dev/fd1h1200", "/dev/floppy/1h1200", "/dev/fd1", nullptr };
const char * const fd1h360[] = { "/dev/fd1u360", "/dev/floppy/1u360","/dev/fd1h360", "/dev/fd1d360", "/dev/fd1", nullptr };

const char * const fd0auto[] = { "/dev/fd0", nullptr };
const char * const fd1auto[] = { "/dev/fd1", nullptr };

#endif


#ifdef ANY_BSD
const char * const fd0[] = { "/dev/fd0", nullptr } ;
const char * const fd1[] = { "/dev/fd1", nullptr } ;
#endif

// Next we have a table of device names and characteristics.
// These are ordered according to 2*densityIndex+deviceIndex,
// ie. primary (0) 1440K (0) is first, then secondary (1) 1440K is
// second, down to secondary (1) 360k (4) in position 3*2+1=7.
//
//
// Note that the data originally contained in KFloppy was
// patently false, so most of this is fake. I guess no one ever
// formatted a 5.25" floppy.
//
// The flags field is unused in this implementation.
//
//
const fdinfo fdtable[] =
{
#ifdef ANY_LINUX
        // device  drv blks trk flg
	{ fd0H1440, 0, 1440, 80, 0 },
	{ fd1H1440, 1, 1440, 80, 0 },
	{ fd0D720,  0,  720, 80, 0 },
	{ fd1D720,  1,  720, 80, 0 },
	{ fd0h1200, 0, 1200, 80, 0 },
	{ fd1h1200, 1, 1200, 80, 0 },
	{ fd0h360,  0,  360, 40, 0 },
	{ fd1h360,  1,  360, 40, 0 },
	{ fd0auto,  0,    0, 80, 0 },
	{ fd1auto,  1,    0, 80, 0 },
#endif

#ifdef ANY_BSD
	// Instead of the number of tracks, which is
	// unneeded, we record the
	// number of F's printed during an fdformat
	{ fd0, 0, 1440, 40, 0 },
	{ fd1, 1, 1440, 40, 0 },
	{ fd0, 0,  720, 40, 0 },
	{ fd1, 1,  720, 40, 0 },
	{ fd0, 0, 1200, 40, 0},
	{ fd1, 1, 1200, 40, 0},
	{ fd0, 0,  360, 40, 0},
	{ fd1, 1,  360, 40, 0},
#endif
    { nullptr, 0, 0, 0, 0 }
} ;


FloppyAction::FloppyAction(QObject *p) :
	KFAction(p),
    deviceInfo(nullptr),
    theProcess(nullptr)
{
	DEBUGSETUP;
}

void FloppyAction::quit()
{
	DEBUGSETUP;
        delete theProcess;
        theProcess=nullptr;

	KFAction::quit();
}

bool FloppyAction::configureDevice( const QString& newDeviceName )
{
    deviceInfo = nullptr; // We have not any idea what the device is
    deviceName = newDeviceName;
    return true; // No problem!
}

bool FloppyAction::configureDevice(int drive,int density)
{
	DEBUGSETUP;
    const char *devicename = nullptr;

    deviceInfo=nullptr;
	deviceName.clear();

	if ((drive<0) || (drive>1))
	{
		emit status(i18n("Unexpected drive number %1.", drive),-1);
		return false;
	}

	const fdinfo *deviceinfo = fdtable;
	for ( ; deviceinfo && (deviceinfo->devices) ; deviceinfo++)
	{
		if (deviceinfo->blocks != density)
			continue;
        }
	if (!deviceinfo)
	{
		emit status(i18n("Unexpected density number %1.", density),-1);
		return false;
	}

	deviceinfo = fdtable;
	for ( ; deviceinfo && (deviceinfo->devices) ; deviceinfo++)
	{
		if (deviceinfo->blocks != density)
			continue;
		if (deviceinfo->drive == drive)
			break;
	}

	if (!deviceinfo || !deviceinfo->devices)
	{
		emit status(i18n("Cannot find a device for drive %1 and density %2.",
			 drive, density),-1);
		return false;
	}

	for (const char* const* devices=deviceinfo->devices ;
		*devices ; devices++)
	{
		if (QT_ACCESS(*devices,W_OK)>=0)
		{
			qCDebug(KFLOPPY_LOG) << "Found device " << *devices ;
			devicename=*devices;
			break;
		}
	}

	if (!devicename)
	{
		const QString str = i18n(
			"Cannot access %1\nMake sure that the device exists and that "
			"you have write permission to it.",QLatin1String( deviceinfo->devices[0] ));
		emit status(str,-1);
		return false;
	}

	deviceName = QLatin1String( devicename );
	deviceInfo = deviceinfo;

	return true;
}

void FloppyAction::readStdOut()
{
    processStdOut( QString::fromUtf8(theProcess->readAllStandardOutput()) );
}

void FloppyAction::readStdErr()
{
    processStdOut( QString::fromUtf8(theProcess->readAllStandardError()) );
}

void FloppyAction::processDone(int exitCode, QProcess::ExitStatus exitStatus)
{
	DEBUGSETUP;

	if (exitStatus == QProcess::NormalExit)
	{
	        if (exitCode == 0)
	        {
			emit status(QString::null,100);	//krazy:exclude=nullstrassign for old broken gcc
			emit done(this,true);
		}
		else
		{
			emit status(i18n("The program %1 terminated with an error.", theProcessName),100);
			emit done(this,false);
		}
	}
	else
	{
		emit status(i18n("The program %1 terminated abnormally.", theProcessName),100);
		emit done(this,false);
	}
}

void FloppyAction::processStdOut(const QString &s)
{
	qCDebug(KFLOPPY_LOG) << "stdout:" << s;
}

void FloppyAction::processStdErr(const QString &s)
{
	processStdOut(s);
}

bool FloppyAction::startProcess()
{
	DEBUGSETUP;

	connect(theProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
		this, SLOT(processDone(int,QProcess::ExitStatus)));
	connect(theProcess, &QProcess::readyReadStandardOutput,
		this, &FloppyAction::readStdOut);
	connect(theProcess, &QProcess::readyReadStandardError,
		this, &FloppyAction::readStdErr);

	theProcess->setEnv( QStringLiteral( "LC_ALL" ), QStringLiteral( "C" ) ); // We need the untranslated output of the tool
	theProcess->setOutputChannelMode(KProcess::SeparateChannels);
	theProcess->start();
	return (theProcess->exitStatus() == QProcess::NormalExit);
}


/* static */ QString FDFormat::fdformatName = QString();

FDFormat::FDFormat(QObject *p) :
	FloppyAction(p),
	doVerify(true)
{
	DEBUGSETUP;
	theProcessName = QStringLiteral("fdformat");
	setObjectName( QStringLiteral("FDFormat" ));
}

/* static */ bool FDFormat::runtimeCheck()
{
	fdformatName = findExecutable(QStringLiteral( "fdformat" ));
	return (!fdformatName.isEmpty());
}

bool FDFormat::configure(bool v)
{
	doVerify=v;
	return true;
}

/* virtual */ void FDFormat::exec()
{
	DEBUGSETUP;

	if ( !deviceInfo || deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (fdformatName.isEmpty())
	{
		emit status(i18n("Cannot find fdformat."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	theProcess = new KProcess;

	formatTrackCount=0;

	*theProcess << fdformatName ;

	// Common to Linux and BSD, others may differ
	if (!doVerify)
	{
		*theProcess << QStringLiteral( "-n" );
	}

#ifdef ANY_BSD
	*theProcess
		<< QStringLiteral( "-y" )
		<< QStringLiteral( "-f" )
		<< QString::number(deviceInfo->blocks) ;
#elif defined(ANY_LINUX)
	// No Linux-specific flags
#endif

	// Common to Linux and BSD, others may differ
	*theProcess << deviceName;

	if (!startProcess())
	{
		emit status(i18n("Could not start fdformat."),-1);
		emit done(this,false);
	}

	// Now depend on fdformat running and producing output.
}

// Parse some output from the fdformat process. Lots of
// #ifdefs here to account for variations in the basic
// fdformat. Uses gotos to branch to whatever error message we
// need, since the messages can be standardized across OSsen.
//
//
void FDFormat::processStdOut(const QString &s)
{
	DEBUGSETUP;

#ifdef ANY_BSD
    	if (s[0]==QLatin1Char('F'))
	{
		formatTrackCount++;
		emit status(QString::null,	//krazy:exclude=nullstrassign for old broken gcc
			formatTrackCount * 100 / deviceInfo->tracks);
	}
	else if (s[0]==QLatin1Char('E'))
	{
		emit status(i18n("Error formatting track %1.", formatTrackCount),-1);
	}
	else
	{
		if (s.contains(QLatin1String( "ioctl(FD_FORM)" )))
		{
                    emit status (i18n(
                            "Cannot access floppy or floppy drive.\n"
                            "Please insert a floppy and make sure that you "
                            "have selected a valid floppy drive."),-1);
                    return;
		}
		if (s.indexOf(QLatin1String( "/dev/" ))>=0)
		{
			emit status(s,-1);
			return;
		}
		DEBUGS(s);
	}
#elif defined(ANY_LINUX)
	DEBUGS(s);
        QRegExp regexp( QStringLiteral( "([0-9]+)" ) );
        if ( s.startsWith( QLatin1String( "bad data at cyl" ) ) || s.contains( QLatin1String( "Problem reading cylinder" ) ) )
        {
            if ( regexp.indexIn( s ) > -1 )
            {
                const int track = regexp.cap(1).toInt();
                emit status(i18n("Low-level formatting error at track %1.", track), -1);
            }
            else
            {
                // This error should not happen
                emit status(i18n("Low-level formatting error: %1", s), -1);
            }
            return;
        }
	else if (s.contains(QLatin1String( "ioctl(FDFMTBEG)" )))
	{
            emit status (i18n(
                    "Cannot access floppy or floppy drive.\n"
                    "Please insert a floppy and make sure that you "
                    "have selected a valid floppy drive."),-1);
            return;
	}
        else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
        {
            emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
            return;
        }
        // Be careful to leave "iotcl" as last before checking numbers
        else if (s.contains(QLatin1String( "ioctl" )))
        {
            emit status(i18n("Low-level format error: %1", s),-1);
            return;
        }
        // Check for numbers at last (as /dev/fd0u1440 has numbers too)
        else if ( regexp.indexIn(s) > -1 )
        {
            // Normal track number (formatting or verifying)
            const int p = regexp.cap(1).toInt();
            if ((p>=0) && (p<deviceInfo->tracks))
            {
                    emit status(QString::null,	//krazy:exclude=nullstrassign for old broken gcc
                            p * 100 / deviceInfo->tracks);
            }
        }
#endif
	return;
}


/* static */ QString DDZeroOut::m_ddName = QString();

DDZeroOut::DDZeroOut(QObject *p) :
    FloppyAction(p)
{
    qCDebug(KFLOPPY_LOG) << k_funcinfo ;
    theProcessName = QStringLiteral("dd");
    setObjectName( QStringLiteral("DD" ));
}

/* static */ bool DDZeroOut::runtimeCheck()
{
    m_ddName = findExecutable(QStringLiteral( "dd" ));
    return (!m_ddName.isEmpty());
}

/* virtual */ void DDZeroOut::exec()
{
    qCDebug(KFLOPPY_LOG) << k_funcinfo ;

    if ( deviceName.isEmpty() )
    {
        emit status( i18n("Internal error: device not correctly defined."), -1 );
        emit done( this, false );
        return;
    }

    if ( m_ddName.isEmpty() )
    {
        emit status( i18n("Cannot find dd."), -1 );
        emit done( this, false );
        return;
    }

    delete theProcess;
    theProcess = new KProcess;

    *theProcess << m_ddName ;

    *theProcess << QStringLiteral( "if=/dev/zero" ) ;
    *theProcess << QStringLiteral( "of=" )+deviceName;

    if ( !startProcess() )
    {
            emit status( i18n("Could not start dd."), -1 );
            emit done( this, false );
    }

}

void DDZeroOut::processDone(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    qCDebug(KFLOPPY_LOG) << k_funcinfo ;

    /**
     * As we do not give a number of blocks to dd(1), it will stop
     * with the error "No space left on device"
     *
     * ### TODO: really check if the exit is not on an other error and then abort the formatting
     */
    emit status(QString::null,100);	//krazy:exclude=nullstrassign for old broken gcc
    emit done(this,true);
}


/* static */ QString FATFilesystem::newfs_fat = QString();

FATFilesystem::FATFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=newfs_fat;
	setObjectName( QStringLiteral("FATFilesystem" ));
}

/* static */ bool FATFilesystem::runtimeCheck()
{
	DEBUGSETUP;

#ifdef ANY_BSD
	newfs_fat = findExecutable(QStringLiteral( "newfs_msdos" ));
#elif defined(ANY_LINUX)
	newfs_fat = findExecutable(QStringLiteral( "mkdosfs" ));
#else
	return false;
#endif

	return !newfs_fat.isEmpty();
}

bool FATFilesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
		label=lbl.simplified();
	else
		label.clear();

	return true;
}

void FATFilesystem::exec()
{
	DEBUGSETUP;

	if (
#ifdef ANY_BSD // BSD needs the deviceInfo for the block count
            !deviceInfo ||
#endif
            deviceName.isEmpty())
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs_fat.isEmpty())
	{
		emit status(i18n("Cannot find a program to create FAT filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs_fat;
#ifdef ANY_BSD
	*p << QStringLiteral( "-f" ) << QString::number(deviceInfo->blocks);
	if (doLabel)
	{
		*p << QStringLiteral( "-L" ) << label ;
	}
#else
#ifdef ANY_LINUX
	if (doLabel)
	{
		*p << QStringLiteral( "-n" ) << label ;
	}
	if (doVerify)
	{
		*p << QStringLiteral( "-c" );
	}
#endif
#endif
	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start FAT format program."),-1);
		emit done(this,false);
	}
}

void FATFilesystem::processStdOut(const QString &s)
{
#ifdef ANY_BSD
    // ### TODO: do some checks
#elif defined(ANY_LINUX)
    qCDebug(KFLOPPY_LOG) << s ;
    if (s.contains(QLatin1String( "mounted" ))) // "/dev/fd0 contains a mounted filesystem"
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains( QLatin1String( "denied" ))) // "Permission denied"
    {
        emit status( s, -1 );
        return;
    }
# if 0
    else if ( s.find( "mkdosfs" ) != -1 ) // DEBUG: get the program header and show it!
    {
        emit status( s, -1 );
        return;
    }
# endif
#endif
}


#ifdef ANY_BSD

/* static */ QString UFSFilesystem::newfs = QString();

UFSFilesystem::UFSFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=newfs;
	setObjectName( QStringLiteral("UFSFilesystem" ));
}

/* static */ bool UFSFilesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = findExecutable(QStringLiteral( "newfs" ));

	return !newfs.isEmpty();
}

void UFSFilesystem::exec()
{
	DEBUGSETUP;

	if ( deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18nc("BSD", "Cannot find a program to create UFS filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;

        // ### TODO: is it still needed? (FreeBSD 5.3's man page says: "For backward compatibility.")
        if ( deviceInfo )
           *p << QStringLiteral("-T") << QStringLiteral("fd%1").arg(deviceInfo->blocks);

        *p << deviceName;

	if (!startProcess())
	{
		emit status(i18nc("BSD", "Cannot start UFS format program."),-1);
		emit done(this,false);
	}
}
#endif


/* static */ QString Ext2Filesystem::newfs = QString();

Ext2Filesystem::Ext2Filesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=QStringLiteral( "mke2fs" );
	setObjectName( QStringLiteral("Ext2Filesystem" ));
}

/* static */ bool Ext2Filesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = findExecutable(QStringLiteral( "mke2fs" ));

	return !newfs.isEmpty();
}

bool Ext2Filesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
	{
		label=lbl.trimmed();
	}
	else
	{
		label.clear();
	}

	return true;
}

void Ext2Filesystem::exec()
{
	DEBUGSETUP;

	if (
#ifdef ANY_BSD // BSD needs the deviceInfo for the block count
            !deviceInfo ||
#endif
            deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18n("Cannot find a program to create ext2 filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;
    *p << QStringLiteral("-q");
	if (doVerify) *p << QStringLiteral( "-c" ) ;
	if (doLabel) *p << QStringLiteral( "-L" ) << label ;

	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start ext2 format program."),-1);
		emit done(this,false);
	}
}

void Ext2Filesystem::processStdOut(const QString &s)
{
#ifdef ANY_BSD
    // ### TODO: do some checks
#elif defined(ANY_LINUX)
    qCDebug(KFLOPPY_LOG) << s ;
    if (s.contains(QLatin1String( "mounted" ))) // "/dev/fd0 is mounted; will not make a filesystem here!"
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains( QLatin1String( "denied" ))) // "Permission denied"
    {
        emit status( s, -1 );
        return;
    }
#endif
}



#ifdef ANY_LINUX
/* static */ QString MinixFilesystem::newfs = QString();

MinixFilesystem::MinixFilesystem(QObject *parent) :
	FloppyAction(parent)
{
	DEBUGSETUP;
	runtimeCheck();
	theProcessName=QStringLiteral( "mkfs.minix" );
	setObjectName( QStringLiteral("Minix2Filesystem" ));
}

/* static */ bool MinixFilesystem::runtimeCheck()
{
	DEBUGSETUP;

	newfs = findExecutable(QStringLiteral( "mkfs.minix" ));

	return !newfs.isEmpty();
}

bool MinixFilesystem::configure(bool v,bool l,const QString &lbl)
{
	doVerify=v;
	doLabel=l;
	if (l)
	{
		label=lbl.trimmed();
	}
	else
	{
		label.clear();
	}

	return true;
}

void MinixFilesystem::exec()
{
	DEBUGSETUP;

	if ( deviceName.isEmpty() )
	{
                emit status( i18n("Internal error: device not correctly defined."), -1 );
		emit done(this,false);
		return;
	}

	if (newfs.isEmpty())
	{
		emit status(i18n("Cannot find a program to create Minix filesystems."),-1);
		emit done(this,false);
		return;
	}

	delete theProcess;
	KProcess *p = theProcess = new KProcess;

	*p << newfs;

        // Labeling is not possible
	if (doVerify) *p << QStringLiteral( "-c" ) ;

	*p << deviceName ;

	if (!startProcess())
	{
		emit status(i18n("Cannot start Minix format program."),-1);
		emit done(this,false);
	}
}

void MinixFilesystem::processStdOut(const QString &s)
{
    qCDebug(KFLOPPY_LOG) << s ;
    if (s.contains(QLatin1String( "mounted" ))) // "mkfs.minix: /dev/fd0 is mounted; will not make a filesystem here!"
    {
        emit status(i18n("Floppy is mounted.\nYou need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains(QLatin1String( "busy" ))) // "Device or resource busy"
    {
        emit status(i18n("Device busy.\nPerhaps you need to unmount the floppy first."),-1);
        return;
    }
    else if (s.contains( QLatin1String( "denied" ))) // "Permission denied"
    {
        emit status( s, -1 );
        return;
    }
}

#endif
