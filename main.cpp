    /*

    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "floppy.h"


static const char *description = 
	I18N_NOOP("KDE Floppy Disk utility");

static const char *version = "2.1";

int main( int argc, char *argv[] )
{
  KAboutData aboutData("kfloppy", 
	I18N_NOOP("KFloppy"),
    version, description, KAboutData::License_GPL,
    "(c) 1997, Bernd Johannes Wuebben\n"
    "(c) 2001, Chris Howells\n"
    "(c) 2002, Adriaan de Groot",
    I18N_NOOP("KFloppy helps you format removeable media (like floppy, Zip,\n"
	"or LS120 disks) with the filesystem of your choice.")
    );

  aboutData.addAuthor("Bernd Johannes Wuebben", I18N_NOOP("Author, Maintainer"), "wuebben@kde.org");
  aboutData.addCredit("Chris Howells", I18N_NOOP("User interface re-design"), "howells@kde.org");
  aboutData.addCredit("Adriaan de Groot", I18N_NOOP("Add BSD support"), "groot@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication::addCmdLineOptions();

  KApplication a;

  FloppyData* floppy  = new FloppyData();
  floppy->show();
  return a.exec();
}


