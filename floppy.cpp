
/*

    $Id$

    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu
    Copyright (C) 2002 Adriaan de Groot

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "floppy.moc"
#include "format.h"
#include "zip.h"

#include <iostream.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kprogress.h>

#include <kmenubar.h>
#include <kpopupmenu.h>
#include <kaction.h>

#include "debug.h"

FloppyData::FloppyData(QWidget * parent, const char * name)
 : KMainWindow( parent, name )
{
	DEBUGSETUP;

	formating = false;


	QWidget *w = new QWidget(this);
	setCentralWidget(w);

	QGridLayout *grid = new QGridLayout(w,5,3,10);

        QLabel *label3 = new QLabel(w);
	label3->setText(i18n("File system:"));
        grid->addWidget( label3, 0, 0, AlignLeft );

	filesystemComboBox = new QComboBox( FALSE, w, "ComboBox_2" );
	QWhatsThis::add(filesystemComboBox,
		i18n("<qt>Select a filesystem format you want. "
			"This list shows the supported combinations "
			"of devices (like floppy or <i>Zip</i> disks) and "
			"filesystems (like FAT or ext2).</qt>"));
	grid->addWidget( filesystemComboBox, 0, 1, AlignLeft );

	formatbutton = new KPushButton( w, "PushButton_3" );
	formatbutton->setText(i18n( "&Format") );
	formatbutton->setAutoRepeat( FALSE );
	QWhatsThis::add(formatbutton,
		i18n("Click here to start the formatting process with "
			"the options you have selected."));

	connect(formatbutton,SIGNAL(clicked()),this,SLOT(format()));
        grid->addWidget( formatbutton,0,2 );


	stack = new QWidgetStack(w,"stack");
	grid->addMultiCellWidget(stack,1,1,0,2);

	frame = new QLabel( w, "NewsWindow" );
	frame->setMinimumHeight( 50 );
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	frame->setAlignment(AlignCenter|WordBreak|ExpandTabs);
	QWhatsThis::add(frame,
		i18n("This box displays diagnostic messages concerning "
			"the formatting process."));
        grid->addMultiCellWidget( frame , 2,2,0,2 );

	progress = new KProgress( w, "Progress" );
        progress->setMinimumHeight( 30 );
	QWhatsThis::add(progress,
		i18n("Displays a rough idea of the progress of the formatting "
			"process. Not all formats allow for an accurate display."));
        grid->addMultiCellWidget( progress,3,3,0,2);

	DiskFormat *df = 0L;

#define	addPage(t) if (t::runtimeCheck()) { df = new t(this,#t); addFormat(df); }

#if defined(ANY_BSD) || defined(ANY_LINUX)
	addPage(DosFloppyFormat);	// Does dos, ext2, ext3
#endif
#ifdef ANY_BSD
	addPage(UFSFloppyFormat);
	addPage(ZipFormat);
#endif

#undef addPage


	connect(filesystemComboBox,SIGNAL(activated(int)),
		this,SLOT(formatSelected(int)));

	initMenu();
	readSettings();
	showFormats();
	if (!df) // ie. never got assigned to anything
	{
		frame->setText(i18n("KFloppy has no suitable floppy formats "
			"for use on your system."));
		filesystemComboBox->setEnabled(false);
		formatbutton->setEnabled(false);
		formatAction->setEnabled(false);

	}
	if (hideProgress->isChecked() && !frame->text().isEmpty())
	{
		KMessageBox::sorry(0L,
			frame->text());
	}

	// Hide various UI elements if necessary.
	if (hideProgress->isChecked())
	{
		frame->hide();
		progress->hide();
	}
	if (hideConfig->isChecked()) stack->hide();

	grid->invalidate();
	w->adjustSize();

	// In this case, we need to shorten the window a bit,
	// probably some QGridLayout weirdness.
	//
	//
	if (hideProgress->isChecked() && hideConfig->isChecked())
	{
		QTimer::singleShot(0,this,SLOT(toggleProgress()));
	}

	readSettings2();

	frame->setText(i18n("Welcome to KFloppy."));
	w->show();
}

void FloppyData::addFormat(DiskFormat *df)
{
	DEBUGSETUP;

	stack->addWidget(df);
	availableFormats.append(df);
	FilesystemList l = df->FSLabels();
	for (unsigned int i=0; i<l.count(); i++)
	{
		DEBUGS(QString("  %1").arg(l.at(i)->name()).latin1());
		availableFilesystems.append(l.at(i));
	}
	connect(df,SIGNAL(statusMessage(const QString &)),
		frame,SLOT(setText(const QString &)));
	connect(df,SIGNAL(setProgress(int)),
		progress,SLOT(setProgress(int)));
	df->show();
}


void FloppyData::initMenu()
{
	DEBUGSETUP;

	// Set up the menus first
	//
	KMenuBar *menu = menuBar();
	KPopupMenu *file = new KPopupMenu(this,"file");
	menu->insertItem(i18n("&File"),file);

	KPopupMenu *settings = new KPopupMenu(this,"settings");
	menu->insertItem(i18n("&Settings"),settings);

	KPopupMenu *help = helpMenu();
	menu->insertItem(i18n("&Help"),help);

	// Handle actions
	//
	KActionCollection *actions = actionCollection();

	KAction *a ;

	formatAction = a = new KAction(i18n("&Format"),
		Qt::CTRL + Qt::Key_F,
		this,SLOT(format()), actions, "format");
	a->plug(file);

	a = KStdAction::quit(this, SLOT(quit()), actions);
	a->plug(file);

	a = hideProgress = new KToggleAction(i18n("Hide &Messages"),
		KShortcut(),
		this,SLOT(toggleProgress()),actions);
	a->plug(settings);

	a = hideConfig = new KToggleAction(i18n("Hide &Configuration"),
		KShortcut(),
		this,SLOT(toggleConfig()),actions);
	a->plug(settings);

	a = showAllFormats = new KToggleAction(i18n("Show All &Formats"),
		KShortcut(),
		this,SLOT(showFormats()),actions);
	a->plug(settings);
}


FloppyData::~FloppyData()
{
	DEBUGSETUP;
}


void FloppyData::quit()
{
	for (unsigned int i=0; i<availableFormats.count(); i++)
	{
		DiskFormat *df = availableFormats.at(i);
		if (df) df->quit();
	}

	writeSettings();
	kapp->quit();
	delete this;
}

/* virtual */ void FloppyData::closeEvent(QCloseEvent *)
{
	quit();
}

void FloppyData::reset()
{
	DEBUGSETUP;

	formating = false;

	progress->setValue(0);
	formatbutton->setText(i18n("&Format"));
	filesystemComboBox->setEnabled(true);
	formatAction->setEnabled(true);

	int index = filesystemComboBox->currentItem();
	DiskFormat *df = availableFilesystems.at(index)->format();
	if (df)
	{
		disconnect(df,SIGNAL(formatDone(int)),this,SLOT(reset()));
		df->setEnabled(true);
		df->quit();
	}
}

void FloppyData::format(){
	DEBUGSETUP;

	if(formating){
		reset();
		return;
	}


	if (KMessageBox::warningContinueCancel(this,
		i18n("Formatting will erase all data on the disk.\n"
        		"Are you sure you wish to proceed?"),
		i18n("Proceed?"),
		KStdGuiItem::cont()) == KMessageBox::Continue)
        {

		formatbutton->setText(i18n("A&bort"));
		filesystemComboBox->setEnabled(false);
		formatAction->setEnabled(false);

		int index = filesystemComboBox->currentItem();
		FilesystemData *p = availableFilesystems.at(index);
		if (!p) return;

		DiskFormat *df = p->format();
		if (df)
		{
			df->setEnabled(false);
			connect(df,SIGNAL(formatDone(int)),this,SLOT(reset()));
			df->format(p);
			formating=true;
		}
	}
}

void FloppyData::formatSelected(int i)
{
	DEBUGSETUP;
	DEBUGS(QString("  Format %1").arg(i).latin1());

	if (showAllFormats->isChecked())
	{
		// Fast way. All filesystems are shown anyway.
		//
		//
		FilesystemData *p = availableFilesystems.at(i);
		if (!p) return;
		DiskFormat *df = p->format();
		if (!df) return;
		stack->raiseWidget(df);
	}
	else
	{
		// Show way. Have to find the i'th basic filesystem.
		for (unsigned int j=0; j<availableFilesystems.count(); j++)
		{
			FilesystemData *p = availableFilesystems.at(j);
			if (p->isBasic())
			{
				i--;
			}
			if (i<0)
			{
				stack->raiseWidget(p->format());
				break;
			}
		}
	}
}

void FloppyData::toggleProgress()
{
	DEBUGSETUP;

	if (!hideProgress->isChecked())
	{
		frame->show();
		progress->show();
		adjustSize();
	}
	else
	{
		frame->hide();
		progress->hide();
		QSize s = size();
		s.setHeight(s.height()-frame->height()-progress->height()-14);
		setMinimumSize(s);
		resize(s);
	}
}

void FloppyData::toggleConfig()
{
	DEBUGSETUP;

	if (!hideConfig->isChecked())
	{
		stack->show();
		adjustSize();
	}
	else
	{
		stack->hide();
		QSize s = size();
		s.setHeight(s.height()-stack->height()-14);
		setMinimumSize(s);
		resize(s);
	}
}

void FloppyData::showFormats()
{
	DEBUGSETUP;

	bool all = showAllFormats->isChecked();
	int count = 0;

	QString currentFS = filesystemComboBox->text(filesystemComboBox->currentItem());
	int newIndex = -1;

	filesystemComboBox->clear();
	for (unsigned int i=0; i<availableFilesystems.count(); i++)
	{
		if (all || availableFilesystems.at(i)->isBasic())
		{
			filesystemComboBox->insertItem(
				availableFilesystems.at(i)->name());
			if (availableFilesystems.at(i)->name() == currentFS)
			{
				newIndex=count;
			}
			count++;
		}
	}

	if (newIndex<0) newIndex=0; // Show warning too?
	filesystemComboBox->setCurrentItem(newIndex);
	formatSelected(newIndex);

	DEBUGS(QString("  Inserted %1 filesystems.").arg(count).latin1());
}

void FloppyData::writeSettings()
{
	DEBUGSETUP;

        KConfig *config = kapp->config();
	config->setGroup("GeneralData");

	QString filesystemconfig = filesystemComboBox->currentText();
	filesystemconfig = filesystemconfig.stripWhiteSpace();

	config->writeEntry("Filesystem",filesystemconfig);
	config->writeEntry("ShowProgress",!hideProgress->isChecked());
	config->writeEntry("ShowConfig",!hideConfig->isChecked());
	config->writeEntry("ShowAllFormats",showAllFormats->isChecked());

	for (unsigned int i=0; i<availableFormats.count(); i++)
	{
		DiskFormat *df = availableFormats.at(i);
		if (df) df->writeSettings(config);

	}
}

void FloppyData::readSettings()
{
	DEBUGSETUP;

        KConfig *config = kapp->config();
	config->setGroup("GeneralData");

	// QString savedFormat = config->readEntry("Filesystem",i18n("Dos"));

	hideProgress->setChecked(!config->readBoolEntry("ShowProgress",true));
	hideConfig->setChecked(!config->readBoolEntry("ShowConfig",true));
	showAllFormats->setChecked(config->readBoolEntry("ShowAllFormats",false));

	for (unsigned int i=0; i < availableFormats.count(); i++)
	{
		DiskFormat *df = availableFormats.at(i);
		if (df) df->readSettings(config);

	}
}

void FloppyData::readSettings2()
{
	DEBUGSETUP;

        KConfig *config = kapp->config();
	config->setGroup("GeneralData");

	QString savedFormat = config->readEntry("Filesystem",i18n("Dos"));
	DEBUGS(QString("  Looking for %1").arg(savedFormat).latin1());

	for (int i=0; i < filesystemComboBox->count(); i++)
	{
		DEBUGS(QString("  Seeing %1.").arg(filesystemComboBox->text(i)).latin1());

		if (filesystemComboBox->text(i) == savedFormat)
		{
			filesystemComboBox->setCurrentItem(i);
			formatSelected(i);
			break;
		}
	}
}

#ifdef BLACKADDER
static void keep_old_i18n_just_in_case()
{
	QString str;
	QString device,errstring;
	int bblock,blocks,badblocks;
	QString fserrstring;


	str = (i18n( "KDE Floppy") );
    str = i18n(
	      "Cannot access %1\nMake sure that the device exists and that "
	      "you have write permission to it.").arg(device);
    str = i18n("Cannot find fdformat.");
    str = i18n("Cannot find mke2fs");
    str = i18n("Cannot find mkdosfs");
  str = (i18n("Formatting..."));
    str = i18n("Cannot start a new program: fork() failed.");
    str = i18n(
		"Cannot access floppy or floppy drive.\n"
		"Please insert a floppy and make sure that you "
		"have selected a valid floppy drive.");
str = i18n("Cannot format: %1\n%2").arg(device).arg(errstring);
      str = i18n("Block %1 is bad. Continuing...").arg(bblock);
    str = i18n(
		"Cannot access floppy or floppy drive.\n"
		"Please insert a floppy and make sure that you "
 		"have selected a valid floppy drive.");
  str = i18n("Cannot create a filesystem on: %1\n%2")
	      .arg(device).arg(fserrstring);
    str = i18n(
		"The floppy was successfully formatted.\n"
		"Blocks marked bad: %1\n"
		"Raw Capacity: %2\n")
		.arg(badblocks)
		.arg((blocks - badblocks)*1024);
    str = i18n("All files were successfully erased.");
  str = (i18n("Creating Filesystem..."));
    str = i18n("Cannot start a new program\nfork() failed.");

}
#endif



