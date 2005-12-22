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

#ifndef FloppyData_included
#define FloppyData_included

#include <kdialog.h>

class QCheckBox;
class QLineEdit;
class QLabel;
class QRadioButton;
class QComboBox;
class QButtonGroup;
class QGroupBox;

class KProgress;
class KConfig;
class KPushButton;
class KHelpMenu;
class KFAction;
class KFActionQueue;

class FloppyData : public KDialog
{
    Q_OBJECT

public:
    FloppyData(QWidget* parent = 0, const char * name = 0);
    virtual ~FloppyData();

    /// Need to overload normal show() in order to mangle caption
    void show();
    /// Maps combobox selection to drive and density
    bool findDevice();
    /// set default device
    bool setInitialDevice(const QString& dev);
    /** Override closeEvent() in order to properly close
      the entire application.*/
    void closeEvent(QCloseEvent*);
    /// Writing the user-visible settings.
    void writeSettings();
    /// Reading the user-visible settings.
    void readSettings();
    /// Map stored settings to widget status
    void setWidgets();
    /// A kind of QString::find()
    int findKeyWord(QString &, const QString &);
    /// Enable/disable all UI elements
    void setEnabled(bool);
    
public slots:
      void quit();
      void format();
      void reset();

      void formatStatus(const QString &,int);
      
protected slots:

private:
        int verifyconfig;
        int labelconfig;
        QString labelnameconfig;
	int quickformatconfig;
	QString driveconfig;
	QString densityconfig;
	QString filesystemconfig;
	KConfig *config;

	int drive;
        /// Number of blocks of the floppy (typically 1440)
	int blocks;
	
	bool formating;
	//bool abort;
        
	QGroupBox*       outerframe;
        QLabel*       label1;
        QLabel*       label2;        
	QLabel*       label3;
	QButtonGroup* buttongroup;
	QCheckBox*    verifylabel;
	QCheckBox*    labellabel;
	QLineEdit*    lineedit;
	QRadioButton* quick;
        QRadioButton* zerooutformat;
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

        bool m_canLowLevel; ///< Low level formatting is possible (i.e. was fdformat found?)
        bool m_canZeroOut; ///< Is zero-out possible (i.e. was dd found?)
protected:
	void keyPressEvent(QKeyEvent *e);

};

#endif // FloppyData_included
