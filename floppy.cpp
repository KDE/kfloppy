/*
    KFloppy
    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu


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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */


#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

#include <kconfig.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kpushbutton.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kprogress.h>
#include <klocale.h>

#include "floppy.moc"
#include "format.h"

FloppyData::FloppyData(QWidget * parent, const char * name)
 : KDialog( parent, name ),
	formatActions(0L)
{

	formating = false;
	quickerase = false;
	abort = false;
	counter = 0;
	tracks = 0;
	blocks = 0;

        QVBoxLayout* ml = new QVBoxLayout( this, 10 );

        QHBoxLayout* h1 = new QHBoxLayout( ml );

        QVBoxLayout* v1 = new QVBoxLayout( h1 );
        h1->addSpacing( 5 );

        QGridLayout* g1 = new QGridLayout( v1, 3, 2 );

        label1 = new QLabel(this);
	label1->setText(i18n("Floppy drive:"));
        g1->addWidget( label1, 0, 0, AlignLeft );


	deviceComboBox = new QComboBox( false, this, "ComboBox_1" );
	g1->addWidget( deviceComboBox, 0, 1, AlignLeft );

	deviceComboBox->insertItem(i18n("Primary"));
	deviceComboBox->insertItem(i18n("Secondary"));

        label2 = new QLabel(this);
	label2->setText(i18n("Size:"));
        g1->addWidget( label2, 1, 0, AlignLeft );

	densityComboBox = new QComboBox( false, this, "ComboBox_1" );
	g1->addWidget( densityComboBox, 1, 1, AlignLeft );

	densityComboBox->insertItem(i18n("3.5\" 1.44MB"));
	densityComboBox->insertItem(i18n("3.5\" 720KB"));
	densityComboBox->insertItem(i18n("5.25\" 1.2MB"));
	densityComboBox->insertItem(i18n("5.25\" 360KB"));


        label3 = new QLabel(this);
	label3->setText(i18n("File system:"));
        g1->addWidget( label3, 2, 0, AlignLeft );

	filesystemComboBox = new QComboBox( false, this, "ComboBox_2" );
	g1->addWidget( filesystemComboBox, 2, 1, AlignLeft );

	filesystemComboBox->insertItem(i18n("DOS"));
#ifdef ANY_LINUX
	filesystemComboBox->insertItem(i18n("ext2"));
#endif
#ifdef ANY_BSD
	filesystemComboBox->insertItem(i18n("UFS"));
#endif

        v1->addSpacing( 10 );

        buttongroup = new QButtonGroup( this, "ButtonGroup_1" );
	buttongroup->setFrameStyle( 49 );
	v1->addWidget( buttongroup );

        QVBoxLayout* v2 = new QVBoxLayout( buttongroup, 10 );

	quick = new QRadioButton( buttongroup, "RadioButton_2" );
	quick->setText(i18n( "Q&uick format") );
        v2->addWidget( quick, AlignLeft );

	fullformat = new QRadioButton( buttongroup, "RadioButton_3" );
	fullformat->setText(i18n( "Fu&ll format") );
	fullformat->setChecked(true);
        v2->addWidget( fullformat, AlignLeft );

	verifylabel = new QCheckBox( buttongroup, "RadioButton_4" );
	verifylabel->setText(i18n( "&Verify integrity" ));
	verifylabel->setChecked(true);
	v2->addWidget( verifylabel, AlignLeft );

	labellabel = new QCheckBox( buttongroup, "RadioButton_4" );
	labellabel->setText(i18n( "Volume &label:") );
	labellabel->setChecked(true);
        v2->addWidget( labellabel, AlignLeft );

        QHBoxLayout* h2 = new QHBoxLayout( v2 );
        h2->addSpacing( 20 );

	lineedit = new QLineEdit( buttongroup, "Lineedit" );
	lineedit->setText(i18n( "KDE Floppy") );
	lineedit->setMaxLength(11);
        lineedit->setMinimumWidth( lineedit->sizeHint().width() );
        h2->addWidget( lineedit, AlignRight );

	connect(labellabel,SIGNAL(toggled(bool)),lineedit,SLOT(setEnabled(bool)));

	QVBoxLayout* v3 = new QVBoxLayout( h1 );

	formatbutton = new KPushButton( this, "PushButton_3" );
	formatbutton->setText(i18n( "&Format") );
	formatbutton->setAutoRepeat( false );
	connect(formatbutton,SIGNAL(clicked()),this,SLOT(format()));
        v3->addWidget( formatbutton );

        v3->addStretch( 1 );

	//Setup the Help Menu
	helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);

	helpbutton = new KPushButton( KStdGuiItem::help(), this );
	helpbutton->setAutoRepeat( false );
	helpbutton->setPopup(helpMenu->menu());
	v3->addWidget( helpbutton );

	quitbutton = new KPushButton( KStdGuiItem::quit(), this );
	quitbutton->setAutoRepeat( false );
	connect(quitbutton,SIGNAL(clicked()),this,SLOT(quit()));
	 v3->addWidget( quitbutton );

        ml->addSpacing( 10 );

	frame = new QLabel( this, "NewsWindow" );
	frame->setMinimumHeight( 50 );
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	frame->setAlignment(AlignCenter|WordBreak|ExpandTabs);
        ml->addWidget( frame );

	progress = new KProgress( this, "Progress" );
        progress->setMinimumHeight( 30 );
        ml->addWidget( progress );

	readSettings();
	setWidgets();

	findExecutables();

    int maxW = QMAX( deviceComboBox->sizeHint().width(),
                     densityComboBox->sizeHint().width() );
    maxW = QMAX( maxW, filesystemComboBox->sizeHint().width() );
    deviceComboBox->setMinimumWidth( maxW );
    densityComboBox->setMinimumWidth( maxW );
    filesystemComboBox->setMinimumWidth( maxW );

    setMinimumSize(minimumSizeHint());
}


FloppyData::~FloppyData()
{
  delete formatActions;
}

void FloppyData::closeEvent(QCloseEvent*)
{
  quit();
}

void FloppyData::keyPressEvent(QKeyEvent *e)
{
	switch(e->key()) {
	case Qt::Key_F1:
		kapp->invokeHelp();
		break;
	default:
		KDialog::keyPressEvent(e);
		return;
	}
}

void FloppyData::show() {
  setCaption(i18n("KDE Floppy Formatter"));
  KDialog::show();
}

bool FloppyData::findDevice()
{
  drive=-1;
  if( deviceComboBox->currentText() == i18n("Primary") )
  {
    drive=0;
  }
  if( deviceComboBox->currentText() == i18n("Secondary") )
  {
    drive=1;
  }

  blocks=-1;

    if( densityComboBox->currentText() == i18n("3.5\" 1.44MB")){
      blocks = 1440;
    }
    else
    if( densityComboBox->currentText() == i18n("3.5\" 720KB")){
      blocks = 720;
    }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 1.2MB")){
      blocks = 1200;
      }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 360KB")){
      blocks = 360;
      }


  return true;
}

bool FloppyData::setInitialDevice(const QString& dev)
{
  int drive = -1;
  if (dev.startsWith("/dev/fd0"))
    drive = 0;
  if (dev.startsWith("/dev/fd1"))
    drive = 1;

  bool ok = (drive>=0);
  if (ok)
    deviceComboBox->setCurrentItem(drive);
  return ok;
}


void FloppyData::findExecutables()
{
	bool fruitful = true ;
	fruitful &= FDFormat::runtimeCheck();
	fruitful &= FATFilesystem::runtimeCheck();
#ifdef ANY_BSD
	fruitful &= UFSFilesystem::runtimeCheck();
#else
#ifdef ANY_LINUX
	fruitful &= Ext2Filesystem::runtimeCheck();
#else
	fruitful = false;
#endif
#endif

  if (!fruitful) {
    formatbutton->setEnabled(false);
    KMessageBox::error(this,
    	i18n("KFloppy cannot find the support programs needed "
		"for sensible operation."));
  }
}

void FloppyData::quit(){
  if (formatActions) formatActions->quit();
  writeSettings();
  kapp->quit();
  delete this;
}

void FloppyData::setEnabled(bool b)
{
  if (b)
	unsetCursor();
  else
	setCursor(QCursor(WaitCursor));
  label1->setEnabled(b);
  deviceComboBox->setEnabled(b);
  label2->setEnabled(b);
  densityComboBox->setEnabled(b);
  label3->setEnabled(b);
  filesystemComboBox->setEnabled(b);
  buttongroup->setEnabled(b);
  quick->setEnabled(b);
  fullformat->setEnabled(b);
  verifylabel->setEnabled(b);
  labellabel->setEnabled(b);
  lineedit->setEnabled(b);
  helpbutton->setEnabled(b);
  quitbutton->setEnabled(b);
  formatbutton->setEnabled(b);
}

void FloppyData::reset()
{
  DEBUGSETUP;

  formating = false;
  quickerase = false;

  if (formatActions)
  {
    formatActions->quit();
    delete formatActions;
    formatActions = 0L;
  }

  progress->setValue(0);
  formatbutton->setText(i18n("&Format"));
  setEnabled(true);
}

void FloppyData::format(){

  errstring = "";
  formatstring ="";
  //mdev = "";

  if(formating){
    abort = true;
    reset();
    return;
  }

  frame->clear();

  if (KMessageBox::warningContinueCancel(0,
   i18n("Formatting will erase all data on the disk.\n"
        "Are you sure you wish to proceed?"), i18n("Proceed?") ) !=
	KMessageBox::Continue)
        {
	return;
	}

  // formatbutton->setText(i18n("A&bort"));
  setEnabled(false);

	if (!findDevice())
	{
		reset();
		return;
	}

	if (formatActions) delete formatActions;
	formatActions = new KFActionQueue(this);

	connect(formatActions,SIGNAL(status(const QString &,int)),
		this,SLOT(formatStatus(const QString &,int)));
	connect(formatActions,SIGNAL(done(KFAction *,bool)),
		this,SLOT(reset()));

	if (quick->isChecked())
	{
		quickerase=true;
		formating=false;
		// No fdformat to push
	}
	else
	{
		FDFormat *f = new FDFormat(this);
		f->configureDevice(drive,blocks);
		f->configure(verifylabel->isChecked());
		formatActions->queue(f);
	}

	if (filesystemComboBox->currentItem() == 0)
	{
		FATFilesystem *f = new FATFilesystem(this);
		connect(f,SIGNAL(status(const QString &,int)),
			this,SLOT(formatStatus(const QString &,int)));
		connect(f,SIGNAL(done(KFAction *,bool)),
			this,SLOT(reset()));
		f->configure(verifylabel->isChecked(),
			labellabel->isChecked(),
			lineedit->text());
		f->configureDevice(drive,blocks);
		formatActions->queue(f);
	}
	else if (filesystemComboBox->currentItem() == 1)
	{
		FloppyAction *f = 0L;
#ifdef ANY_LINUX
		Ext2Filesystem *e2f = new Ext2Filesystem(this);
		e2f->configure(verifylabel->isChecked(),
			labellabel->isChecked(),
			lineedit->text());
		f=e2f;
#else
#ifdef ANY_BSD
		f = new UFSFilesystem(this);
#endif
#endif

		if (f)
		{
			connect(f,SIGNAL(status(const QString &,int)),
				this,SLOT(formatStatus(const QString &,int)));
			connect(f,SIGNAL(done(KFAction *,bool)),
				this,SLOT(reset()));
			f->configureDevice(drive,blocks);
			formatActions->queue(f);
		}
	}



	formatActions->exec();
}

void FloppyData::formatStatus(const QString &s,int p)
{
	if (!s.isEmpty())
		frame->setText(s);

	if ((0<=p) && (p<=100))
		progress->setValue(p);
}

#if 0
void FloppyData::readStdout(KProcess *, char *buffer, int buflen)
{
  bool increment = true;

  formatstring = QString::fromLocal8Bit(buffer, buflen);

  if (formatstring.contains("track"))
  {
    int pos = formatstring.find('\n');
    QString newstring;

    if(pos != -1)
      newstring = formatstring.left(pos);
    else
      newstring = formatstring;

    frame->setText(newstring);
    increment =false;
  }


  if (increment)
  {
    counter ++;
    progress->setValue(counter);
  }

  kdDebug(2002) << "STDOUT: " << formatstring << endl;
}

void FloppyData::readStderr(KProcess *, char *buffer, int buflen){

  char mybuffer[1001];
  int amount;


  if(buflen > 1000)
    amount = 1000;
  else
    amount = buflen;

  memcpy(mybuffer,buffer,amount);
  mybuffer[amount] = '\0';

  abort = true;
  errstring  +=mybuffer;


  errtimer->start(300,true);

  kdDebug(2002) << "STDERR: " << mybuffer << endl;
}

void FloppyData::errslot(){

  abort = true;

  if(errstring.contains("ioctl(FDFMTBEG)")){

    QString str = i18n(
		"Cannot access floppy or floppy drive.\n"
		"Please insert a floppy and make sure that you "
		"have selected a valid floppy drive.");

    KMessageBox::error(this, str);

    reset();
    return;

  }

  QString str = i18n("Cannot format: %1\n%2").arg(drive).arg(errstring);

  KMessageBox::error(this, str);

  reset();
}

void FloppyData::readfsStdout(KProcess *, char *buffer, int buflen){
  char mybuffer[1001];
  int amount;

  if(buflen > 1000)
    amount = 1000;
  else
    amount = buflen;

  memcpy(mybuffer,buffer,amount);
  mybuffer[amount] = '\0';

  fsstring += mybuffer;
  if( fsstring.find('\n') == -1)
    return;

  QString string ;
  QString newstring = fsstring;


  int i;

  while( (i =newstring.find('\n')) != -1){

    QString mystring;
    mystring = newstring.left(i);

    kdDebug(2002) << "NEWLINE: " << mystring.data() << endl;

    if(findKeyWord(mystring,"BBF ")){
      int bblock = mystring.left(8).toInt();
      QString mstr;
      mstr = i18n("Block %1 is bad. Continuing...").arg(bblock);
      frame->setText(mstr);
    }

    if(findKeyWord(mystring,"TNBB ")){
      badblocks = mystring.left(8).toInt();
    }
    newstring = newstring.mid(i+1,newstring.length());
  }

  counter += findKeyWord(fsstring,"BLOCK");

  kdDebug(2002) << "Block Counter: " << counter << endl;

  if(quickerase){

    if(findKeyWord(fsstring,"START"))
      counter ++;

    if(findKeyWord(fsstring,"EXIT"))
      counter ++;


  }

  fsstring = newstring;

  progress->setValue(counter);

  kdDebug(2002) << "STDOUT: " << mybuffer << endl;
}


void FloppyData::readfsStderr(KProcess *, char *buffer, int buflen){

  char mybuffer[1001];
  int amount;


  if(buflen > 1000)
    amount = 1000;
  else
    amount = buflen;

  memcpy(mybuffer,buffer,amount);
  mybuffer[amount] = '\0';

  // skip version message
  int pos=0;
  if (fserrstring.isEmpty() && strncmp(mybuffer, "mke2fs", 6) == 0)
    {
      pos = QString(mybuffer).find('\n');
      if (pos+1 == amount)
	return;
      pos++;
    }
  if (fserrstring.isEmpty() && strncmp(mybuffer, "/sbin/mkdosfs", 13) == 0)
    {
      pos = QString(mybuffer).find('\n');
      if (pos+1 == amount)
	return;
      pos++;
    }

  abort = true;
  fserrstring += mybuffer+pos;

  // the timers are put in so that I get all of the error message.
  fserrtimer->start(300,true);

  kdDebug(2002) << "STDOUT: " << mybuffer << endl;
}

void FloppyData::fserrslot(){


  if(fserrstring.contains("No such device")){

    QString str = i18n(
		"Cannot access floppy or floppy drive.\n"
		"Please insert a floppy and make sure that you "
 		"have selected a valid floppy drive.");

    KMessageBox::sorry(this, str);

    reset();
    return;
  }

  reset();

  QString str = i18n("Cannot create a filesystem on: %1\n%2")
	      .arg(drive).arg(fserrstring);

  KMessageBox::error(this, str);
}


void FloppyData::cfdone(KProcess*){

    mytimer->start(10,true);

}

void FloppyData::cf2done(){


  bool lcquick;
  lcquick = quickerase;

  mytimer->stop();

  reset();

  if(abort)
    return;

  if(!lcquick){
    QString str = i18n(
		"The floppy was successfully formatted.\n"
		"Blocks marked bad: %1\n"
		"Raw Capacity: %2\n")
		.arg(badblocks)
		.arg((blocks - badblocks)*1024);

    KMessageBox::information(this, str);
  }
  else{
    QString str = i18n("All files were successfully erased.");

    KMessageBox::information(this, str);
  }
}

int FloppyData::findKeyWord(QString & string,const QString & word){

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
#endif

void FloppyData::writeSettings(){

        config = kapp->config();
	config->setGroup("GeneralData");

	densityconfig = densityComboBox->currentText();
	densityconfig = densityconfig.stripWhiteSpace();
	filesystemconfig = filesystemComboBox->currentText();
	filesystemconfig = filesystemconfig.stripWhiteSpace();
	driveconfig = deviceComboBox->currentText();
	driveconfig = driveconfig.stripWhiteSpace();

	quickformatconfig = quick->isChecked();

	labelnameconfig = lineedit->text();
	labelnameconfig = labelnameconfig.stripWhiteSpace();

	labelconfig = labellabel->isChecked();

	verifyconfig = verifylabel->isChecked();

	config->writeEntry("CreateLabel",labelconfig);
	config->writeEntry("Label",labelnameconfig);


	config->writeEntry("QuickFormat",quickformatconfig);
	config->writeEntry("FloppyDrive",driveconfig);
	config->writeEntry("Density",densityconfig);
	config->writeEntry("Filesystem",filesystemconfig);
	config->writeEntry("Verify",verifyconfig);
	config->sync();

}

void FloppyData::readSettings(){

        config = kapp->config();
	config->setGroup("GeneralData");

	verifyconfig = config->readNumEntry("Verify", 1);
	labelconfig = config->readNumEntry("CreateLabel",1);
	labelnameconfig = config->readEntry("Label",i18n("KDE Floppy"));
	quickformatconfig = config->readNumEntry("QuickFormat",0);
	driveconfig = config->readEntry("FloppyDrive","A: 3.5");
	densityconfig = config->readEntry("Density",i18n("HD"));
	filesystemconfig = config->readEntry("Filesystem",i18n("Dos"));

}

void FloppyData::setWidgets(){

  labellabel->setChecked(labelconfig);
  verifylabel->setChecked(verifyconfig);
  quick->setChecked(quickformatconfig);

  fullformat->setChecked(!quickformatconfig);
  lineedit->setText(labelnameconfig);

  for(int i = 0 ; i < deviceComboBox->count(); i++){
    if ( (QString) deviceComboBox->text(i) == driveconfig){
      deviceComboBox->setCurrentItem(i);
    }
  }

  for(int i = 0 ; i < filesystemComboBox->count(); i++){
    if ( (QString) filesystemComboBox->text(i) == filesystemconfig){
      filesystemComboBox->setCurrentItem(i);
    }
  }

  for(int i = 0 ; i < densityComboBox->count(); i++){
    if ( (QString) densityComboBox->text(i) == densityconfig){
      densityComboBox->setCurrentItem(i);
    }
  }
}


