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

class FloppyData : public KDialog
{
    Q_OBJECT

public:
    FloppyData(QWidget* parent = 0, const char * name = 0);
    virtual ~FloppyData();

    void addDevice(const QString & name);
    void show();
    void addFileSystem(const QString & name);
    void addDensity(const QString & name);
    bool findDevice();
    void findExecutables();
    void closeEvent(QCloseEvent*);
    void writeSettings();
    void readSettings();
    void setWidgets();
    int findKeyWord(QString &, const QString &);
    
public slots:
      void fserrslot();
      void errslot();
      void quit();
      void format();
      void createfilesystem();
      void reset();

      void formatdone(KProcess* );
      void readStdout(KProcess *proc, char *buffer, int buflen);
      void readStderr(KProcess *proc, char *buffer, int buflen);

      void cfdone(KProcess* );
      void cf2done();
      void readfsStdout(KProcess *proc, char *buffer, int buflen);
      void readfsStderr(KProcess *proc, char *buffer, int buflen);

protected slots:

private:

        QString fdformat, mke2fs, mkdosfs; 

        int verifyconfig;
        int labelconfig;
        QString labelnameconfig;
	int quickformatconfig;
	QString driveconfig;
	QString densityconfig;
	QString filesystemconfig;
	//QString mdev;
	KConfig *config;

	QString formatstring;
	QString fsstring;
	QString fserrstring;
	QString errstring;
	int badblocks;
	int blocks;
	int tracks;
	bool quickerase;
	bool formating;
	bool abort;
        int counter;
        bool ver_notified;
	QString device;
        KProcess*     proc;
        QTimer*	      mytimer;
	QTimer*	      fserrtimer;
	QTimer*	      errtimer;
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

protected:

};

#endif // FloppyData_included
