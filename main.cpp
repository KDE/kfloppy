/*

    This file is part of the KFloppy program, part of the KDE project

    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    Copyright (C) 2004, 2005 Nicolas GOUTTE <goutte@kde.org>
    Copyright (C) 2015, 2016 Wolfgang Bauer <wbauer@tmo.at>


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

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include "floppy.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Not needed in Qt6 (and doesn't exist at all)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setFallbackSessionManagementEnabled(false);

    Kdelibs4ConfigMigrator migrator(QStringLiteral("kfloppy"));
    migrator.setConfigFiles(QStringList() << QStringLiteral("kfloppyrc"));
    migrator.migrate();
#endif

    KLocalizedString::setApplicationDomain("kfloppy");
    KAboutData aboutData(QStringLiteral("kfloppy"),
                         i18n("KFloppy"),
                         QStringLiteral(KFLOPPY_VERSION),
                         i18n("KDE Floppy Disk Utility"),
                         KAboutLicense::GPL,
                         i18n("(c) 1997, Bernd Johannes Wuebben\n"
                              "(c) 2001, Chris Howells\n"
                              "(c) 2002, Adriaan de Groot\n"
                              "(c) 2004, 2005, Nicolas Goutte\n"
                              "(c) 2015, 2016, Wolfgang Bauer"),
                         i18n("KFloppy helps you format floppies with the filesystem of your choice."),
                         QStringLiteral("https://utils.kde.org/projects/kfloppy"));

    aboutData.addAuthor(i18n("Bernd Johannes Wuebben"), i18n("Author and former maintainer"), QStringLiteral("wuebben@kde.org"));
    aboutData.addCredit(i18n("Chris Howells"), i18n("User interface re-design"), QStringLiteral("howells@kde.org"));
    aboutData.addCredit(i18n("Adriaan de Groot"), i18n("Add BSD support"), QStringLiteral("groot@kde.org"));
    aboutData.addCredit(i18n("Nicolas Goutte"), i18n("Make KFloppy work again for KDE 3.4"), QStringLiteral("goutte@kde.org"));
    aboutData.addCredit(i18n("Wolfgang Bauer"), i18n("Port KFloppy to KF5"), QStringLiteral("wbauer@tmo.at"));
    // necessary to make the "Translators" tab appear in the About dialog
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KAboutData::setApplicationData(aboutData);

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kfloppy")));

    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("[device]"), i18n("Default device"));
    parser.process(app);
    aboutData.processCommandLine(&parser);

    const QStringList args = parser.positionalArguments();
    QString device;
    if (args.count()) {
        device = args.at(0);
    }

    auto floppy = new FloppyData();
    bool autoformat = floppy->setInitialDevice(device);
    floppy->show();
    if (autoformat) {
        floppy->format();
    }
    return app.exec();
}
