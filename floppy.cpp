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

#include "floppy.h"
#include "format.h"

#include <QApplication>
#include <QCheckBox>
#include <QCursor>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QUrl>
#include <QVBoxLayout>

#include <KAboutData>
#include <KComboBox>
#include <KConfigGroup>
#include <KGuiItem>
#include <KHelpClient>
#include <KHelpMenu>
#include <KLineEdit>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <kmessagebox.h>

FloppyData::FloppyData(QWidget *parent)
    : QDialog(parent)
    , formatActions(nullptr)
    , m_canLowLevel(false)
    , m_canZeroOut(false)
{
    auto widget = new QWidget(this);
    formating = false;
    // abort = false;
    blocks = 0;

    auto ml = new QVBoxLayout(widget);
    ml->setSpacing(10);

    auto h1 = new QHBoxLayout();
    ml->addItem(h1);

    auto v1 = new QVBoxLayout();
    h1->addItem(v1);
    h1->addSpacing(5);

    auto g1 = new QGridLayout();
    v1->addItem(g1);

    deviceComboBox = new KComboBox(false, widget);
    label1 = new QLabel(i18n("Floppy &drive:"), widget);
    label1->setBuddy(deviceComboBox);
    g1->addWidget(label1, 0, 0, Qt::AlignLeft);
    g1->addWidget(deviceComboBox, 0, 1);

    // Make the combo box editable, so that the user can enter a device name
    deviceComboBox->setEditable(true);

    deviceComboBox->addItem(i18nc("Primary floppy drive", "Primary"));
    deviceComboBox->addItem(i18nc("Secondary floppy drive", "Secondary"));

    const QString deviceWhatsThis = i18n("<qt>Select the floppy drive.</qt>");

    label1->setWhatsThis(deviceWhatsThis);
    deviceComboBox->setWhatsThis(deviceWhatsThis);

    densityComboBox = new KComboBox(false, widget);
    label2 = new QLabel(i18n("&Size:"), widget);
    label2->setBuddy(densityComboBox);
    g1->addWidget(label2, 1, 0, Qt::AlignLeft);
    g1->addWidget(densityComboBox, 1, 1);

#if defined(ANY_LINUX)
    densityComboBox->addItem(i18n("Auto-Detect"));
#endif
    densityComboBox->addItem(i18n("3.5\" 1.44MB"));
    densityComboBox->addItem(i18n("3.5\" 720KB"));
    densityComboBox->addItem(i18n("5.25\" 1.2MB"));
    densityComboBox->addItem(i18n("5.25\" 360KB"));

    const QString densityWhatsThis = i18n(
        "<qt>This allows you to select the "
        "floppy disk's size and density.</qt>");

    label2->setWhatsThis(densityWhatsThis);
    densityComboBox->setWhatsThis(densityWhatsThis);

    filesystemComboBox = new KComboBox(false, widget);
    label3 = new QLabel(i18n("F&ile system:"), widget);
    label3->setBuddy(filesystemComboBox);
    g1->addWidget(label3, 2, 0, Qt::AlignLeft);
    g1->addWidget(filesystemComboBox, 2, 1);
    g1->setColumnStretch(1, 1);

#if defined(ANY_LINUX)
    label3->setWhatsThis(i18nc("Linux", "KFloppy supports three file formats under Linux: MS-DOS, Ext2, and Minix"));
#elif defined(ANY_BSD)
    label3->setWhatsThis(i18nc("BSD", "KFloppy supports three file formats under BSD: MS-DOS, UFS, and Ext2"));
#endif
    // If you modify the user visible string, change them also (far) below

    QString userFeedBack;
    uint numFileSystems = 0;

#if defined(ANY_LINUX)
    filesystemComboBox->setWhatsThis(i18nc("Linux", "KFloppy supports three file formats under Linux: MS-DOS, Ext2, and Minix"));
    if (FATFilesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("DOS"));
        ++numFileSystems;
        userFeedBack += i18nc("Linux", "Program mkdosfs found.");
    } else {
        userFeedBack += i18nc("Linux", "Program mkdosfs <b>not found</b>. MSDOS formatting <b>not available</b>.");
    }
    userFeedBack += QLatin1String("<br>");
    if (Ext2Filesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("ext2"));
        ++numFileSystems;
        userFeedBack += i18n("Program mke2fs found.");
    } else {
        userFeedBack += i18n("Program mke2fs <b>not found</b>. Ext2 formatting <b>not available</b>");
    }
    userFeedBack += QLatin1String("<br>");
    if (MinixFilesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("Minix"));
        ++numFileSystems;
        userFeedBack += i18nc("Linux", "Program mkfs.minix found.");
    } else {
        userFeedBack += i18nc("Linux", "Program mkfs.minix <b>not found</b>. Minix formatting <b>not available</b>");
    }
#elif defined(ANY_BSD)
    filesystemComboBox->setWhatsThis(i18nc("BSD", "KFloppy supports two file formats under BSD: MS-DOS and UFS"));
    if (FATFilesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("DOS"));
        ++numFileSystems;
        userFeedBack += i18nc("BSD", "Program newfs_msdos found.");
    } else {
        userFeedBack += i18nc("BSD", "Program newfs_msdos <b>not found</b>. MSDOS formatting <b>not available</b>.");
    }
    userFeedBack += QLatin1String("<br>");
    if (UFSFilesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("UFS"));
        ++numFileSystems;
        userFeedBack += i18nc("BSD", "Program newfs found.");
    } else {
        userFeedBack += i18nc("BSD", "Program newfs <b>not found</b>. UFS formatting <b>not available</b>.");
    }
    userFeedBack += QLatin1String("<br>");
    if (Ext2Filesystem::runtimeCheck()) {
        filesystemComboBox->addItem(i18n("ext2"));
        ++numFileSystems;
        userFeedBack += i18n("Program mke2fs found.");
    } else {
        userFeedBack += i18n("Program mke2fs <b>not found</b>. Ext2 formatting <b>not available</b>");
    }
#endif

    v1->addSpacing(10);

    buttongroup = new QGroupBox(i18n("&Formatting"), this);
    auto buttonGroupLayout = new QVBoxLayout(buttongroup);

    quick = new QRadioButton(i18n("Q&uick format"), buttongroup);
    buttonGroupLayout->addWidget(quick);
    quick->setObjectName(QStringLiteral("RadioButton_2"));
    quick->setWhatsThis(
        i18n("<qt>Quick format is only a high-level format:"
             " it creates only a file system.</qt>"));

    zerooutformat = new QRadioButton(i18n("&Zero out and quick format"), buttongroup);
    buttonGroupLayout->addWidget(zerooutformat);
    zerooutformat->setObjectName(QStringLiteral("RadioButton_ZeroOutFormat"));
    zerooutformat->setWhatsThis(i18n("<qt>This first erases the floppy by writing zeros and then it creates the file system.</qt>"));

    fullformat = new QRadioButton(i18n("Fu&ll format"), buttongroup);
    buttonGroupLayout->addWidget(fullformat);
    fullformat->setObjectName(QStringLiteral("RadioButton_3"));
    fullformat->setWhatsThis(i18n("Full format is a low-level and high-level format. It erases everything on the disk."));

    v1->addWidget(buttongroup);

    // ### TODO: we need some user feedback telling why full formatting is disabled.
    userFeedBack += QLatin1String("<br>");
    m_canLowLevel = FDFormat::runtimeCheck();
    if (m_canLowLevel) {
        fullformat->setChecked(true);
        userFeedBack += i18n("Program fdformat found.");
    } else {
        fullformat->setDisabled(true);
        quick->setChecked(true);
        userFeedBack += i18n("Program fdformat <b>not found</b>. Full formatting <b>disabled</b>.");
    }
    userFeedBack += QLatin1String("<br>");
    m_canZeroOut = DDZeroOut::runtimeCheck();
    if (m_canZeroOut) {
        zerooutformat->setChecked(true);
        userFeedBack += i18n("Program dd found.");
    } else {
        zerooutformat->setDisabled(true);
        userFeedBack += i18n("Program dd <b>not found</b>. Zeroing-out <b>disabled</b>.");
    }

    verifylabel = new QCheckBox(this);
    verifylabel->setObjectName(QStringLiteral("CheckBox_Integrity"));
    verifylabel->setText(i18n("&Verify integrity"));
    verifylabel->setChecked(true);
    v1->addWidget(verifylabel, Qt::AlignLeft);
    verifylabel->setWhatsThis(
        i18n("<qt>Check this if you want the floppy disk to be checked after formatting."
             " Please note that the floppy will be checked twice if you have selected full formatting.</qt>"));

    labellabel = new QCheckBox(this);
    labellabel->setObjectName(QStringLiteral("Checkbox_Label"));
    labellabel->setText(i18n("Volume la&bel:"));
    labellabel->setChecked(true);
    v1->addWidget(labellabel, Qt::AlignLeft);
    labellabel->setWhatsThis(
        i18n("<qt>Check this if you want a volume label for your floppy."
             " Please note that Minix does not support labels at all.</qt>"));

    auto h2 = new QHBoxLayout();
    v1->addItem(h2);
    h2->addSpacing(20);

    lineedit = new KLineEdit(widget);
    // ### TODO ext2 supports 16 characters. Minix has not any label. UFS?
    lineedit->setText(i18nc("Volume label, maximal 11 characters", "KDE Floppy"));
    lineedit->setMaxLength(11);
    h2->addWidget(lineedit, Qt::AlignRight);
    lineedit->setWhatsThis(
        i18n("<qt>This is for the volume label."
             " Due to a limitation of MS-DOS the label can only be 11 characters long."
             " Please note that Minix does not support labels, whatever you enter here.</qt>"));

    connect(labellabel, &QAbstractButton::toggled, lineedit, &QWidget::setEnabled);

    auto v3 = new QVBoxLayout();
    h1->addItem(v3);

    formatbutton = new QPushButton(widget);
    formatbutton->setText(i18n("&Format"));
    formatbutton->setAutoRepeat(false);
    if (!numFileSystems)
        formatbutton->setDisabled(false); // We have not any helper program for creating any file system
    connect(formatbutton, &QAbstractButton::clicked, this, &FloppyData::format);
    v3->addWidget(formatbutton);
    formatbutton->setWhatsThis(i18n("<qt>Click here to start formatting.</qt>"));

    v3->addStretch(1);

    // Setup the Help Menu
    helpMenu = new KHelpMenu(this, KAboutData::applicationData(), false);

    helpbutton = new QPushButton(widget);
    KGuiItem::assign(helpbutton, KStandardGuiItem::help());
    helpbutton->setAutoRepeat(false);
    helpbutton->setMenu(helpMenu->menu());
    v3->addWidget(helpbutton);

    quitbutton = new QPushButton(widget);
    KGuiItem::assign(quitbutton, KStandardGuiItem::quit());
    quitbutton->setAutoRepeat(false);
    connect(quitbutton, &QAbstractButton::clicked, this, &FloppyData::quit);
    v3->addWidget(quitbutton);

    ml->addSpacing(10);

    frame = new QLabel(widget);
    frame->setObjectName(QStringLiteral("NewsWindow"));
    frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    frame->setWordWrap(true);
    frame->setWhatsThis(i18n("<qt>This is the status window, where error messages are displayed.</qt>"));

    QString frameText(userFeedBack);
    frameText.prepend(QLatin1String("<qt>"));
    frameText.append(QLatin1String("</qt>"));
    frame->setText(frameText);

    ml->addWidget(frame);

    progress = new QProgressBar(widget);
    progress->setDisabled(true);
    ml->addWidget(progress);

    progress->setWhatsThis(i18n("<qt>Shows progress of the format.</qt>"));

    setLayout(ml);

    readSettings();
    setWidgets();
    if (!numFileSystems) {
        QString errorMessage;
        errorMessage += QLatin1String("<qt>");
        errorMessage += i18n("KFloppy cannot find any of the needed programs for creating file systems; please check your installation.<br /><br />Log:");
        errorMessage += QLatin1String("<br>");
        errorMessage += userFeedBack;
        errorMessage += QLatin1String("</qt>");
        KMessageBox::error(this, errorMessage);
    }
}

FloppyData::~FloppyData()
{
    delete formatActions;
}

void FloppyData::closeEvent(QCloseEvent *)
{
    quit();
}

void FloppyData::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_F1:
        KHelpClient::invokeHelp();
        break;
    default:
        QDialog::keyPressEvent(e);
        return;
    }
}

void FloppyData::show()
{
    setWindowTitle(i18n("KDE Floppy Formatter"));
    QDialog::show();
}

bool FloppyData::findDevice()
{
    // Note: this function does not handle user-given devices

    drive = -1;
    if (deviceComboBox->currentText() == i18nc("Primary floppy drive", "Primary")) {
        drive = 0;
    } else if (deviceComboBox->currentText() == i18nc("Secondary floppy drive", "Secondary")) {
        drive = 1;
    }

    blocks = -1;

    if (densityComboBox->currentText() == i18n("3.5\" 1.44MB")) {
        blocks = 1440;
    } else if (densityComboBox->currentText() == i18n("3.5\" 720KB")) {
        blocks = 720;
    } else if (densityComboBox->currentText() == i18n("5.25\" 1.2MB")) {
        blocks = 1200;
    } else if (densityComboBox->currentText() == i18n("5.25\" 360KB")) {
        blocks = 360;
    }
#if defined(ANY_LINUX)
    else { // For Linux, anything else is Auto
        blocks = 0;
    }
#endif

    return true;
}

bool FloppyData::setInitialDevice(const QString &dev)
{
    QString newDevice = dev;

    QUrl url(newDevice);
    if (url.isValid() && (url.scheme() == QLatin1String("media") || url.scheme() == QLatin1String("system"))) {
        QString name = url.fileName();

        QDBusInterface mediamanager(QStringLiteral("org.kde.kded"), QStringLiteral("/modules/mediamanager"), QStringLiteral("org.kde.MediaManager"));
        QDBusReply<QStringList> reply = mediamanager.call(QStringLiteral("properties"), name);
        if (!reply.isValid()) {
            qCritical() << "Invalid reply from mediamanager";
        } else {
            const QStringList properties = reply;
            newDevice = properties.at(5);
        }
    }

    int drive = -1;
    if (newDevice.startsWith(QLatin1String("/dev/fd0")))
        drive = 0;
    else if (newDevice.startsWith(QLatin1String("/dev/fd1")))
        drive = 1;

    // ### TODO user given devices

    bool ok = (drive >= 0);
    if (ok)
        deviceComboBox->setCurrentIndex(drive);
    return ok;
}

void FloppyData::quit()
{
    if (formatActions)
        formatActions->quit();
    writeSettings();
    qApp->quit();
    delete this;
}

void FloppyData::setEnabled(bool b)
{
    if (b)
        unsetCursor();
    else
        setCursor(QCursor(Qt::WaitCursor));
    label1->setEnabled(b);
    deviceComboBox->setEnabled(b);
    label2->setEnabled(b);
    densityComboBox->setEnabled(b);
    label3->setEnabled(b);
    filesystemComboBox->setEnabled(b);
    buttongroup->setEnabled(b);
    quick->setEnabled(b);
    fullformat->setEnabled(b && m_canLowLevel);
    zerooutformat->setEnabled(b && m_canZeroOut);
    verifylabel->setEnabled(b);
    labellabel->setEnabled(b);
    lineedit->setEnabled(b && labellabel->isChecked());
    helpbutton->setEnabled(b);
    quitbutton->setEnabled(b);
    formatbutton->setEnabled(b);
    progress->setDisabled(b); // The other way round!
}

void FloppyData::reset()
{
    DEBUGSETUP;

    formating = false;

    if (formatActions) {
        formatActions->quit();
        delete formatActions;
        formatActions = nullptr;
    }

    progress->setValue(0);
    formatbutton->setText(i18n("&Format"));
    setEnabled(true);
}

void FloppyData::format()
{
    if (formating) {
        // abort = true;
        reset();
        return;
    }

    frame->clear();

    const QString currentComboBoxDevice(deviceComboBox->currentText());
    const bool userDevice = (currentComboBoxDevice.startsWith(QLatin1String("/dev/")));

#ifdef ANY_BSD
    if (userDevice && filesystemComboBox->currentText() != i18n("UFS")) {
        KMessageBox::error(this, i18nc("BSD", "Formatting with BSD on a user-given device is only possible with UFS"));
        return;
    }
    // no "else" !
#endif
    if (userDevice && (quick->isChecked() || zerooutformat->isChecked())) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("<qt>Formatting will erase all data on the device:<br/><b>%1</b><br/>"
                                                    "(Please check the correctness of the device name.)<br/>"
                                                    "Are you sure you wish to proceed?</qt>",
                                                    currentComboBoxDevice),
                                               i18n("Proceed?"))
            != KMessageBox::Continue) {
            return;
        }
    } else if (userDevice) {
        // The user has selected full formatting on a user-given device. That is not supported yet!
        KMessageBox::error(this, i18n("Full formatting of a user-given device is not possible."));
        return;
    } else {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("Formatting will erase all data on the disk.\n"
                                                    "Are you sure you wish to proceed?"),
                                               i18n("Proceed?"))
            != KMessageBox::Continue) {
            return;
        }
    }

    // formatbutton->setText(i18n("A&bort"));
    setEnabled(false);

    // Erase text box
    frame->setText(QString());

    if (!userDevice) {
        if (!findDevice()) {
            reset();
            return;
        }
    }

    delete formatActions;
    formatActions = new KFActionQueue(this);

    connect(formatActions, &KFAction::status, this, &FloppyData::formatStatus);
    connect(formatActions, &KFAction::done, this, &FloppyData::reset);

    if (quick->isChecked()) {
        formating = false;
        // No fdformat to push
    } else if (zerooutformat->isChecked()) {
        auto f = new DDZeroOut(this);
        if (userDevice) {
            f->configureDevice(currentComboBoxDevice);
        } else {
            f->configureDevice(drive, blocks);
        }
        formatActions->queue(f);
    } else if (userDevice) {
        // We should not have got here, assume quick format
        formating = false;
        // No fdformat to push
    } else {
        auto f = new FDFormat(this);
        f->configureDevice(drive, blocks);
        f->configure(verifylabel->isChecked());
        formatActions->queue(f);
    }

    if (filesystemComboBox->currentText() == i18n("DOS")) {
        auto f = new FATFilesystem(this);
        f->configure(verifylabel->isChecked(), labellabel->isChecked(), lineedit->text());
        if (userDevice) {
            f->configureDevice(currentComboBoxDevice);
        } else {
            f->configureDevice(drive, blocks);
        }
        formatActions->queue(f);
    }

    else if (filesystemComboBox->currentText() == i18n("ext2")) {
        auto f = new Ext2Filesystem(this);
        f->configure(verifylabel->isChecked(), labellabel->isChecked(), lineedit->text());
        if (userDevice) {
            f->configureDevice(currentComboBoxDevice);
        } else {
            f->configureDevice(drive, blocks);
        }
        formatActions->queue(f);
    }
#ifdef ANY_BSD
    else if (filesystemComboBox->currentText() == i18n("UFS")) {
        FloppyAction *f = new UFSFilesystem(this);
        f->configureDevice(drive, blocks);
        formatActions->queue(f);
    }
#endif

#ifdef ANY_LINUX
    else if (filesystemComboBox->currentText() == i18n("Minix")) {
        auto f = new MinixFilesystem(this);
        f->configure(verifylabel->isChecked(), labellabel->isChecked(), lineedit->text());
        if (userDevice) {
            f->configureDevice(currentComboBoxDevice);
        } else {
            f->configureDevice(drive, blocks);
        }
        formatActions->queue(f);
    }
#endif

    formatActions->exec();
}

void FloppyData::formatStatus(const QString &s, int p)
{
    qCDebug(KFLOPPY_LOG) << "FloppyData::formatStatus: " << s << " : " << p;
    if (!s.isEmpty()) {
        const QString oldText(frame->text());
        if (oldText.isEmpty()) {
            frame->setText(s);
        } else {
            frame->setText(oldText + QLatin1Char('\n') + s);
        }
    }

    if ((0 <= p) && (p <= 100))
        progress->setValue(p);
}

void FloppyData::writeSettings()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("GeneralData"));

    densityconfig = densityComboBox->currentText().trimmed();
    filesystemconfig = filesystemComboBox->currentText().trimmed();
    driveconfig = deviceComboBox->currentText().trimmed();

    quickformatconfig = quick->isChecked();

    labelnameconfig = lineedit->text().trimmed();

    labelconfig = labellabel->isChecked();

    verifyconfig = verifylabel->isChecked();

    config.writeEntry("CreateLabel", labelconfig);
    config.writeEntry("Label", labelnameconfig);

    config.writeEntry("QuickFormat", quickformatconfig);
    config.writeEntry("FloppyDrive", driveconfig);
    config.writeEntry("Density", densityconfig);
    config.writeEntry("Filesystem", filesystemconfig);
    config.writeEntry("Verify", verifyconfig);
    config.sync();
}

void FloppyData::readSettings()
{
    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("GeneralData"));

    verifyconfig = config.readEntry("Verify", 1);
    labelconfig = config.readEntry("CreateLabel", 1);
    labelnameconfig = config.readEntry("Label", i18nc("Volume label, maximal 11 characters", "KDE Floppy"));
    quickformatconfig = config.readEntry("QuickFormat", 0);
    driveconfig = config.readEntry("FloppyDrive", i18nc("Primary floppy drive", "Primary"));
#if defined(ANY_LINUX)
    densityconfig = config.readEntry("Density", i18n("Auto-Detect"));
#else
    densityconfig = config.readEntry("Density", i18n("3.5\" 1.44MB"));
#endif
    filesystemconfig = config.readEntry("Filesystem", i18n("DOS"));
}

void FloppyData::setWidgets()
{
    labellabel->setChecked(labelconfig);
    verifylabel->setChecked(verifyconfig);
    quick->setChecked(quickformatconfig || !m_canLowLevel);
    fullformat->setChecked(!quickformatconfig && m_canLowLevel);
    lineedit->setText(labelnameconfig);

    for (int i = 0; i < deviceComboBox->count(); i++) {
        if (deviceComboBox->itemText(i) == driveconfig) {
            deviceComboBox->setCurrentIndex(i);
        }
    }

    for (int i = 0; i < filesystemComboBox->count(); i++) {
        if (filesystemComboBox->itemText(i) == filesystemconfig) {
            filesystemComboBox->setCurrentIndex(i);
        }
    }

    for (int i = 0; i < densityComboBox->count(); i++) {
        if (densityComboBox->itemText(i) == densityconfig) {
            densityComboBox->setCurrentIndex(i);
        }
    }
}

#include "moc_floppy.cpp"
