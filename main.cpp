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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */


#include <qobject.h>
#include <qstring.h>
#include <qdstream.h>
#include <qmsgbox.h>
#include <qstrlist.h> 
#include <qtabdlg.h>
#include <qdir.h>
#include <qfileinf.h>
#include <qstring.h>
#include <qfile.h>
#include <qtstream.h>


#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>


#include <kapp.h>


#include "floppy.h"


KApplication* mykapp;

int main( int argc, char *argv[] ){


  KApplication a( argc, argv, "kfloppy" );
  mykapp = &a;

  FloppyData* floppy  = new FloppyData();
  a.enableSessionManagement(true);
  a.setWmCommand(argv[0]);      
  
  a.setTopWidget(floppy);
  floppy->show();
  a.exec();

  return 0;
}



