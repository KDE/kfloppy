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

#ifndef FloppyData_included
#define FloppyData_included

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qprogressbar.h> 
#include <qtimer.h>
#include <qfileinfo.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <stdlib.h>
#include <string.h>

#include <kprogress.h>
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdialog.h>
#include <khelpmenu.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>


class KFAction;
class KFActionQueue;

class FloppyData : public KDialog
{
    Q_OBJECT

public:
    FloppyData(QWidget* parent = 0, const char * name = 0);
    virtual ~FloppyData();

    // Need to overload normal show() in order to mangle caption
    void show();
    // Maps combobox selection to drive and density
    bool findDevice();
    // Startup check for nescessary executables
    void findExecutables();
    // Override closeEvent() in order to properly close
    // the entire application.    
    void closeEvent(QCloseEvent*);
    // Reading and writing the user-visible settings.
    void writeSettings();
    void readSettings();
    // Map stored settings to widget status
    void setWidgets();
    // A kind of QString::find()
    int findKeyWord(QString &, const QString &);
    // Enable/disable all UI elements
    void setEnabled(bool);
    
public slots:
      // void fserrslot();
      // void errslot();

      void quit();
      void format();
      void reset();

      // void formatdone(KProcess* );
      // void readStdout(KProcess *proc, char *buffer, int buflen);
      // void readStderr(KProcess *proc, char *buffer, int buflen);

      // void cfdone(KProcess* );
      // void cf2done();
      // void readfsStdout(KProcess *proc, char *buffer, int buflen);
      // void readfsStderr(KProcess *proc, char *buffer, int buflen);

      void formatStatus(const QString &,int);
      
protected slots:

private:

        // QString fdformat, mke2fs, mkdosfs; 

        int verifyconfig;
        int labelconfig;
        QString labelnameconfig;
	int quickformatconfig;
	QString driveconfig;
	QString densityconfig;
	QString filesystemconfig;
	//QString mdev;
	KConfig *config;

	int drive;
	int blocks;
	
	QString formatstring;
	QString fsstring;
	QString fserrstring;
	QString errstring;
	int badblocks;
	int tracks;
	bool quickerase;
	bool formating;
	bool abort;
        int counter;
        
	// QTimer*	      mytimer;
	// QTimer*	      fserrtimer;
	// QTimer*	      errtimer;
        
	QGroupBox*       outerframe;
        QLabel*       label1;
        QLabel*       label2;        
	QLabel*       label3;
	QButtonGroup* buttongroup;
	QCheckBox*    verifylabel;
	QCheckBox*    labellabel;
	QLineEdit*    lineedit;
	QRadioButton* quick;
	KPushButton* quitbutton;
	KPushButton* helpbutton;
	QRadioButton* fullformat;
	KPushButton*  formatbutton;
	QLabel* frame;
	QComboBox* deviceComboBox;
	QComboBox* filesystemComboBox;
	QComboBox* densityComboBox;
	KProgress* progress;
	KHelpMenu* helpMenu;
	
	KFActionQueue *formatActions;

protected:
	void keyPressEvent(QKeyEvent *e);

};

#endif // FloppyData_included
