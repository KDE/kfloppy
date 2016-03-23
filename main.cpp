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

#include <Kdelibs4ConfigMigrator>
#include <KAboutData>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "floppy.h"


static const char description[] =
	I18N_NOOP("KDE Floppy Disk Utility");

int main( int argc, char *argv[] )
{
  QApplication app(argc, argv);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  QGuiApplication::setFallbackSessionManagementEnabled(false);
#endif

  Kdelibs4ConfigMigrator migrator(QStringLiteral("kfloppy"));
  migrator.setConfigFiles(QStringList() << QStringLiteral("kfloppyrc"));
  migrator.migrate();

  KLocalizedString::setApplicationDomain("kfloppy");

  KAboutData aboutData(QStringLiteral("kfloppy"),
    i18n("KFloppy"),
    QStringLiteral("5.0"), i18n(description), KAboutLicense::GPL,
    i18n("(c) 1997, Bernd Johannes Wuebben\n"
    "(c) 2001, Chris Howells\n"
    "(c) 2002, Adriaan de Groot\n"
    "(c) 2004, 2005, Nicolas Goutte"),
    i18n("KFloppy helps you format floppies with the filesystem of your choice."),
    QStringLiteral("http://utils.kde.org/projects/kfloppy")
    );

  aboutData.addAuthor(i18n("Bernd Johannes Wuebben"), i18n("Author and former maintainer"), QStringLiteral("wuebben@kde.org"));
  aboutData.addCredit(i18n("Chris Howells"), i18n("User interface re-design"), QStringLiteral("howells@kde.org"));
  aboutData.addCredit(i18n("Adriaan de Groot"), i18n("Add BSD support"), QStringLiteral("groot@kde.org"));
  aboutData.addCredit(i18n("Nicolas Goutte"), i18n("Make KFloppy work again for KDE 3.4"), QStringLiteral("goutte@kde.org"));
  // necessary to make the "Translators" tab appear in the About dialog
  aboutData.setTranslator( i18nc( "NAME OF TRANSLATORS", "Your names" ), i18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );
  KAboutData::setApplicationData(aboutData);
  
  QCommandLineParser parser;
  parser.addVersionOption();
  parser.addHelpOption();
  aboutData.setupCommandLine(&parser);

  parser.addPositionalArgument(QStringLiteral("[device]"), i18n("Default device"));
  parser.process(app);
  aboutData.processCommandLine(&parser);

  const QStringList args = parser.positionalArguments();
  QString device;
  if (args.count()) {
    device = args.at(0);
  }

  FloppyData* floppy  = new FloppyData();
  bool autoformat = floppy->setInitialDevice(device);
  floppy->show();
  if (autoformat)
    floppy->format();
  return app.exec();
}
