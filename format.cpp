/*
** $Id$
**
** Copyright (C) 2002 by Adriaan de Groot
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "format.moc"

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include "debug.h"


FilesystemData::FilesystemData(QString s,int i, DiskFormat *f)
{
	formatName=s; formatNumber=i; fformat=f;
	fflags=0;
} ;



DiskFormat::DiskFormat(QWidget *p,const char *n) :
	QWidget(p,n)
{
	DEBUGSETUP;
}

/* virtual slot */ void DiskFormat::quit()
{
}

void DiskFormat::endInit()
{
	DEBUGSETUP;

	this->adjustSize();
	setMinimumSize(size());

	DEBUGS("  Final widget size:");
	DEBUGSZ(size());
}


/* static */ QString DiskFormat::findExecutable(const QString &e)
{
	QString path = getenv("PATH");
	path.append(":/usr/sbin:/sbin");

	return KGlobal::dirs()->findExe(e, path);
}

/* virtual */ void DiskFormat::readSettings(KConfig *)
{
	DEBUGSETUP;
}

/* virutal */ void DiskFormat::writeSettings(KConfig *)
{
	DEBUGSETUP;
}

void DiskFormat::complainAboutFormat(FilesystemData *i)
{
	emit statusMessage(i18n("Incompatible format \"%1\".")
		.arg(i->name()));
	emit formatDone(-1);
}




FloppyFormat::FloppyFormat(QWidget *p,const char *n) :
	DiskFormat(p,n),
	grid(0L),
	deviceComboBox(0L),
	densityComboBox(0L),
	formatProcess(0L)
{
	DEBUGSETUP;
}

void FloppyFormat::init()
{
	DEBUGSETUP;

	grid = new QGridLayout(this,5,2,0,7);


        QLabel *label1 = new QLabel(this);
	label1->setText(i18n("Floppy drive:"));
        grid->addWidget( label1, 0, 0, AlignLeft );


	deviceComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	QWhatsThis::add(deviceComboBox,
		i18n("Select which floppy drive you want to use. "
			"Primary would be called A: under DOS and "
			"in your BIOS, while Secondary is B:."));

	grid->addWidget( deviceComboBox, 0, 1 );

	deviceComboBox->insertItem(i18n("Primary"));
	deviceComboBox->insertItem(i18n("Secondary"));

        label1 = new QLabel(this);
	label1->setText(i18n("Size:"));
        grid->addWidget( label1, 1, 0, AlignLeft );

	densityComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	QWhatsThis::add(densityComboBox,
		i18n("Select the density that matches your "
			"choice of drive and floppy."));
	grid->addWidget( densityComboBox, 1, 1 );

	// The order here has to match the order in fdtable below.
	//
	densityComboBox->insertItem(i18n("3.5\" 1.44MB"));
	densityComboBox->insertItem(i18n("3.5\" 720KB"));
	densityComboBox->insertItem(i18n("5.25\" 1.2MB"));
	densityComboBox->insertItem(i18n("5.25\" 360KB"));
}

void FloppyFormat::setEnabled(bool b)
{
	deviceComboBox->setEnabled(b);
	densityComboBox->setEnabled(b);
}

// Here we have names of devices. The variable
// names are basically the linux device names,
// replace with whatever your OS needs instead.
//
//
#ifdef ANY_LINUX
const char *fd0H1440[] = { "/dev/fd0H1440", "/dev/fd0u1440", 0L } ;
const char *fd0D720[]={ "/dev/fd0D720", "/dev/fd0u720", 0L };
const char *fd0h1200[]={ "/dev/fd0h1200", 0L };
const char *fd0h360[]={ "/dev/fd0h360", 0L };
const char *fd1H1440[] = { "/dev/fd1H1440", "/dev/fd0u1440", 0L } ;
const char *fd1D720[]={ "/dev/fd1D720", "/dev/fd0u720", 0L };
const char *fd1h1200[]={ "/dev/fd1h1200", 0L };
const char *fd1h360[]={ "/dev/fd1h360", 0L };
#endif

#ifdef ANY_BSD
const char *fd0[] = { "/dev/fd0", 0L } ;
const char *fd1[] = { "/dev/fd1", 0L } ;
#endif

// Next we have a table of device names and characteristics.
// These are ordered according to 2*densityIndex+deviceIndex,
// ie. primary (0) 1440K (0) is first, then secondary (1) 1440K is
// second, down to secondary (1) 360k (4) in position 3*2+1=7.
//
//
// Note that the data originally contained in KFloppy was
// patently false, so most of this is fake. I guess noone ever
// formatted a 5.25" floppy.
//
// The flags field is unused in this implementation.
//
//
typedef struct { const char **devices;
	int blocks;
	int tracks;
	int flags; } fdinfo;

fdinfo fdtable[] =
{
#ifdef ANY_LINUX
	{ fd0H1440, 1440, 80, 0 },
	{ fd1H1440, 1440, 80, 0 },
	{ fd0D720, 720, 80, 0 },
	{ fd1D720, 720, 80, 0 },
	{ fd0h1200, 720, 80, 0 },
	{ fd1h1200, 720, 80, 0 },
	{ fd0h360, 720, 80, 0 },
	{ fd1h360, 720, 80, 0 },
#endif

#ifdef ANY_BSD
	{ fd0, 1440, 40, 0 },	// Number of F's printed during an fdformat
	{ fd1, 1440, 40, 0 },
	{ fd0, 720, 40, 0 },
	{ fd1, 720, 40, 0 },
	{ fd0, 1200, 40, 0},
	{ fd1, 1200, 40, 0},
	{ fd0, 360, 40, 0},
	{ fd1, 360, 40, 0},
#endif
	{ 0L, 0, 0, 0 }
} ;

bool FloppyFormat::findDevice()
{
	DEBUGSETUP;

	int index = deviceComboBox->currentItem() +
		2*densityComboBox->currentItem();

	// Check the table for consistency first
	for (int i=0; i<index; i++)
	{
		if (fdtable[i].devices == 0L) return false;
	}

	blocks=fdtable[index].blocks;
	tracks=fdtable[index].tracks;

	for (const char **devices=fdtable[index].devices ;
		*devices ; devices++)
	{
		if (access(*devices,W_OK)>=0)
		{
			device=*devices;
			DEBUGS(QString("  Found device %1").arg(device).latin1());
			return true;
		}
	}

    QString str = i18n(
	      "Cannot access %1\nMake sure that the device exists and that "
	      "you have write permission to it.").arg(fdtable[index].devices[0]);
    KMessageBox::error(this, str);

    return false;

}

int FloppyFormat::findKeyWord(QString & string,const QString & word){

  int count = 0;
  int index = 0;
  int len = 0;

  QString wordstring = word;
  len = wordstring.length();

  while( (index = string.find(word)) >= 0)
  {
    count++;
    string = string.mid(index + len,string.length());
  }

  return count;
}

void FloppyFormat::fdformat()
{
	DEBUGSETUP;

	if (!findDevice())
	{
		emit statusMessage(i18n("Don't know which device to use."));
		emit fdformatCompleted(-1);
	}

	if (formatProcess) delete formatProcess;
	formatProcess = new KProcess;

	connect(formatProcess,SIGNAL(processExited(KProcess *)),
		this,SLOT(fdformatDone(KProcess *)));
	connect(formatProcess,SIGNAL(receivedStdout(KProcess *,char *,int)),
		this,SLOT(fdformatOutput(KProcess *,char *,int)));
	connect(formatProcess,SIGNAL(receivedStderr(KProcess *,char *,int)),
		this,SLOT(fdformatOutput(KProcess *,char *,int)));

	formatTrackCount=0;

#ifdef ANY_BSD
	DEBUGS(QString("  Running %1 -y -f %2 %3").arg(fdformatName).arg(blocks).arg(device).latin1() );
	*formatProcess << fdformatName
		<< "-y"
		<< "-f"
		<< QString::number(blocks)
		<< device;
#else
#ifdef ANY_LINUX
	*formatProcess << fdformatName
		<<  device ;
#endif
#endif

	if (!formatProcess->start(KProcess::NotifyOnExit,
		KProcess::AllOutput))
	{
		emit statusMessage(i18n("Couldn't start fdformat."));
		emit fdformatCompleted(-1);
	}
}

void FloppyFormat::fdformatDone(KProcess *)
{
	DEBUGSETUP;
	emit fdformatCompleted(formatProcess->exitStatus());
	delete formatProcess;
	formatProcess=0L;
}

// Parse some output from the fdformat process. Lots of
// #ifdefs here to account for variations in the basic
// fdformat. Uses gotos to branch to whatever error message we
// need, since the messages can be standardized across OSsen.
//
//
void FloppyFormat::fdformatOutput(KProcess *, char *b, int l)
{
	DEBUGSETUP;
	QString s;

#ifdef ANY_BSD
	if (b[0]=='F')
	{
		formatTrackCount++;
		emit setProgress(formatTrackCount * 100 / tracks);
	}
	else if (b[0]=='E')
	{
		emit statusMessage(i18n("Error formatting track %1.").arg(formatTrackCount));
	}
	else
	{
		s = QString::fromLatin1(b,l);
		if (s.contains("ioctl(FD_FORM)"))
		{
			goto ioError;
		}
		DEBUGS(s.latin1());
	}
#else
#ifdef ANY_LINUX
	s = QString::fromLatin1(b,l);
	if (s.contains("ioctl(FDFMTBEG)"))
	{
		goto ioError;
	}
	DEBUGS(s.latin1());
#endif
#endif
	return;

// Now a bunch of (goto) labels for all the possible error messages.
//
//
ioError:
	KMessageBox::sorry(this,i18n(
		"Cannot access floppy or floppy drive.\n"
		"Please insert a floppy and make sure that you "
		"have selected a valid floppy drive."));
	return;
}

/* static */ bool FloppyFormat::runtimeCheck()
{
	DEBUGSETUP;
	fdformatName = findExecutable("fdformat");
	DEBUGS(QString("  Found fdformat is %1.").arg(fdformatName).latin1());
	return !fdformatName.isEmpty();
}

/* static */ QString FloppyFormat::fdformatName;



void FloppyFormat::readSettings(KConfig *c)
{
	deviceComboBox->setCurrentItem(
		c->readNumEntry("Device",0));
	densityComboBox->setCurrentItem(
		c->readNumEntry("Density",0));
}

void FloppyFormat::writeSettings(KConfig *c)
{
	c->writeEntry("Device",deviceComboBox->currentItem());
	c->writeEntry("DeviceName",deviceComboBox->currentText()); // Unused
	c->writeEntry("Density",densityComboBox->currentItem());
	c->writeEntry("DensityName",densityComboBox->currentText()); // Unused
}






DosFloppyFormat::DosFloppyFormat(QWidget *w,const char *n) :
	FloppyFormat(w,n)
{
	DEBUGSETUP;

	init();

	DEBUGS("  Creating own widgets.");

	buttongroup = new QButtonGroup( this, "ButtonGroup_1" );
	buttongroup->setFrameStyle( 49 );
	grid->addMultiCellWidget(buttongroup,2,2,0,1);

        QVBoxLayout* v2 = new QVBoxLayout( buttongroup, 10 );

	quick = new QRadioButton( buttongroup, "RadioButton_2" );
	quick->setText(i18n( "Q&uick format") );
	QWhatsThis::add(quick,
		i18n("Just rebuild the filesystem. "
			"Do not check for bad blocks. "
			"The floppy must already be formatted to use this option."));
        v2->addWidget( quick, AlignLeft );

	fullformat = new QRadioButton( buttongroup, "RadioButton_3" );
	fullformat->setText(i18n( "Fu&ll format") );
	fullformat->setChecked(TRUE);
	QWhatsThis::add(fullformat,
		i18n("Low-level format and check for bad blocks, "
			"followed by filesystem creation."));
        v2->addWidget( fullformat, AlignLeft );

	labellabel = new QCheckBox( buttongroup, "RadioButton_4" );
	labellabel->setText(i18n( "Volume &label:") );
	labellabel->setChecked(TRUE);
	QWhatsThis::add(labellabel,
		i18n("If you want to label the floppy disk, check this box."));
        v2->addWidget( labellabel, AlignLeft );

        QHBoxLayout* h2 = new QHBoxLayout( v2 );
        h2->addSpacing( 20 );

	lineedit = new QLineEdit( buttongroup, "Lineedit" );
	lineedit->setText(i18n( "KDE Floppy") );
	lineedit->setMaxLength(11);
        lineedit->setMinimumWidth( lineedit->sizeHint().width() );
	QWhatsThis::add(lineedit,
		i18n("Label for the newly formatted floppy."));
        h2->addWidget( lineedit, AlignRight );

	connect(labellabel,SIGNAL(toggled(bool)),lineedit,SLOT(setEnabled(bool)));

	endInit();

	grid->addRowSpacing(3,10);
	grid->setRowStretch(3,100);
}


const char *doslabel = I18N_NOOP("dos floppy");

/* virtual */ FilesystemList DosFloppyFormat::FSLabels() const
{
	FilesystemList l;
	FilesystemData *d = 0L;
	if (!msdosfs.isEmpty())
	{
		d = new FilesystemData(i18n(doslabel),MSDOS_FS,(DiskFormat *)this);
		// Basic everywhere
		d->setFlags(1);
		l.append(d);
	}

	if (!ext2fs.isEmpty())
	{
		d=new FilesystemData(i18n("ext2 floppy"),EXT2_FS,(DiskFormat *)this);
		// Basic only on Linux systems
#ifdef ANY_LINUX
		d->setFlags(1);
#endif
		l.append(d);
	}
	if (!ext3fs.isEmpty())
	{
		// Nowhere basic
		d = new FilesystemData(i18n("ext3 floppy"),EXT3_FS,(DiskFormat *)this);
		l.append(d);
	}
	return l;
}

/* virtual slot */ void DosFloppyFormat::format(FilesystemData *f)
{
	DEBUGSETUP;

	bool ok = false;
	switch(f->magic())
	{
	case MSDOS_FS: // msdos
		ok = !msdosfs.isEmpty();
		break;
	case EXT2_FS:	// ext2fs
		ok = !ext2fs.isEmpty();
		break;
	case EXT3_FS:// ext3fs
		ok = !ext3fs.isEmpty();
		break;
	}
	if (!ok)
	{
		complainAboutFormat(f);
		return;
	}
	formatNumber = f->magic();

	if(!findDevice()){
		emit statusMessage(i18n("Can't find device."));
		emit formatDone(-1);
		return;
	}

	if(quick->isChecked())
	{
		QTimer::singleShot(0,this,SLOT(createFileSystem()));
		return;
	}

	emit statusMessage(i18n("Formatting..."));

	FloppyFormat::fdformat();
	connect(this,SIGNAL(fdformatCompleted(int)),
		this,SLOT(createFileSystem(int)));
}

void DosFloppyFormat::createFileSystem()
{
	createFileSystem(0);
}

void DosFloppyFormat::createFileSystem(int status)
{
	if (status)
	{
		emit formatDone(status);
		return;
	}

	KProcess *p = new KProcess;

	switch(formatNumber)
	{
	case MSDOS_FS :
		*p << msdosfs;
#ifdef ANY_BSD
		*p << "-f" << QString::number(blocks);
		if (labellabel->isChecked())
		{
			QString l = lineedit->text().stripWhiteSpace();
			*p << "-L" << l ;
		}
		*p << device ;
#else
#ifdef ANY_LINUX
		if (labellabel->isChecked())
		{
			*p << "-n" << lineedit->text().stripWhiteSpace();
		}
		*p << device;
#endif
#endif
		break;
	case EXT2_FS :
	case EXT3_FS :
		if (formatNumber == EXT2_FS)
			*p << ext2fs;
		else	*p << ext3fs;
#ifdef ANY_BSD
		*p << "-F"	// Force. FreeBSD doesn't use block-devices
			<< "-v" // Verbose, fwiw.
			;
		if (labellabel->isChecked())
		{
			*p << "-L" << lineedit->text().stripWhiteSpace();
		}
		*p << device << QString::number(blocks);
#else
#ifdef ANY_LINUX
		*p << "-q" ;
		if (labellabel->isChecked())
		{
			*p << "-L" << lineedit->text().stripWhiteSpace();
		}
		*p << device ;
#endif
#endif
		break;
	}

	connect(p,SIGNAL(processExited(KProcess *)),
		this,SLOT(createFSDone(KProcess *)));
	connect(p,SIGNAL(receivedStdout(KProcess *,char *,int)),
		this,SLOT(createFSOutput(KProcess *,char *,int)));
	connect(p,SIGNAL(receivedStderr(KProcess *,char *,int)),
		this,SLOT(createFSOutput(KProcess *,char *,int)));

	p->start(KProcess::NotifyOnExit,KProcess::AllOutput);

	emit statusMessage(i18n("Creating filesystem ..."));
}

void DosFloppyFormat::createFSDone(KProcess *p)
{
	if (p->exitStatus())
	{
#ifdef ANY_BSD
		emit statusMessage(i18n("newfs failed."));
#else
#ifdef ANY_LINUX
		emit statusMessage(i18n("Format failed."));
#endif
#endif
	}
	else
	{
		emit statusMessage(i18n("Format complete."));
	}

	emit formatDone(p->exitStatus());

	delete p;
}

void DosFloppyFormat::createFSOutput(KProcess *,char *b,int l)
{
	QString o = QString::fromLatin1(b,l);
	switch(formatNumber)
	{
	case EXT2_FS :
	case EXT3_FS :
		if (o.find("mke2fs")>=0) emit setProgress(20);
		else if (o.find("inode tables")>0) emit setProgress(65);
		else if (o.find("superblocks")>0) emit setProgress(95);
		else { DEBUGS(o.latin1()); }
		break;
	case MSDOS_FS :
		DEBUGS(o.latin1());
		break;
	}
}

/* virtual slot */ void DosFloppyFormat::setEnabled(bool b)
{
	FloppyFormat::setEnabled(b);

	buttongroup->setEnabled(b);
	quick->setEnabled(b);
	fullformat->setEnabled(b);
	labellabel->setEnabled(b);
	lineedit->setEnabled(b);
}

// Find the necessary executables to be able to do
// dos, ext2 or ext3 formats. Lots of #ifdefs here for
// various OSsen.
//
//
bool DosFloppyFormat::runtimeCheck()
{
	DEBUGSETUP;

	if (!FloppyFormat::runtimeCheck()) return false;

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	msdosfs = findExecutable("newfs_msdos");
#endif
#ifdef __FreeBSD__
	ext2fs = findExecutable("mkfs.ext2");
	// Support ext3 only if ext2 also found.
	if (!ext2fs.isEmpty())
	{
		ext3fs = findExecutable("mkfs.ext3");
	}
#endif

	// If any one was found, we can work.
	return (!msdosfs.isEmpty() ||
		!ext2fs.isEmpty() ||
		!ext3fs.isEmpty()) ;
}

/* static */ QString DosFloppyFormat::msdosfs;
/* static */ QString DosFloppyFormat::ext2fs;
/* static */ QString DosFloppyFormat::ext3fs;



void DosFloppyFormat::readSettings(KConfig *c)
{
	c->setGroup(doslabel);
	FloppyFormat::readSettings(c);

	if (c->readBoolEntry("QuickFormat",true))
	{
		quick->setChecked(true);
		fullformat->setChecked(false);
	}
	else
	{
		fullformat->setChecked(false);
		quick->setChecked(true);
	}

	labellabel->setChecked(c->readBoolEntry("LabelEnabled",true));
	lineedit->setText(c->readEntry("Label"));

	if (!labellabel->isChecked()) lineedit->setEnabled(false);
}

void DosFloppyFormat::writeSettings(KConfig *c)
{
	c->setGroup(doslabel);
	FloppyFormat::writeSettings(c);
	c->writeEntry("QuickFormat",quick->isChecked());
	c->writeEntry("LabelEnabled",labellabel->isChecked());
	c->writeEntry("Label",lineedit->text());
}





UFSFloppyFormat::UFSFloppyFormat(QWidget *w,const char *n) :
	FloppyFormat(w,n)
{
	DEBUGSETUP;

	init();

	quick = new QCheckBox( this );
	quick->setText(i18n( "Q&uick format") );
	QWhatsThis::add(quick,
		i18n("Just rebuild the filesystem. "
			"Do not check for bad blocks. "
			"The floppy must already be formatted to use this option."));
	grid->addWidget(quick,2,0);

	endInit();

	grid->addRowSpacing(3,10);
	grid->setRowStretch(3,100);
}


const char *ufslabel = I18N_NOOP("UFS floppy");

/* virtual */ FilesystemList UFSFloppyFormat::FSLabels() const
{
	FilesystemList l;
	FilesystemData *d;
	d = new FilesystemData(i18n(ufslabel),0xefcf5899,(DiskFormat *)this);
#ifdef ANY_BSD
	// This is a basic filesystem in BSD land.
	d->setFlags(1);
#endif
	l.append(d);
	return l;
}

/* virtual slot */ void UFSFloppyFormat::format(FilesystemData *f)
{
	DEBUGSETUP;

	if (f->magic()!=0xefcf5899)
	{
		complainAboutFormat(f);
		return;
	}

	if(!findDevice()){
		emit statusMessage(i18n("Can't find device."));
		emit formatDone(-1);
		return;
	}

	if(quick->isChecked())
	{
		QTimer::singleShot(0,this,SLOT(createFileSystem()));
		return;
	}

	emit statusMessage(i18n("Formatting..."));

	FloppyFormat::fdformat();
	connect(this,SIGNAL(fdformatCompleted(int)),
		this,SLOT(createFileSystem(int)));
}

void UFSFloppyFormat::createFileSystem()
{
	createFileSystem(0);
}

void UFSFloppyFormat::createFileSystem(int status)
{
	if (status)
	{
		emit formatDone(status);
		return;
	}

	KProcess *p = new KProcess;

	*p << createfs
		<< "-T" << QString("fd%1").arg(blocks)
		<< device;

	connect(p,SIGNAL(processExited(KProcess *)),
		this,SLOT(createFSDone(KProcess *)));
	connect(p,SIGNAL(receivedStdout(KProcess *,char *,int)),
		this,SLOT(createFSOutput(KProcess *,char *,int)));
	connect(p,SIGNAL(receivedStderr(KProcess *,char *,int)),
		this,SLOT(createFSOutput(KProcess *,char *,int)));

	p->start(KProcess::NotifyOnExit,KProcess::AllOutput);

	emit statusMessage(i18n("Creating filesystem ..."));
}

void UFSFloppyFormat::createFSDone(KProcess *p)
{
	if (p->exitStatus())
	{
		emit statusMessage(i18n("newfs failed."));
	}
	else
	{
		emit statusMessage(i18n("Format complete."));
	}

	emit formatDone(p->exitStatus());

	delete p;
}

void UFSFloppyFormat::createFSOutput(KProcess *,char *b,int l)
{
	QString o = QString::fromLatin1(b,l);
	if (o.find("/dev")>=0) emit setProgress(30);
	else if ((b[0]==' ') && (isdigit(b[1]))) emit setProgress(65);
	else
	{
		DEBUGS(o.latin1());
	}
}

/* virtual slot */ void UFSFloppyFormat::setEnabled(bool b)
{
	FloppyFormat::setEnabled(b);

	quick->setEnabled(b);
}

bool UFSFloppyFormat::runtimeCheck()
{
	DEBUGSETUP;

	createfs = findExecutable("newfs");
	return !createfs.isEmpty() && FloppyFormat::runtimeCheck();
}

/* static */ QString UFSFloppyFormat::createfs;



void UFSFloppyFormat::readSettings(KConfig *c)
{
	c->setGroup(ufslabel);
	FloppyFormat::readSettings(c);

	quick->setChecked(c->readBoolEntry("QuickFormat",true));
}

void UFSFloppyFormat::writeSettings(KConfig *c)
{
	c->setGroup(ufslabel);
	FloppyFormat::writeSettings(c);
	c->writeEntry("QuickFormat",quick->isChecked());
}



