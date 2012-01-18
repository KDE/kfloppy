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
#include <klocale.h>
#include "floppy.h"


static const char description[] =
	I18N_NOOP("KDE Floppy Disk Utility");

int main( int argc, char *argv[] )
{
  KAboutData aboutData("kfloppy", 0,
	ki18n("KFloppy"),
    "4.9 pre", ki18n(description), KAboutData::License_GPL,
    ki18n("(c) 1997, Bernd Johannes Wuebben\n"
    "(c) 2001, Chris Howells\n"
    "(c) 2002, Adriaan de Groot\n"
    "(c) 2004, 2005, Nicolas Goutte"),
    ki18n("KFloppy helps you format floppies with the filesystem of your choice."),
    "http://utils.kde.org/projects/kfloppy"
    );

  aboutData.addAuthor(ki18n("Bernd Johannes Wuebben"), ki18n("Author and former maintainer"), "wuebben@kde.org");
  aboutData.addCredit(ki18n("Chris Howells"), ki18n("User interface re-design"), "howells@kde.org");
  aboutData.addCredit(ki18n("Adriaan de Groot"), ki18n("Add BSD support"), "groot@kde.org");
  aboutData.addCredit(ki18n("Nicolas Goutte"), ki18n("Make KFloppy work again for KDE 3.4"), "goutte@kde.org");
  
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add("+[device]", ki18n("Default device"));
  KCmdLineArgs::addCmdLineOptions( options );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QString device;
  if (args->count()) {
	device = args->arg(0);
  }
  args->clear();

  KApplication a;

  FloppyData* floppy  = new FloppyData();
  bool autoformat = floppy->setInitialDevice(device);
  floppy->show();
  if (autoformat)
	floppy->format();
  return a.exec();
}
