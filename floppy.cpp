
/*

    $Id$

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */


#include "floppy.h"
#include "floppy.moc"

#include <iostream.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>


FloppyData::FloppyData(QWidget * parent, const char * name)
 : KDialog( parent, name )
{

        proc = 0L;
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
	label1->setText(i18n("Floppy Drive:"));
        g1->addWidget( label1, 0, 0, AlignLeft );


	deviceComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	g1->addWidget( deviceComboBox, 0, 1, AlignLeft );

	deviceComboBox->insertItem(i18n("Primary"));
	deviceComboBox->insertItem(i18n("Secondary"));

        label2 = new QLabel(this);
	label2->setText(i18n("Size:"));
        g1->addWidget( label2, 1, 0, AlignLeft );

	densityComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	g1->addWidget( densityComboBox, 1, 1, AlignLeft );

	densityComboBox->insertItem(i18n("3.5\" 1.44MB"));
	densityComboBox->insertItem(i18n("3.5\" 720KB"));
	densityComboBox->insertItem(i18n("5.25\" 1.2MB"));
	densityComboBox->insertItem(i18n("5.25\" 360KB"));


        label3 = new QLabel(this);
	label3->setText(i18n("File System:"));
        g1->addWidget( label3, 2, 0, AlignLeft );

	filesystemComboBox = new QComboBox( FALSE, this, "ComboBox_2" );
	g1->addWidget( filesystemComboBox, 2, 1, AlignLeft );

	filesystemComboBox->insertItem("DOS");
	filesystemComboBox->insertItem("ext2");

        v1->addSpacing( 10 );

        buttongroup = new QButtonGroup( this, "ButtonGroup_1" );
	buttongroup->setFrameStyle( 49 );
	v1->addWidget( buttongroup );

        QVBoxLayout* v2 = new QVBoxLayout( buttongroup, 10 );

	quick = new QRadioButton( buttongroup, "RadioButton_2" );
	quick->setText(i18n( "Q&uick Format") );
        v2->addWidget( quick, AlignLeft );

	fullformat = new QRadioButton( buttongroup, "RadioButton_3" );
	fullformat->setText(i18n( "Fu&ll Format") );
	fullformat->setChecked(TRUE);
        v2->addWidget( fullformat, AlignLeft );

	verifylabel = new QCheckBox( buttongroup, "RadioButton_4" );
	verifylabel->setText(i18n( "&Verify Integrity" ));
	verifylabel->setChecked(TRUE);
	v2->addWidget( verifylabel, AlignLeft );

	labellabel = new QCheckBox( buttongroup, "RadioButton_4" );
	labellabel->setText(i18n( "Volume &Label:") );
	labellabel->setChecked(TRUE);
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

	formatbutton = new QPushButton( this, "PushButton_3" );
	formatbutton->setText(i18n( "&Format") );
	formatbutton->setAutoRepeat( FALSE );
	connect(formatbutton,SIGNAL(clicked()),this,SLOT(format()));
        v3->addWidget( formatbutton );

        v3->addStretch( 1 );

	//Setup the Help Menu
	helpMenu = new KHelpMenu(this, KGlobal::instance()->aboutData(), false);

	helpbutton = new QPushButton( this, "PushButton_4" );
	helpbutton->setText(i18n( "&Help") );
	helpbutton->setAutoRepeat( FALSE );
	helpbutton->setPopup(helpMenu->menu());
	v3->addWidget( helpbutton );

	quitbutton = new QPushButton( this, "PushButton_1" );
	quitbutton->setText(i18n( "&Quit") );
	quitbutton->setAutoRepeat( FALSE );
	connect(quitbutton,SIGNAL(clicked()),this,SLOT(quit()));
	 v3->addWidget( quitbutton );

        ml->addSpacing( 10 );

	frame = new QLabel( this, "NewsWindow" );
	frame->setMinimumHeight( 50 );
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	frame->setAlignment(AlignCenter|WordBreak|ExpandTabs);
        ml->addWidget( frame );

	progress = new KProgress( this, "Progress" );
	progress->setFont(QFont("Helvetica",12,QFont::Normal));
	progress->setBarColor(QApplication::winStyleHighlightColor());
        progress->setMinimumHeight( 30 );
        ml->addWidget( progress );

	mytimer = new QTimer;
	connect(mytimer,SIGNAL(timeout()),this,SLOT(cf2done()));

	fserrtimer = new QTimer;
	connect(fserrtimer,SIGNAL(timeout()),this,SLOT(fserrslot()));

	errtimer = new QTimer;
	connect(errtimer,SIGNAL(timeout()),this,SLOT(errslot()));

	readSettings();
	setWidgets();

	findExecutables();

    int maxW = QMAX( deviceComboBox->sizeHint().width(),
                     densityComboBox->sizeHint().width() );
    maxW = QMAX( maxW, filesystemComboBox->sizeHint().width() );
    deviceComboBox->setMinimumWidth( maxW );
    densityComboBox->setMinimumWidth( maxW );
    filesystemComboBox->setMinimumWidth( maxW );
}


FloppyData::~FloppyData()
{
}

void FloppyData::closeEvent(QCloseEvent*){

  quit();

}

void FloppyData::show() {
  setCaption(i18n("KDE Floppy Formatter"));
  KDialog::show();
}


bool FloppyData::findDevice()
{
  if( deviceComboBox->currentText() == i18n("Primary") ){
    if( densityComboBox->currentText() == i18n("3.5\" 1.44MB")){
      device = "/dev/fd0H1440";
      blocks = 1440;
      tracks = 80;
      if( access(QFile::encodeName(device),W_OK) < 0){
	device = "/dev/fd0u1440";
      }
    }
    else
    if( densityComboBox->currentText() == i18n("3.5\" 720KB")){
      device = "/dev/fd0D720";
      blocks = 720;
      tracks = 80;
      if( access(QFile::encodeName(device),W_OK) < 0){
	device = "/dev/fd0u720";
      }
    }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 1.2MB")){
      device = "/dev/fd0h1200";
      blocks = 720;
      tracks = 80;
      }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 360KB")){
      device = "/dev/fd0h360";
      blocks = 720;
      tracks = 80;
      }
    }
  if( deviceComboBox->currentText() == i18n("Secondary") ){
    if( densityComboBox->currentText() == i18n("3.5\" 1.44MB")){
      device = "/dev/fd1H1440";
      blocks = 1440;
      tracks = 80;
      if( access(QFile::encodeName(device),W_OK) < 0){
	device = "/dev/fd1u1440";
      }
    }
    else
    if( densityComboBox->currentText() == i18n("3.5\" 720KB")){
      device = "/dev/fd1D720";
      blocks = 720;
      tracks = 80;
      if( access(QFile::encodeName(device),W_OK) < 0){
	device = "/dev/fd1u720";
      }
    }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 1.2MB")){
      device = "/dev/fd1h1200";
      blocks = 720;
      tracks = 80;
      }
    else
    if( densityComboBox->currentText() == i18n("5.25\" 360KB")){
      device = "/dev/fd1h360";
      blocks = 720;
      tracks = 80;
      }
    }

  if( access(QFile::encodeName(device),W_OK) < 0){

    QString str = i18n(
	      "Cannot access %1\nMake sure that the device exists and that\n"
	      "you have write permission to it.").arg(device);
    KMessageBox::error(this, str);

    //formatbutton->setEnabled(FALSE);
    return false;

  }

  return true;
}


void FloppyData::findExecutables()
{
  QString path = getenv("PATH");
  path.append(":/usr/sbin:/sbin");

  fdformat = KGlobal::dirs()->findExe("fdformat", path);
  mke2fs = KGlobal::dirs()->findExe("mke2fs", path);
  mkdosfs = KGlobal::dirs()->findExe("mkdosfs", path);
  QString str = "";
  if (fdformat.isEmpty()) {
    str = i18n("Cannot find fdformat.");
  }

  if (mke2fs.isEmpty()) {
    str = i18n("Cannot find mke2fs");
  }

  if (mkdosfs.isEmpty()) {
    str = i18n("Cannot find mkdosfs");
  }

  if (str != "") {
    formatbutton->setEnabled(FALSE);
    KMessageBox::error(this, str);
  }
}

void FloppyData::quit(){

  if(proc){
    if(proc->isRunning())
      proc->kill();
    
  }
  writeSettings();
  kapp->quit();
}

void FloppyData::reset(){


  formating = false;
  quickerase = false;

  if(proc){
    if(proc->isRunning())
      proc->kill();
  }
  proc = 0L;
  progress->setValue(0);
  frame->clear();
  formatbutton->setText(i18n("&Format"));
  label1->setEnabled(true);
  deviceComboBox->setEnabled(true);
  label2->setEnabled(true);
  densityComboBox->setEnabled(true);
  label3->setEnabled(true);
  filesystemComboBox->setEnabled(true);
  buttongroup->setEnabled(true);
  quick->setEnabled(true);
  fullformat->setEnabled(true);
  verifylabel->setEnabled(true);
  labellabel->setEnabled(true);
  lineedit->setEnabled(true);
  helpbutton->setEnabled(true);
  quitbutton->setEnabled(true);
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
  if (!formating) {
        if (KMessageBox::warningContinueCancel(0, "Formatting will erase all data on the disk.\n"
        "Are you sure you wish to proceed?", "Proceed?" ) == KMessageBox::Continue)
        {

  formatbutton->setText(i18n("A&bort"));
  label1->setEnabled(false);
  deviceComboBox->setEnabled(false);
  label2->setEnabled(false);
  densityComboBox->setEnabled(false);
  label3->setEnabled(false);
  filesystemComboBox->setEnabled(false);
  buttongroup->setEnabled(false);
  quick->setEnabled(false);
  fullformat->setEnabled(false);
  verifylabel->setEnabled(false);
  labellabel->setEnabled(false);
  lineedit->setEnabled(false);
  helpbutton->setEnabled(false);
  quitbutton->setEnabled(false);



  if(!findDevice()){
    reset();
    return;
  }

  if(quick->isChecked()){
    quickerase = true;
    formating = false;
    createfilesystem();
    return;
  }

  frame->setText(i18n("Formatting ..."));
  badblocks = 0;
  abort = false;
  formating = true;
  progress->setRange(0,tracks*2);
  progress->setValue(0);
  counter = 0;

  proc = new KProcess;
  *proc << fdformat;
  if (!verifylabel->isChecked()) {
    *proc << "-n";
  }
  *proc << device;

  connect(proc, SIGNAL(processExited(KProcess *)),this, SLOT(formatdone(KProcess*)));

  connect(proc, SIGNAL(receivedStdout(KProcess *,char*,int)),this,
	  SLOT(readStdout(KProcess*,char*,int)));

  connect(proc, SIGNAL(receivedStderr(KProcess *,char*,int)),this,
	  SLOT(readStderr(KProcess*,char*,int)));

  bool result = proc->start(KProcess::NotifyOnExit , KProcess::All);

  if(!result){
    QString str = i18n("Cannot start a new program: fork() failed.");
    KMessageBox::error(this, str);
    reset();
  }
}
}
}

void FloppyData::formatdone(KProcess*){

  if(proc){
    if(proc->isRunning())
      proc->kill();
  }

  delete proc;
  proc = 0L;

  if(!abort)
    createfilesystem();

}

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
#ifdef MY_DEBUG
    printf("STDOUT:%s\n",mybuffer);
#endif 
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
#ifdef MY_DEBUG
  printf("STDERR:%s\n",mybuffer);
#endif
}

void FloppyData::errslot(){

  abort = true;

  if(errstring.contains("ioctl(FDFMTBEG)")){

    QString str = i18n(
		"Cannot access floppy or floppy drive\n"
		"Please insert a floppy and make sure that you\n"
		"have selected a valid floppy drive.\n");

    KMessageBox::error(this, str);
    
    reset();
    return;

  }

  QString str = i18n("Cannot format: %1\n%2").arg(device).arg(errstring);

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

#ifdef MY_DEBUG
printf("NEWLINE:%s\n",mystring.data());
#endif

    if(findKeyWord(mystring,"BBF ")){
      int bblock = mystring.left(8).toInt();
      QString mstr;
      mstr = i18n("Block %1 is bad. Continuing ...").arg(bblock);
      frame->setText(mstr);
    }

    if(findKeyWord(mystring,"TNBB ")){
      badblocks = mystring.left(8).toInt();
    }
    newstring = newstring.mid(i+1,newstring.length());
  }

  counter += findKeyWord(fsstring,"BLOCK");
#ifdef MY_DEBUG
printf("Block Counter: %d\n",counter);  
#endif
  if(quickerase){

    if(findKeyWord(fsstring,"START"))
      counter ++;

    if(findKeyWord(fsstring,"EXIT"))
      counter ++;

    
  }

  fsstring = newstring;

  progress->setValue(counter);     
#ifdef MY_DEBUG
  printf("STDOUT:%s\n",mybuffer);
#endif
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
      //printf("pos=%d, len=%d\n", pos, amount);
      if (pos+1 == amount)
	return;
      pos++;
    }
  if (fserrstring.isEmpty() && strncmp(mybuffer, "/sbin/mkdosfs", 13) == 0)
    {
      pos = QString(mybuffer).find('\n');
      //printf("pos=%d, len=%d\n", pos, amount);
      if (pos+1 == amount)
	return;
      pos++;
    }

  printf("ERR: <%s>\n", mybuffer);
  printf("pos=%d, len=%d\n", pos, amount);

  abort = true;
  fserrstring += mybuffer+pos;

  // the timers are put in so that I get all of the error message.
  fserrtimer->start(300,true);
#ifdef MY_DEBUG
  printf("STDERR:%s\n",mybuffer);
#endif
}

void FloppyData::fserrslot(){


  if(fserrstring.contains("No such device")){

    QString str = i18n(
		"Cannot access floppy or floppy drive\n"\
		"Please insert a floppy and make sure that you\n"
 		"have selected a valid floppy drive.\n");

    KMessageBox::sorry(this, str);
    
    reset();
    return;
  }

  reset();

  QString str = i18n("Cannot create a filesystem on: %1\n%2")
	      .arg(device).arg(fserrstring);

  KMessageBox::error(this, str);
}


void FloppyData::cfdone(KProcess*){

    delete proc;
    proc = 0L;
    mytimer->start(10,TRUE);

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

void FloppyData::createfilesystem()
{
  fsstring = "";
  fserrstring = "";

  frame->setText(i18n("Creating Filesystem ..."));

  proc = new KProcess;

  if (filesystemComboBox->currentText() == i18n("Dos")){

        *proc << mkdosfs;
	if(labellabel->isChecked())
	  *proc << "-n" <<lineedit->text();
        if (verifylabel->isChecked()) {
          *proc << "-c";
        }
	*proc << device;
  }
  else{

    *proc << mke2fs;
    *proc << "-q";
    if(labellabel->isChecked())
      *proc << "-L" <<lineedit->text();
    if (verifylabel->isChecked()) {
      *proc << "-c";
    }
    *proc << device;
  }



  connect(proc, SIGNAL(processExited(KProcess *)),this, SLOT(cfdone(KProcess*)));

  connect(proc, SIGNAL(receivedStdout(KProcess *,char*,int)),this, 
	  SLOT(readfsStdout(KProcess*,char*,int)));

  connect(proc, SIGNAL(receivedStderr(KProcess *,char*,int)),this, 
	  SLOT(readfsStderr(KProcess*,char*,int)));

  bool result = proc->start(KProcess::NotifyOnExit , KProcess::All);

  if(!result){
    QString str = i18n("Cannot start a new program\nfork() failed.");
    KMessageBox::error(this, str);
    frame->clear();
    proc = 0L;
    progress->setValue(0);
  }
}

void FloppyData::writeSettings(){

        config = kapp->config();
	config->setGroup("GeneralData");

	densityconfig = densityComboBox->currentText();
	densityconfig = densityconfig.stripWhiteSpace();
	filesystemconfig = filesystemComboBox->currentText();
	filesystemconfig = filesystemconfig.stripWhiteSpace();
	driveconfig = deviceComboBox->currentText();
	driveconfig = driveconfig.stripWhiteSpace();

	if(quick->isChecked())
	  quickformatconfig  = 1;
	else
	  quickformatconfig = 0;

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
	labelnameconfig = config->readEntry("Label","KDE Floppy");
	quickformatconfig = config->readNumEntry("QuickFormat",0);
	driveconfig = config->readEntry("FloppyDrive","A: 3.5");
	densityconfig = config->readEntry("Density",i18n("HD"));
	filesystemconfig = config->readEntry("Filesystem",i18n("Dos"));

}

void FloppyData::setWidgets(){

  if(labelconfig){
    labellabel->setChecked(TRUE);
  }
  else{

    labellabel->setChecked(FALSE);
  }

  if(verifyconfig) {
    verifylabel->setChecked(TRUE);
  }
  else {
    verifylabel->setChecked(FALSE);
  }

  if(quickformatconfig){
    quick->setChecked(TRUE);
    fullformat->setChecked(FALSE);
  }
  else{
    quick->setChecked(FALSE);
    fullformat->setChecked(TRUE);
  }
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


