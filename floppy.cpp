
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

extern KApplication *mykapp;

extern "C" {
int  check_if_mounted(const char *file, int *mount_flags);
}

FloppyData::FloppyData
(
	QWidget* parent,
	const char* name
)
	:
	QWidget( parent, name )
{

        proc = 0L;
	formating = false;
	quickerase = false;
	abort = false;
	counter = 0;
	tracks = 0;
	blocks = 0;

        outerframe = new QGroupBox(this,"box");
	outerframe->setGeometry(5,5,350,360);
	outerframe->setFrameStyle(QFrame::Box|QFrame::Sunken);

        label1 = new QLabel(this);
	label1->setText(i18n("Floppy Drive:"));
	label1->setGeometry( 20, 20, 130, 25 );


	deviceComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	deviceComboBox->setGeometry( 120, 20, 100, 25 );
	deviceComboBox->setAutoResize( FALSE );

        label2 = new QLabel(this);
	label2->setText(i18n("Density:"));
	label2->setGeometry( 20, 55, 130, 25 );


	densityComboBox = new QComboBox( FALSE, this, "ComboBox_1" );
	densityComboBox->setGeometry( 120, 55, 100, 25 );
	densityComboBox->setAutoResize( FALSE );

        label3 = new QLabel(this);
	label3->setText(i18n("File System:"));
	label3->setGeometry( 20,90, 130, 25 );

 	filesystemComboBox = new QComboBox( FALSE, this, "ComboBox_2" );
	filesystemComboBox->setGeometry( 120, 90, 100, 25 );
	filesystemComboBox->setAutoResize( FALSE );
	connect(filesystemComboBox,SIGNAL(activated(int)),
		this,SLOT(filesystemchanged(int)));
	
	quitbutton = new QPushButton( this, "PushButton_1" );
	quitbutton->setGeometry( 240, 40, 100, 25 );
	quitbutton->setText(i18n( "Quit") );
	quitbutton->setAutoRepeat( FALSE );
	quitbutton->setAutoResize( FALSE );
	connect(quitbutton,SIGNAL(clicked()),this,SLOT(quit()));

	aboutbutton = new QPushButton( this, "PushButton_2" );
	aboutbutton->setGeometry( 240, 75, 100, 25 );
	aboutbutton->setText( i18n("About") );
	aboutbutton->setAutoRepeat( FALSE );
	aboutbutton->setAutoResize( FALSE );
	connect(aboutbutton,SIGNAL(clicked()),this,SLOT(about()));

	helpbutton = new QPushButton( this, "PushButton_1" );
	helpbutton->setGeometry( 240, 165, 100, 25 );
	helpbutton->setText(i18n( "Help") );
	helpbutton->setAutoRepeat( FALSE );
	helpbutton->setAutoResize( FALSE );
	connect(helpbutton,SIGNAL(clicked()),this,SLOT(help()));



	formatbutton = new QPushButton( this, "PushButton_3" );
	formatbutton->setGeometry( 240, 200, 100, 25 );
	formatbutton->setText(i18n( "Format") );
	formatbutton->setAutoRepeat( FALSE );
	formatbutton->setAutoResize( FALSE );
	connect(formatbutton,SIGNAL(clicked()),this,SLOT(format()));


	progress = new KProgress( this, "Progress" );
	progress->setGeometry( 20, 325, 320, 30 );
	progress->setFont(QFont("Helvetica",12,QFont::Normal));
	progress->setBarColor(QApplication::winStyleHighlightColor());

	frame = new QLabel( this, "NewsWindow" );
	frame->setGeometry( 20, 275, 320, 38 );
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	frame->setAlignment(AlignCenter|WordBreak|ExpandTabs);
	
	buttongroup = new QButtonGroup( this, "ButtonGroup_1" );
	buttongroup->setGeometry( 20, 125, 200, 135 );
	buttongroup->setFrameStyle( 49 );
	buttongroup->setAlignment( 1 );

	quick = new QRadioButton( buttongroup, "RadioButton_2" );
	quick->setGeometry( 15, 10, 170, 30 );
	quick->setText(i18n( "Quick Erase") );
	quick->setAutoResize( TRUE );

	fullformat = new QRadioButton( buttongroup, "RadioButton_3" );
	fullformat->setGeometry( 15, 40, 170, 30 );
	fullformat->setText(i18n( "Full Format") );
	fullformat->setAutoResize( TRUE );
	fullformat->setChecked(TRUE);

	labellabel = new QCheckBox( this, "RadioButton_4" );
	labellabel->setGeometry( 35, 195, 170, 30 );
	labellabel->setText(i18n( "Create Label:") );
	labellabel->setAutoResize( TRUE );
	labellabel->setChecked(TRUE);

	lineedit = new QLineEdit( this, "Lineedit" );
	lineedit->setGeometry( 35, 225, 170, 25 );
	lineedit->setText(i18n( "KDE FLoppy") );
	lineedit->setMaxLength(10);



	mytimer = new QTimer;
	connect(mytimer,SIGNAL(timeout()),this,SLOT(cf2done()));

	fserrtimer = new QTimer;
	connect(fserrtimer,SIGNAL(timeout()),this,SLOT(fserrslot()));

	errtimer = new QTimer;
	connect(errtimer,SIGNAL(timeout()),this,SLOT(errslot()));

	setCaption(i18n("KDE Floppy Formater"));

	addDevice(FLOPPYA3);
	addDevice(FLOPPYA5);
	addDevice(FLOPPYB3);
	addDevice(FLOPPYB5);
	addDensity(i18n("HD"));	
	addDensity(i18n("DD"));	
	addFileSystem("Dos");
	addFileSystem("ext2fs");

	readSettings();
	setWidgets();

	if(!findExecutables())
	  formatbutton->setEnabled(FALSE);

	resize( 360, 370 );
	setFixedSize(360,370);
}


FloppyData::~FloppyData()
{
}

void FloppyData::closeEvent(QCloseEvent*){

  quit();
   
}

void FloppyData::addDevice(const char* name){


  deviceComboBox->insertItem(name);
}



void FloppyData::addDensity(const char* name){


  densityComboBox->insertItem(name);
}



void FloppyData::addFileSystem(const char* name){

  filesystemComboBox->insertItem(name);

}


bool FloppyData::findDevice()
{


  if((QString) deviceComboBox->currentText() == (QString) FLOPPYA3){
    if((QString) densityComboBox->currentText() == (QString)i18n( "HD")){
      device = "/dev/fd0h1440";
      blocks = 1440;
      tracks = 80;
      mdev = "/dev/fd0";
      if( access(device.data(),W_OK) < 0){
	device = "/dev/fd0H1440";
      }
    }
    else{
      device = "/dev/fd0D720";
      blocks = 720;
      tracks = 80;
      mdev = "/dev/fd0";
    }
  }

  if((QString) deviceComboBox->currentText() == (QString) FLOPPYA5){
    if((QString) densityComboBox->currentText() == (QString)i18n( "HD")){
      device = "/dev/fd0h1200";
      blocks = 1200;
      tracks = 80;
      mdev = "/dev/fd0";
    }
    else{
      device = "/dev/fd0h360";
      blocks = 720;
      tracks = 40;
      mdev = "/dev/fd0";
    }
  }

  if((QString) deviceComboBox->currentText() == (QString) FLOPPYB3){
    if((QString) densityComboBox->currentText() == (QString)i18n( "HD")){
      device = "/dev/fd1h1440";
      blocks = 1400;
      tracks = 80;
      mdev = "/dev/fd1";
      if( access(device.data(),W_OK) < 0){
	device = "/dev/fd1H1440";
      }
    }
    else{
      device = "/dev/fd1D720";
      blocks = 720;
      tracks = 80;
      mdev = "/dev/fd1";
    }
  }

  if((QString) deviceComboBox->currentText() == (QString) FLOPPYB5){
    if((QString) densityComboBox->currentText() == (QString)i18n( "HD")){
      device = "/dev/fd1h1200";
      blocks = 1200;
      tracks = 80;
      mdev = "/dev/fd1";
    }
    else{
      device = "/dev/fd1h720";
      blocks = 720;
      tracks = 80;
      mdev = "/dev/fd1";
    }
  }


  if( access(device.data(),W_OK) < 0){

    QString str;
    str.sprintf(i18n(
	      "Cannot access %s\nMake sure that the device exists and that\n"
	      "you have write permission to it."),device.data());
    QMessageBox::critical(this,
		      i18n( "KFloppy"),
		       str.data());

    formatbutton->setEnabled(FALSE);
    return false;

  }
  
  return true;
}

bool FloppyData::findExecutables(){


  bool mkformat = false;
  bool mkdosfs  = false;
  bool mke2fs   = false;
  bool ok 	= true;
  
  QFileInfo info;
  QString directory;
 
  directory = mykapp->kde_bindir();

  info.setFile(directory + "/kfdformat");
  if (info.isExecutable()){
    mkformat = true;
  }

  info.setFile(directory + "/kmke2fs");
  if (info.isExecutable()){
    mke2fs = true;
  }

  info.setFile(directory + "/kmkdosfs");
  if (info.isExecutable()){
    mkdosfs = true;
  }

  if(!mkformat){

    QString str;
    str.sprintf(
		i18n(
       	"Cannot find kfdformat\nkfdformat is part of the KFloppy distribution.\n"
	"Please install KFloppy properly."));
    QMessageBox::critical(this,
			  i18n("KFloppy"),
		       str.data());

    formatbutton->setEnabled(FALSE);
    ok = false;

  }

  if( !mke2fs){

    QString str;
    str.sprintf(
		i18n(
          "Cannot find kmke2fs\nkmke2fs is part of the KFloppy distribution.\n"
	  "Please install KFloppy properly."));
    QMessageBox::critical(this,
		       i18n("KFloppy"),
		       str.data());

    formatbutton->setEnabled(FALSE);
    ok = false;
  }

  if( !mkdosfs){

    QString str;
    str.sprintf(
		i18n(
		"Cannot find kmkdosfs\nkmkdosfs is part of the KFloppy distribution.\n"
		"Please install KFloppy properly.")
		);
    QMessageBox::critical(this,
		       i18n("KFloppy"),
		       str.data());

    formatbutton->setEnabled(FALSE);
    ok = false;

  }


  return ok;

}

void FloppyData::quit(){

  if(proc){
    if(proc->isRunning())
      proc->kill();
    
  }
  writeSettings();
  mykapp->quit();



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
  frame->setText("");
  formatbutton->setText(i18n("Format"));

}

void FloppyData::format(){

  errstring = "";
  formatstring ="";
  mdev = "";

  if(formating){
    abort = true;
    reset();
    return;
  }

  formatbutton->setText(i18n("Abort"));

  if(!findDevice()){
    reset();
    return;
  }

  if(!checkmount()){
    reset();
    return;
  }


  if(quick->isChecked()){
    quickerase = true;
    formating = false;
    createfilesystem();
    return;
  }


  badblocks = 0;
  abort = false;
  formating = true;
  progress->setRange(0,tracks);
  progress->setValue(0);
  counter = 0;

  proc = new KProcess;

  proc->setExecutable("kfdformat");
  *proc << "-n" << device.data();


  connect(proc, SIGNAL(processExited(KProcess *)),this, SLOT(formatdone(KProcess*)));

  connect(proc, SIGNAL(receivedStdout(KProcess *,char*,int)),this, 
	  SLOT(readStdout(KProcess*,char*,int)));

  connect(proc, SIGNAL(receivedStderr(KProcess *,char*,int)),this, 
	  SLOT(readStderr(KProcess*,char*,int)));

  bool result = proc->start(KProcess::NotifyOnExit , KProcess::All);

  if(!result){
    QString str;
    str.sprintf(
		i18n(
		"Cannot start a new program: fork() failed."
		));
    QMessageBox::critical(this,
		       i18n("KFloppy"),
		       str.data());
    reset();
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

void FloppyData::readStdout(KProcess *, char *buffer, int buflen){

  char mybuffer[1001];
  int amount;
  bool  increment = true;

  if(buflen > 1000)
    amount = 1000;
  else
    amount = buflen;

  memcpy(mybuffer,buffer,amount);
  mybuffer[amount] = '\0';

  formatstring = mybuffer;


  if(formatstring.contains("track")){
    int pos = formatstring.find('\n');
    QString newstring;

    if(pos != -1)
      newstring = formatstring.left(pos);
    else
      newstring = formatstring;

    frame->setText(newstring.data());
    increment =false;
  }


  if(increment){
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

    QString str;
    str.sprintf(
		i18n(
		"Cannot access floppy or floppy drive\n"\
		"Please insert a floppy and make sure that you\n"
		"have selected a valid floppy drive.\n")
		);

    QMessageBox::warning(this,
		       i18n("KFloppy"),
		       str.data());
    
    reset();
    return;

  }

  QString str;
  str.sprintf(i18n("Cannot format: "\
	      "%s\n"\
	      "%s"),device.data(),errstring.data());

  QMessageBox::warning(this,
		   i18n(    "KFloppy"),
		       str.data());


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
  newstring.detach();

  int i;

  while( (i =newstring.find('\n')) != -1){

    QString mystring;
    mystring = newstring.left(i);

#ifdef MY_DEBUG
printf("NEWLINE:%s\n",mystring.data());
#endif

    if(findKeyWord(mystring,"BBF ")){
      int bblock = atoi(mystring.left(8).data());
      QString mstr;
      mstr.sprintf(i18n("Block %d is bad. Continuing ..."),bblock);
      frame->setText(mstr.data());
    }

    if(findKeyWord(mystring,"TNBB ")){
      badblocks = atoi(mystring.left(8).data());
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

  abort = true;
  fserrstring += mybuffer;

  // the timers are put in so that I get all of the error message.
  fserrtimer->start(300,true);
#ifdef MY_DEBUG
  printf("STDERR:%s\n",mybuffer);
#endif
}

void FloppyData::fserrslot(){


  if(fserrstring.contains("No such device")){

    QString str;
    str.sprintf(
		i18n(
		"Cannot access floppy or floppy drive\n"\
		"Please insert a floppy and make sure that you\n"
 		"have selected a valid floppy drive.\n"
		)
		);

    QMessageBox::warning(this,
		       i18n("KFloppy"),
		       str.data());
    
    reset();
    return;
  }

  reset();

  QString str;
  str.sprintf(i18n("Cannot create a filesystem on: %s\n%s")
	      ,device.data(),fserrstring.data());

  QMessageBox::warning(this,
		       i18n("KFloppy"),
		       str.data());

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
    QString str;

    str.sprintf(
		i18n(
		"The floppy was sucessfully formatted.\n"\
		"Blocks marked bad: %d\n"\
		"Raw Capacity: %d\n"),
		badblocks,
		(blocks - badblocks)*1024
		);

    /*    str.sprintf("The floppy was sucessfully formatted.\n"\
		"Raw Capacity: %d\n"
		"Blocks marked bad: %d\n"\
		"Free Space: %d",blocks*1024,badblocks,
		(blocks - badblocks) * 1024);
		*/

    QMessageBox::information(this,
			     i18n("KFloppy"),
			     str.data());
  }
  else{

    QString str;
    str.sprintf(i18n("All files were sucessfully erased."));

    QMessageBox::information(this,
			     i18n("KFloppy"),
			     str.data());

  }

}


int FloppyData::findKeyWord(QString& string,char* word){

  int count = 0;
  int index = 0;
  int len = 0;

  QString wordstring = word;
  len = wordstring.length();
    
  while( (index = string.find(word)) >= 0){

    count ++;
    string = string.mid(index + len,string.length());

  }

  return count;

}


void FloppyData::createfilesystem(){

  fsstring = "";
  fserrstring = "";

  if(quickerase){
    if(!findDevice())
      return;
    quickerase = true;
  }

  if(quickerase)
    progress->setRange(0,3);
  else
    progress->setRange(0,blocks/16);


  if(quickerase)
    counter = 1;	
  else
    counter = 0;

  progress->setValue(counter);

  if(quickerase){

    frame->setText(i18n("Creating Filesystem ..."));
  }
  else{

    frame->setText(i18n("Verifying ..."));

  }

  proc = new KProcess;

  if((QString)filesystemComboBox->currentText() == "Dos"){

    proc->setExecutable("kmkdosfs");
    if(!quickerase)
      
      *proc << "-c" ;

    if(labellabel->isChecked())
      *proc << "-n" <<lineedit->text();

    *proc << device.data();
  }
  else{

    proc->setExecutable("kmke2fs");

    if(!quickerase)
      *proc << "-c" ;

    *proc << device.data();

  }



  connect(proc, SIGNAL(processExited(KProcess *)),this, SLOT(cfdone(KProcess*)));

  connect(proc, SIGNAL(receivedStdout(KProcess *,char*,int)),this, 
	  SLOT(readfsStdout(KProcess*,char*,int)));

  connect(proc, SIGNAL(receivedStderr(KProcess *,char*,int)),this, 
	  SLOT(readfsStderr(KProcess*,char*,int)));

  bool result = proc->start(KProcess::NotifyOnExit , KProcess::All);

  if(!result){
    QString str;
    str.sprintf(
		i18n(
		"Cannot start a new program\nfork() failed.")
		);
    QMessageBox::critical(this,
		       i18n("KFloppy"),
		       str.data());
    frame->setText("");
    proc = 0L;
    progress->setValue(0);
  }


}



bool FloppyData::checkmount(){

  int		retval;
  int		mount_flags;


  retval = check_if_mounted(mdev.data(), &mount_flags);
  if (retval) {
    QMessageBox::warning(this,
		      i18n("KFloppy"),
		      i18n(
		      "Error while trying to determine whether floppy is mounted\n"
		      )
			 );
    return false;
  }


  if (!(mount_flags & EXT2_MF_MOUNTED)){
   return true;
  }




  QString str;
  str.sprintf(
	      i18n(
	      "%s is mounted.\nPlease unmount the floppy first."
	      )
	      ,mdev.data());

  QMessageBox::warning(this,
		       i18n("KFloppy"),
		       str.data());  


 return false;

}




void FloppyData::about(){

  QDialog *dlg = new MyAbout(0);

  QPoint point = this->mapToGlobal (QPoint (0,0));

  QRect pos = this->geometry();
  dlg->setGeometry(point.x() + pos.width()/2  - dlg->width()/2,
		   point.y() + pos.height()/2 - dlg->height()/2, 
		   dlg->width(),dlg->height());

  dlg->exec();
  delete dlg;


}



void FloppyData::help(){

  mykapp->invokeHTMLHelp("kfloppy/kfloppy.html","");


}


void FloppyData::writeSettings(){

        config = mykapp->getConfig();
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

	config->writeEntry("CreateLabel",labelconfig);
	config->writeEntry("Label",labelnameconfig.data());


	config->writeEntry("QuickFormat",quickformatconfig);
	config->writeEntry("FLoppyDrive",driveconfig.data());
	config->writeEntry("Density",densityconfig.data());
	config->writeEntry("Filesystem",filesystemconfig.data());
	config->sync();

}

void FloppyData::readSettings(){

        config = mykapp->getConfig();
	config->setGroup("GeneralData");

	labelconfig = config->readNumEntry("CreateLabel",1);
	labelnameconfig = config->readEntry("Label","KDE Floppy");
	quickformatconfig = config->readNumEntry("QuickFormat",0);
	driveconfig = config->readEntry("FLoppyDrive","A: 3.5");
	densityconfig = config->readEntry("Density","HD");
	filesystemconfig = config->readEntry("Filesystem","Dos");

}

void FloppyData::setWidgets(){

  if(labelconfig){
    labellabel->setChecked(TRUE);
  }
  else{

    labellabel->setChecked(FALSE);
  }

  if(quickformatconfig){
    quick->setChecked(TRUE);
    fullformat->setChecked(FALSE);
  }
  else{
    quick->setChecked(FALSE);
    fullformat->setChecked(TRUE);
  }
  lineedit->setText(labelnameconfig.data());

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

  if((QString) filesystemComboBox->currentText() == (QString) "ext2fs"){
    labellabel->setEnabled(FALSE);
    lineedit->setEnabled(FALSE);
  }

}

void  FloppyData::filesystemchanged(int index){


  if ((QString)filesystemComboBox->text(index) == (QString)"ext2fs"){

    labellabel->setEnabled(FALSE);
    lineedit->setEnabled(FALSE);

  }
  else{

    labellabel->setEnabled(TRUE);
    lineedit->setEnabled(TRUE);

  }


}
