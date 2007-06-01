/*
    
    This file is part of the KFloppy program, part of the KDE project
    
    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    Copyright (C) 2004, 2005 Nicolas GOUTTE <goutte@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qwhatsthis.h>

#include <kconfig.h>

#include <kmessagebox.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kpushbutton.h>
#include <kpopupmenu.h>
#include <kapplication.h>
#include <kprogress.h>
#include <klocale.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <dcopref.h>
#include <kurl.h>

#include "floppy.h"
#include "format.h"

FloppyData::FloppyData(QWidget * parent, const char * name)
 : KDialog( parent, name ),
	formatActions(0L), m_canLowLevel(false), m_canZeroOut( false )
{

	formating = false;
	//abort = false;
	blocks = 0;

        QVBoxLayout* ml = new QVBoxLayout( this, 10 );

        QHBoxLayout* h1 = new QHBoxLayout( ml );

        QVBoxLayout* v1 = new QVBoxLayout( h1 );
        h1->addSpacing( 5 );

        QGridLayout* g1 = new QGridLayout( v1, 3, 2 );

        deviceComboBox = new KComboBox( false, this, "ComboBox_1" );
        label1 = new QLabel( deviceComboBox, i18n("Floppy &drive:"), this );
        g1->addWidget( label1, 0, 0, AlignLeft );
        g1->addWidget( deviceComboBox, 0, 1 );

        // Make the combo box editable, so that the user can enter a device name
        deviceComboBox->setEditable( true );
	
        deviceComboBox->insertItem(i18n("Primary"));
	deviceComboBox->insertItem(i18n("Secondary"));

	const QString deviceWhatsThis = i18n("<qt>Select the floppy drive.</qt>");
	
	QWhatsThis::add(label1, deviceWhatsThis);
	QWhatsThis::add(deviceComboBox, deviceWhatsThis);

        
        densityComboBox = new KComboBox( false, this, "ComboBox_1" );
        label2 = new QLabel( densityComboBox, i18n("&Size:"), this);
        g1->addWidget( label2, 1, 0, AlignLeft );
        g1->addWidget( densityComboBox, 1, 1 );

#if defined(ANY_LINUX)
	densityComboBox->insertItem( i18n( "Auto-Detect" ) );
#endif
	densityComboBox->insertItem(i18n("3.5\" 1.44MB"));
	densityComboBox->insertItem(i18n("3.5\" 720KB"));
	densityComboBox->insertItem(i18n("5.25\" 1.2MB"));
	densityComboBox->insertItem(i18n("5.25\" 360KB"));

	const QString densityWhatsThis = 
	    i18n("<qt>This allows you to select the "
	         "floppy disk's size and density.</qt>");
	
	QWhatsThis::add(label2, densityWhatsThis);
	QWhatsThis::add(densityComboBox, densityWhatsThis);


        filesystemComboBox = new KComboBox( false, this, "ComboBox_2" );
        label3 = new QLabel( filesystemComboBox, i18n("F&ile system:"), this);
        g1->addWidget( label3, 2, 0, AlignLeft );
        g1->addWidget( filesystemComboBox, 2, 1 );
	g1->setColStretch(1, 1);

#if defined(ANY_LINUX)
        QWhatsThis::add( label3,
            i18n( "Linux", "KFloppy supports three file formats under Linux: MS-DOS, Ext2, and Minix" ) );
#elif defined(ANY_BSD)
        QWhatsThis::add( label3,
            i18n( "BSD", "KFloppy supports three file formats under BSD: MS-DOS, UFS, and Ext2" ) );
#endif
        // If you modify the user visible string, change them also (far) below

        QString userFeedBack;
        uint numFileSystems = 0;

#if defined(ANY_LINUX)
        QWhatsThis::add( filesystemComboBox,
            i18n( "Linux", "KFloppy supports three file formats under Linux: MS-DOS, Ext2, and Minix" ) );
        if (FATFilesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("DOS"));
            ++numFileSystems;
            userFeedBack += i18n( "Linux", "Program mkdosfs found." );
        }
        else {
            userFeedBack += i18n( "Linux", "Program mkdosfs <b>not found</b>. MSDOS formatting <b>not available</b>." );
        }
        userFeedBack += "<br>";
        if (Ext2Filesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("ext2"));
            ++numFileSystems;
            userFeedBack += i18n( "Program mke2fs found." );
        }
        else {
            userFeedBack += i18n( "Program mke2fs <b>not found</b>. Ext2 formatting <b>not available</b>" );
        }
        userFeedBack += "<br>";
        if (MinixFilesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("Minix"));
            ++numFileSystems;
            userFeedBack += i18n( "Linux", "Program mkfs.minix found." );
        }
        else {
            userFeedBack += i18n( "Linux", "Program mkfs.minix <b>not found</b>. Minix formatting <b>not available</b>" );
        }
#elif defined(ANY_BSD)
        QWhatsThis::add( filesystemComboBox,
            i18n( "BSD", "KFloppy supports two file formats under BSD: MS-DOS and UFS" ) );
        if (FATFilesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("DOS"));
            ++numFileSystems;
            userFeedBack += i18n( "BSD", "Program newfs_msdos found." );
        }
        else {
            userFeedBack += i18n( "BSD", "Program newfs_msdos <b>not found</b>. MSDOS formatting <b>not available</b>." );
        }
        userFeedBack += "<br>";
        if (UFSFilesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("UFS"));
            ++numFileSystems;
            userFeedBack += i18n( "BSD", "Program newfs found." );
        }
        else {
            userFeedBack += i18n( "BSD", "Program newfs <b>not found</b>. UFS formatting <b>not available</b>." );
        }
        userFeedBack += "<br>";
        if (Ext2Filesystem::runtimeCheck()) {
            filesystemComboBox->insertItem(i18n("ext2"));
            ++numFileSystems;
            userFeedBack += i18n( "Program mke2fs found." );
        }
        else {
            userFeedBack += i18n( "Program mke2fs <b>not found</b>. Ext2 formatting <b>not available</b>" );
        }
#endif

        v1->addSpacing( 10 );

        buttongroup = new QButtonGroup( 3, Qt::Vertical, i18n("&Formatting"), this, "ButtonGroup_1" );


        quick = new QRadioButton( i18n( "Q&uick format" ), buttongroup, "RadioButton_2" );
        QWhatsThis::add( quick,
            i18n("<qt>Quick format is only a high-level format:"
                " it creates only a file system.</qt>") );

	zerooutformat = new QRadioButton( i18n( "&Zero out and quick format"), buttongroup, "RadioButton_ZeroOutFormat" );
        QWhatsThis::add( zerooutformat,
            i18n("<qt>This first erases the floppy by writing zeros and then it creates the file system.</qt>") );
	
        fullformat = new QRadioButton( i18n( "Fu&ll format"), buttongroup, "RadioButton_3" );
        QWhatsThis::add( fullformat,
            i18n("Full format is a low-level and high-level format. It erases everything on the disk.") );

        v1->addWidget( buttongroup );

        // ### TODO: we need some user feedback telling why full formatting is disabled.
        userFeedBack += "<br>";
        m_canLowLevel = FDFormat::runtimeCheck();
        if (m_canLowLevel){
            fullformat->setChecked(true);
            userFeedBack += i18n( "Program fdformat found." );
        }
        else {
            fullformat->setDisabled(true);
            quick->setChecked(true);
            userFeedBack += i18n( "Program fdformat <b>not found</b>. Full formatting <b>disabled</b>." );
        }
        userFeedBack += "<br>";
        m_canZeroOut = DDZeroOut::runtimeCheck();
        if ( m_canZeroOut )
        {
            zerooutformat->setChecked( true );
            userFeedBack += i18n( "Program dd found." );
        }
        else {
            zerooutformat->setDisabled(true);
            userFeedBack += i18n( "Program dd <b>not found</b>. Zeroing-out <b>disabled</b>." );
        }
        
	verifylabel = new QCheckBox( this, "CheckBox_Integrity" );
	verifylabel->setText(i18n( "&Verify integrity" ));
	verifylabel->setChecked(true);
	v1->addWidget( verifylabel, AlignLeft );
        QWhatsThis::add( verifylabel,
            i18n("<qt>Check this if you want the floppy disk to be checked after formatting."
            " Please note that the floppy will be checked twice if you have selected full formatting.</qt>") );

	labellabel = new QCheckBox( this, "Checkbox_Label" );
	labellabel->setText(i18n( "Volume la&bel:") );
	labellabel->setChecked(true);
        v1->addWidget( labellabel, AlignLeft );
        QWhatsThis::add( labellabel,
            i18n("<qt>Check this if you want a volume label for your floppy."
            " Please note that Minix does not support labels at all.</qt>") );

        QHBoxLayout* h2 = new QHBoxLayout( v1 );
        h2->addSpacing( 20 );

	lineedit = new KLineEdit( this, "Lineedit" );
        // ### TODO ext2 supports 16 characters. Minix has not any label. UFS?
	lineedit->setText(i18n( "Volume label, maximal 11 characters", "KDE Floppy" ) );
	lineedit->setMaxLength(11);
        h2->addWidget( lineedit, AlignRight );
        QWhatsThis::add( lineedit,
            i18n("<qt>This is for the volume label."
            " Due to a limitation of MS-DOS the label can only be 11 characters long."
            " Please note that Minix does not support labels, whatever you enter here.</qt>") );

	connect(labellabel,SIGNAL(toggled(bool)),lineedit,SLOT(setEnabled(bool)));

	QVBoxLayout* v3 = new QVBoxLayout( h1 );

	formatbutton = new KPushButton( this, "PushButton_3" );
	formatbutton->setText(i18n( "&Format") );
	formatbutton->setAutoRepeat( false );
        if (!numFileSystems)
            formatbutton->setDisabled(false); // We have not any helper program for creating any file system
	connect(formatbutton,SIGNAL(clicked()),this,SLOT(format()));
        v3->addWidget( formatbutton );
        QWhatsThis::add( formatbutton,
            i18n("<qt>Click here to start formatting.</qt>") );

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
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	frame->setAlignment(WordBreak|ExpandTabs);
        QWhatsThis::add( frame,
            i18n("<qt>This is the status window, where error messages are displayed.</qt>") );

        QString frameText( userFeedBack );
        frameText.prepend( "<qt>" );
        frameText.append( "</qt>" );
        frame->setText( frameText );
        
        ml->addWidget( frame );

	progress = new KProgress( this, "Progress" );
        progress->setDisabled( true );
        ml->addWidget( progress );

	QWhatsThis::add(progress,
			i18n("<qt>Shows progress of the format.</qt>"));

	readSettings();
	setWidgets();

    if (!numFileSystems) {
        QString errorMessage;
        errorMessage += "<qt>";
        errorMessage += i18n("KFloppy cannot find any of the needed programs for creating file systems; please check your installation.<br><br>Log:");
        errorMessage += "<br>";
        errorMessage += userFeedBack;
        errorMessage += "</qt>";
        KMessageBox::error( this, errorMessage );
    }
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
    // Note: this function does not handle user-given devices

  drive=-1;
  if( deviceComboBox->currentText() == i18n("Primary") )
  {
    drive=0;
  }
  else if( deviceComboBox->currentText() == i18n("Secondary") )
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
#if defined(ANY_LINUX)
    else { // For Linux, anything else is Auto
        blocks = 0;
    }
#endif

  return true;
}

bool FloppyData::setInitialDevice(const QString& dev)
{

  QString newDevice = dev;

  KURL url( newDevice );
  if( url.isValid() && ( url.protocol() == "media" || url.protocol() == "system" ) ) {
    QString name = url.fileName();

    DCOPRef mediamanager( "kded", "mediamanager" );
    DCOPReply reply = mediamanager.call("properties(QString)", name);
    if (!reply.isValid()) {
      kdError() << "Invalid reply from mediamanager" << endl;
    } else {
      QStringList properties = reply;
      newDevice = properties[5];
    }
  }

  int drive = -1;
  if ( newDevice.startsWith("/dev/fd0") )
    drive = 0;
  if ( newDevice.startsWith("/dev/fd1"))
    drive = 1;

  // ### TODO user given devices
    
  bool ok = (drive>=0);
  if (ok)
    deviceComboBox->setCurrentItem(drive);
  return ok;
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
  fullformat->setEnabled(b && m_canLowLevel);
  zerooutformat->setEnabled(b && m_canZeroOut);
  verifylabel->setEnabled(b);
  labellabel->setEnabled(b);
  lineedit->setEnabled(b && labellabel->isChecked() );
  helpbutton->setEnabled(b);
  quitbutton->setEnabled(b);
  formatbutton->setEnabled(b);
  progress->setDisabled( b ); // The other way round!
}

void FloppyData::reset()
{
  DEBUGSETUP;

  formating = false;

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

  if(formating){
    //abort = true;
    reset();
    return;
  }

  frame->clear();

    const QString currentComboBoxDevice (  deviceComboBox->currentText() );
    const bool userDevice = ( currentComboBoxDevice.startsWith ("/dev/") );

#ifdef ANY_BSD
    if ( userDevice && filesystemComboBox->currentText() != i18n("UFS"))
    {
        KMessageBox::error( this, i18n("BSD", "Formatting with BSD on a user-given device is only possible with UFS") );
        return;
    }
    // no "else" !
#endif    
    if ( userDevice && ( quick->isChecked() || zerooutformat->isChecked() ) ) 
    {
        if (KMessageBox::warningContinueCancel( this,
            i18n("<qt>Formatting will erase all data on the device:<br/><b>%1</b><br/>"
                "(Please check the correctness of the device name.)<br/>"
                "Are you sure you wish to proceed?</qt>").arg( currentComboBoxDevice )
                , i18n("Proceed?") ) != KMessageBox::Continue)
            {
                return;
            }
    }
    else if ( userDevice )
    {
        // The user has selected full formatting on a user-given device. That is not supported yet!
        KMessageBox::error( this, "Full formatting of a user-given device is not possible!" );
        return;
    }
    else
    {
        if (KMessageBox::warningContinueCancel( this,
            i18n("Formatting will erase all data on the disk.\n"
            "Are you sure you wish to proceed?"), i18n("Proceed?") ) !=
            KMessageBox::Continue)
        {
            return;
        }
    }

  // formatbutton->setText(i18n("A&bort"));
  setEnabled(false);

        // Erase text box
        frame->setText( QString::null );

    if ( !userDevice )
    {
        if ( !findDevice() )
	{
		reset();
		return;
	}
    }

	if (formatActions) delete formatActions;
	formatActions = new KFActionQueue(this);

	connect(formatActions,SIGNAL(status(const QString &,int)),
		this,SLOT(formatStatus(const QString &,int)));
	connect(formatActions,SIGNAL(done(KFAction *,bool)),
		this,SLOT(reset()));

	if ( quick->isChecked())
	{
		formating=false;
		// No fdformat to push
	}
        else if ( zerooutformat->isChecked() )
        {
            DDZeroOut* f = new DDZeroOut( this );
            if ( userDevice )
            {
                f->configureDevice( currentComboBoxDevice );
            }
            else
            {
                f->configureDevice( drive, blocks );
            }
            formatActions->queue(f);
        }
	else if ( userDevice )
        {
            // We should not have got here, assume quick format
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

	if ( filesystemComboBox->currentText() == i18n("DOS") )
	{
		FATFilesystem *f = new FATFilesystem(this);
		f->configure(verifylabel->isChecked(),
			labellabel->isChecked(),
			lineedit->text());
                if ( userDevice )
                {
                    f->configureDevice( currentComboBoxDevice );
                }
                else
                {
                    f->configureDevice(drive,blocks);
                }
		formatActions->queue(f);
	}

	else if ( filesystemComboBox->currentText() == i18n("ext2") )
	{
		Ext2Filesystem *f = new Ext2Filesystem(this);
		f->configure(verifylabel->isChecked(),
			labellabel->isChecked(),
			lineedit->text());
                if ( userDevice )
                {
                    f->configureDevice( currentComboBoxDevice );
                }
                else
                {
                    f->configureDevice(drive,blocks);
                }
                formatActions->queue(f);
	}

#ifdef ANY_BSD
	else if ( filesystemComboBox->currentText() == i18n("UFS") )
	{
		FloppyAction *f = new UFSFilesystem(this);
                f->configureDevice(drive,blocks);
                formatActions->queue(f);
	}
#endif

#ifdef ANY_LINUX
	else if ( filesystemComboBox->currentText() == i18n("Minix") )
	{
		MinixFilesystem *f = new MinixFilesystem(this);
		f->configure(verifylabel->isChecked(),
			labellabel->isChecked(),
			lineedit->text());
                if ( userDevice )
                {
                    f->configureDevice( currentComboBoxDevice );
                }
                else
                {
                    f->configureDevice(drive,blocks);
                }
		formatActions->queue(f);
	}
#endif



	formatActions->exec();
}

void FloppyData::formatStatus(const QString &s,int p)
{
    kdDebug(2002) << "FloppyData::formatStatus: " << s << " : "  << p << endl;
	if (!s.isEmpty())
        {
            const QString oldText ( frame->text() );
            if ( oldText.isEmpty() )
            {
                frame->setText( s );
            }
            else
            {
                frame->setText( oldText + '\n' + s );
            }
        }

	if ((0<=p) && (p<=100))
		progress->setValue(p);
}

void FloppyData::writeSettings(){

        config = kapp->config();
	config->setGroup("GeneralData");

	densityconfig = densityComboBox->currentText().stripWhiteSpace();
	filesystemconfig = filesystemComboBox->currentText().stripWhiteSpace();
	driveconfig = deviceComboBox->currentText().stripWhiteSpace();

	quickformatconfig = quick->isChecked();

	labelnameconfig = lineedit->text().stripWhiteSpace();

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
	labelnameconfig = config->readEntry( "Label", i18n("Volume label, maximal 11 characters", "KDE Floppy") );
	quickformatconfig = config->readNumEntry("QuickFormat",0);
	driveconfig = config->readEntry( "FloppyDrive", i18n("Primary") );
#if defined(ANY_LINUX)
	densityconfig = config->readEntry( "Density", i18n( "Auto-Detect" ) );
#else
	densityconfig = config->readEntry( "Density", i18n("3.5\" 1.44MB") );
#endif
	filesystemconfig = config->readEntry( "Filesystem", i18n("DOS") );

}

void FloppyData::setWidgets(){

  labellabel->setChecked(labelconfig);
  verifylabel->setChecked(verifyconfig);
  quick->setChecked(quickformatconfig || !m_canLowLevel);
  fullformat->setChecked(!quickformatconfig && m_canLowLevel);
  lineedit->setText(labelnameconfig);

  for(int i = 0 ; i < deviceComboBox->count(); i++){
    if ( deviceComboBox->text(i) == driveconfig){
      deviceComboBox->setCurrentItem(i);
    }
  }

  for(int i = 0 ; i < filesystemComboBox->count(); i++){
    if ( filesystemComboBox->text(i) == filesystemconfig){
      filesystemComboBox->setCurrentItem(i);
    }
  }

  for(int i = 0 ; i < densityComboBox->count(); i++){
    if ( densityComboBox->text(i) == densityconfig){
      densityComboBox->setCurrentItem(i);
    }
  }
}

#include "floppy.moc"
