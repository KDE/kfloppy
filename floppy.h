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

#ifndef FloppyData_included
#define FloppyData_included

#include <qbuttongroup.h>
#include <qcombobox.h>
// #include <qframe.h>
#include <qradiobutton.h>
class QLabel ;
// #include <qdialog.h>
// #include <qprogressbar.h>
#include <qtimer.h>
#include <qfileinfo.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <stdlib.h>
#include <string.h>

#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
// #include <kapplication.h>
// #include <khelpmenu.h>
// #include <kpopupmenu.h>

class QWidgetStack;

class KPushButton;
class KProgress;
class KToggleAction;

class DiskFormat;
class FilesystemData;

#include <qptrlist.h>
#include <kmainwindow.h>

class FloppyData : public KMainWindow
{
    Q_OBJECT

public:
    FloppyData(QWidget* parent = 0, const char * name = 0);
    virtual ~FloppyData();


public slots:
      void quit();
      void format();
      void reset();

protected slots:
	void formatSelected(int);
	void toggleProgress();
	void toggleConfig();
	void showFormats();

private:
        QLabel*       label1;
	KPushButton*  formatbutton;
	KAction *formatAction;
	QLabel* frame;
	QComboBox* filesystemComboBox;
	QWidgetStack *stack;
	KProgress* progress;
	KToggleAction *hideProgress, *hideConfig, *showAllFormats;

	bool formating;

	QPtrList<FilesystemData> availableFilesystems;
	QPtrList<DiskFormat> availableFormats;

protected:
	void addFormat(DiskFormat *);
	void initMenu();
	void writeSettings();
	void readSettings();
	void readSettings2();

	// Handle users clicking the X instead of chosing file->quit.
	virtual void closeEvent(QCloseEvent *);
};

#endif // FloppyData_included


