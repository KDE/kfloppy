/*
  
    This file is part of the KFloppy program, part of the KDE project
    
    Copyright (C) 2002 by Adriaan de Groot
  
  
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


#ifndef _ZIP_FORMAT_H
#define _ZIP_FORMAT_H

/**
 * \file zip.h
 *
 * This file defines the ZipFormat class, a DiskFormat
 * for KFloppy that deals with Zip disks and UFS under
 * FreeBSD (and probably other BSD's, as well).
 *
 * \todo Add device selector to zip class
 * \note This class is not used by KFloppy
 * \bug This class assumes thatthe Zip disk has 100MB. It does not support 250MB and 750MB
 */

#include "format.h"

class QCheckBox;
class QTimer;
class KProcess;
class KConfig;

class ZipFormat : public DiskFormat
{
Q_OBJECT

public:
	ZipFormat(QWidget *w, const char *n);

	// All the virtuals we need to make
	// a concrete DiskFormat class. See
	// format.h for details.
	//
	virtual FilesystemList FSLabels() const;
	virtual void setEnabled(bool);
	virtual void format(FilesystemData *);
	virtual void quit();
	virtual void readSettings(KConfig *);
	virtual void writeSettings(KConfig *);

	/** Check for dd and newfs, which we
	 * need to do the formatting.
	 */
	static bool runtimeCheck();

protected:
	QCheckBox *zeroWholeDisk;
	QCheckBox *enableSoftUpdates;

	static QString newfs,dd;

	KProcess *p;	///< dd or newfs, doing the real work
	int formatStep;	///< keeps track of what phase we are in

	// Variables for the zeroing phase
	int totalBlocks;
	QTimer *statusTimer;

protected slots:
	/**
         * transition() realises the state machine we use
	 * to handle the different phases of the format:
	 * startup, dd and newfs.
         */
	void transition();

        /**
         * processResult() reads output
	 * from either dd or newfs and interprets it.
         */
	void processResult(KProcess *,char *,int);
         
        /**
         *statusRequest()
	 * sends dd a SIGINFO to get it to print out block counts,
	 * which then triggers processResult() so that we can keep
	 * the progress bar moving.
	 */
	void statusRequest();
};

#endif
