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

#include <kdeversion.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "floppy.h"


static const char description[] =
	I18N_NOOP("KDE Floppy Disk Utility");

static const KCmdLineOptions options[] =
{
	{ "+[device]", I18N_NOOP("Default device"), 0 },
	KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
  KAboutData aboutData("kfloppy",
	I18N_NOOP("KFloppy"),
    KDE_VERSION_STRING, description, KAboutData::License_GPL,
    "(c) 1997, Bernd Johannes Wuebben\n"
    "(c) 2001, Chris Howells\n"
    "(c) 2002, Adriaan de Groot\n"
    "(c) 2004, 2005, Nicolas Goutte",
    I18N_NOOP("KFloppy helps you format floppies with the filesystem of your choice.")
    );

  aboutData.addAuthor("Bernd Johannes Wuebben", I18N_NOOP("Author and former maintainer"), "wuebben@kde.org");
  aboutData.addCredit("Chris Howells", I18N_NOOP("User interface re-design"), "howells@kde.org");
  aboutData.addCredit("Adriaan de Groot", I18N_NOOP("Add BSD support"), "groot@kde.org");
  aboutData.addCredit("Nicolas Goutte", I18N_NOOP("Make KFloppy work again for KDE 3.4"), "goutte@kde.org");
  
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString device;
  if (args->count()) {
	device = args->arg(0);
  }
  args->clear();

  KApplication a;

  FloppyData* floppy  = new FloppyData();
  a.setMainWidget(floppy);
  bool autoformat = floppy->setInitialDevice(device);
  floppy->show();
  if (autoformat)
	floppy->format();
  return a.exec();
}

