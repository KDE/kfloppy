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

#include <qwidget.h>
#include <qbttngrp.h>
#include <qcombo.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qprogbar.h> 
#include <qtimer.h>
#include <qfileinf.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qmsgbox.h>

#include <stdlib.h>
#include <string.h>

#include <kprogress.h>
#include <kprocess.h>
#include <klocale.h>
#include "about.h"
#include <kapp.h>

#define EXT2_MF_MOUNTED		1
#define EXT2_MF_ISROOT		2
#define EXT2_MF_READONLY	4 

#define FLOPPYA3 "A: 3.5"
#define FLOPPYA5 "A: 5.25"
#define FLOPPYB3 "B: 3.5"
#define FLOPPYB5 "B: 5.25"

class FloppyData : public QWidget
{
    Q_OBJECT

public:

    FloppyData
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~FloppyData();
    void addDevice(char* name);
    void addFileSystem(char* name);
    void addDensity(char* name);
    bool findDevice();
    bool findExecutables();
    void closeEvent(QCloseEvent*);
    void writeSettings();
    void readSettings();
    void setWidgets();
    int findKeyWord(QString&,char*);
    bool checkmount();

    
public slots:
      void fserrslot();
      void errslot();
      void filesystemchanged(int);
      void quit();
      void format();
      void createfilesystem();
      void about();
      void help();
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


        int labelconfig;
        QString labelnameconfig;
	int quickformatconfig;
	QString driveconfig;
	QString densityconfig;
	QString filesystemconfig;
	QString mdev;
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
	QCheckBox*    labellabel;
	QLineEdit*    lineedit;
	QRadioButton* quick;
	QPushButton* aboutbutton;
	QPushButton* quitbutton;
	QPushButton* helpbutton;
	QRadioButton* fullformat;
	QPushButton*  formatbutton;
	QLabel* frame;
	QComboBox* deviceComboBox;
	QComboBox* filesystemComboBox;
	QComboBox* densityComboBox;
	KProgress* progress;

protected:

};

#endif // FloppyData_included
