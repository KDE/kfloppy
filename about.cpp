/*

    $Id$
       
    Copyright (C) 1997 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu
		       
    this file adopted from about.cpp written by Mario Weilguni 

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

#include "about.h"
#include "version.h"
#include <qpushbt.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <kapp.h>

extern KApplication *mykapp;

MyAbout::MyAbout(QWidget *parent) : QDialog(parent, "About KFloppy Formater", TRUE) {
  resize(350, 250);
  setFixedSize(size());
  setCaption("About KDE Floppy Formater");

  QString pixdir = mykapp->kdedir() + QString("/share/apps/kfloppy/pics/");  
  QPixmap pm((pixdir + "kfloppylogo.xpm").data());
  QLabel *logo = new QLabel(this);
  logo->setPixmap(pm);
  logo->setGeometry(20, (height()-pm.height())/2 - 25, pm.width(), pm.height());

  QLabel *l;
  l = new QLabel("KFloppy", this);
  l->setFont(QFont("Helvetica", 19, QFont::Bold));
  l->setGeometry(145,40,100,30);

  QString s;
  s = "Version " KFLOPPYVERSION \
  "\n\nCopyright (c) 1997\nBernd Johannes Wuebben\n"\
  "wuebben@kde.org";

  l = new QLabel(s.data(), this);
  l->setGeometry(145, 70, 230, 170);
  l->setAlignment(AlignLeft|WordBreak|ExpandTabs);

  QFrame *f1 = new QFrame(this);
  f1->setLineWidth(1);
  f1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  f1->setGeometry(10,180,330,4);


  QPushButton *b_ok = new QPushButton("Ok", this);
  b_ok->setGeometry(width()/2-40, height() - 48, 80, 32);
  b_ok->setDefault(TRUE);
  b_ok->setAutoDefault(TRUE);
  connect(b_ok, SIGNAL(released()),
	  this, SLOT(accept()));
  b_ok->setFocus();
}

#include "about.moc"
