#ifndef _FORMAT_H
#define _FORMAT_H
/* $Id: $
**
** Copyright (C) 2002 by Adriaan de Groot
**
** Defines base classes for all DiskFormats.
** Also defines some concrete implementations:
**   DosFloppyFormat, which does DOS and ext2.
**   UFSFloppyFormat, which does UFS.
**
*/

/*
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/


#include <qwidget.h>
#include <qptrlist.h>

class QGridLayout;
class QComboBox;
class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QTimer;

class KProcess;
class KConfig;

class FilesystemData;
class DiskFormat;

// Data class for keeping track of the diskformats in use.
// See format.cpp for details. We're going to be calling this
// a "FileSystem" even though it's really a product of the
// physical media and the filesystem that will be written to it.
//
//
// formatName is the i18n()ed name (ie. "DOS Diskette" or "40Mb MO XFS"),
// number should be unique across the filesystems supported by each DiskFormat
//   and serves to distinguish the filesystems internally. This is typically
//   a hex number generated as follows:
//      1) think of a short descriptive identifier (ie. ext2fs)
//	2) run it through
//		tr 'abcdefghijklmnopqrstuvwxyz' 'abcdef0123456789abcdef0123'
//	3) if you get a neat-o hex number, use it.
//   This pretty much ensures unicity of the format numbers across
//   all formats, which is good. But 1,2,3 are also fine if you like.
// format should always point to the DiskFormat object that
//   created the FilesystemData object.
//
// Note that passing "this" invokes some weird type rules in C++.
// You'll need to explicitly cast (DiskLabel *)this in constructor
// calls to FilesystemData within member functions of (subclasses of)
// DiskFormat, yielding:
//
// new FilesystemData(i18n(ufslabel),0xefcf5899,(DiskFormat *)this)
//
// for a UFS Floppy disk (note the id is "ufsflopp", run through the
// tr invocation above).
//
// Filesystems can have arbitrary flags for internal use; only the
// first bit of the flags field is defined to mean that the filesystem
// is in common use on the current OS (ie. ext2 is basic
// on Linux while it isn't under BSD). This is to support showing only
// the basic filesystems unless the user requests otherwise.
// You should aim to have only one or two filesystem types
// basic on a given platform.
//
//
class FilesystemData
{
public:
	FilesystemData(QString n,int i,DiskFormat *f);

	bool isBasic() const { return (fflags & 0x1); }
	int flags() const { return fflags; }
	void setFlags(int f) { fflags=f; }

	QString name() const { return formatName; }
	unsigned int magic() const { return formatNumber; }
	DiskFormat *format() const { return fformat; }

private:
	QString formatName;
	unsigned int formatNumber;
	DiskFormat *fformat;
	int fflags;
} ;



typedef QPtrList<FilesystemData> FilesystemList;


/*
** Abstract base class for all disk formats. This
** defines the signals all subclasses can send
** and the essential API.
**
** A DiskFormat can handle formatting one or more
** different devices with one or more different
** filesystems. It displays the settings for all
** those devices and filesystems (so basically
** a DiskFormat is a bunch of settings and it does
** the work for any combination of device and FS
** for which these settings make sense).
*/
class DiskFormat : public QWidget
{
Q_OBJECT

public:
	DiskFormat(QWidget *parent,const char *name);

	// This needs to return an i18n()ed string
	// stating the names of the formats this
	// DiskFormat can handle, for example
	// "ext2fs floppy" or "UFS (zip100)".
	//
	virtual FilesystemList FSLabels() const = 0;

	// Saving and restoring settings. Every DiskFormat
	// should store its settings in a separate [group]
	//  named after the format. Change the group first.
	//
	//
	virtual void readSettings(KConfig *);
	virtual void writeSettings(KConfig *);

	// Every concrete implementation of a DiskFormat
	// should also have a static bool runtimeCheck()
	// that checks for whatever executables are needed
	// and returns false if there's no point in including
	// the format in the KFloppy program.
	//
	//
	// static bool runtimeCheck();

public slots:
	// Format is called once to start a format.
	// Every DiskFormat is responsible for emitting
	// formatDone(int) when finished. 
	//
	// d tells the DiskFormat which of its formats has been
	// chosen -- it's a member of the  the list
	// returned by FSLabels().
	//
	//
	virtual void format(FilesystemData *d) = 0;

	// During a format, the UI is disabled. Afterwards,
	// it is reenabled. Each DiskFormat should reimplement
	// this to actually disable all its UI elements.
	//
	//
	virtual void setEnabled(bool) = 0;

	// Called at the end of a format (when it should
	// really be a no-op) and if a format is aborted because
	// the user clicks on the abort button. Should stop any
	// timers and processes associated with the format.
	//
	//
	virtual void quit();

signals:
	// A DiskFormat needs to emit this when the formatting has finished.
	// Use an errno of -1 to indicate an error, and 0 for a succesful
	// completion. More detailed error reporting may be added in future.
	//
	// Note that a DiskFormat _must_ emit this, otherwise the UI
	// will remain disabled.
	//
	//
	void formatDone(int errno);

	// Print a message in the status panel. This can indicate the
	// phase the format is in, for multi-phase formats.
	//
	//
	void statusMessage(const QString &);

	// Sets the progress bar to some percentage. The parameter
	// should be between 0 and 100.
	//
	//
	void setProgress(int);

protected:
	// Polish and resize. Remember to call this after
	// creating all your widgets so that the UI is sized properly.
	//
	//
	void endInit();

	// Returns the full pathname of an executable in
	// the current search path (and in /usr/sbin:/sbin as well).
	//
	//
	static QString findExecutable(const QString &);

	// Issues a generic complaing that the format
	// is being asked to do a filesystem number
	// it doesn't know about. (ie. the int passed
	// to format is out of range for FSLabels().at()).
	//
	void complainAboutFormat(FilesystemData *);
} ;

/*
** Abstract base class of all floppy (ie. 3.5" diskette)
** based formats. Adds a bunch of utility functions
*/
class FloppyFormat : public DiskFormat
{
Q_OBJECT
public:
	FloppyFormat(QWidget *parent,const char *name);

	virtual void setEnabled(bool);

	static bool runtimeCheck();

	// Set the group correctly before calling these
	// from subclasses, since these just write to the
	// current group, not a separate FloppyFormat section.
	//
	//
	virtual void writeSettings(KConfig *);
	virtual void readSettings(KConfig *);

protected:
	QGridLayout *grid;

	QComboBox* deviceComboBox;
	QComboBox* densityComboBox;

	// Insert initial widgets into the layout
	void init();

	// Check the device entered by the user and setup
	// variables pointing to the device node.
	bool findDevice();
	QString device;
	int blocks;
	int tracks;

	int findKeyWord(QString &, const QString &);

	// Since all the floppy-based formats will want to
	// run fdformat or something similar, include code
	// to do so in the base class.
	//
	// Usage: call fdformat() and wait for the signal
	// fdformatCompleted(int) to be emitted. This carries
	// the exit code of fdformat.
protected:
	void fdformat();
	KProcess *formatProcess;
	static QString fdformatName;
	int formatTrackCount;
protected slots:
	void fdformatDone(KProcess *);
	void fdformatOutput(KProcess *, char *,int);
signals:
	void fdformatCompleted(int);
} ;

/*
** Concrete implementation of a DOS (FAT) filesystem floppy.
*/
class DosFloppyFormat : public FloppyFormat
{
Q_OBJECT
public:
	DosFloppyFormat(QWidget *parent, const char *name);

	virtual FilesystemList FSLabels() const;
	virtual void setEnabled(bool);

	static bool runtimeCheck();

	virtual void writeSettings(KConfig *);
	virtual void readSettings(KConfig *);

public slots:
	/* slot */ virtual void format(FilesystemData *);

protected:
	QButtonGroup* buttongroup;
	QCheckBox*    labellabel;
	QLineEdit*    lineedit;
	QRadioButton* quick;
	QRadioButton* fullformat;

        static QString msdosfs,ext2fs,ext3fs;

	enum supportedFormats {
		MSDOS_FS=0x6cd8cfc,
		EXT2_FS=0xe1d2fc,
		EXT3_FS=0x70e1d3fc
		} ;
	unsigned int formatNumber; // Remeber whether we're doing msdos, ext2 or ext3

protected slots:
	void createFileSystem();
	void createFileSystem(int);
	void createFSDone(KProcess *);
	void createFSOutput(KProcess *, char *, int);
} ;

class UFSFloppyFormat : public FloppyFormat
{
Q_OBJECT
public:
	UFSFloppyFormat(QWidget *parent, const char *name);

	virtual FilesystemList FSLabels() const;
	virtual void setEnabled(bool);

	static bool runtimeCheck();

	virtual void writeSettings(KConfig *);
	virtual void readSettings(KConfig *);

public slots:
	/* slot */ virtual void format(FilesystemData *);

protected:
	QCheckBox *quick;

        static QString createfs;

protected slots:
	void createFileSystem();
	void createFileSystem(int);
	void createFSDone(KProcess *);
	void createFSOutput(KProcess *, char *, int);
} ;
#endif




