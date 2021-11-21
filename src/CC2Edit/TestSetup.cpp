/******************************************************************************
 * This file is part of CCTools.                                              *
 *                                                                            *
 * CCTools is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * CCTools is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with CCTools.  If not, see <http://www.gnu.org/licenses/>.           *
 ******************************************************************************/

#include "TestSetup.h"

#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QGridLayout>
#include <QIntValidator>
#include <QFileDialog>
#include "CommonWidgets/CCTools.h"
#include "CommonWidgets/PathCompleter.h"

#ifdef Q_OS_WIN
#   define EXE_FILTER tr("Executables (*.exe)")
#   define WINEXE_FILTER tr("Executables (*.exe)")
#   define EXE_LIST QStringList{ QStringLiteral("*.exe") }
#   define WINEXE_LIST QStringList{ QStringLiteral("*.exe") }
#else
#   define EXE_FILTER tr("Executables (*)")
#   define WINEXE_FILTER tr("Windows Executables (*.exe *.EXE)")
#   define EXE_LIST QStringList()
#   define WINEXE_LIST QStringList{ QStringLiteral("*.exe"), QStringLiteral("*.EXE") }
#endif

TestSetupDialog::TestSetupDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Setup testing parameters"));

    QSettings settings;
    auto winExeCompleter = new FileCompleter(WINEXE_LIST, this);

#ifndef Q_OS_WIN
    auto exeCompleter = new FileCompleter(EXE_LIST, this);
    m_protonPath = new QLineEdit(settings.value(QStringLiteral("ProtonExe")).toString(), this);
    m_protonPath->setCompleter(exeCompleter);
    auto lblProtonPath = new QLabel(tr("&Proton Path:"), this);
    lblProtonPath->setBuddy(m_protonPath);
    auto browseProton = new QToolButton(this);
    browseProton->setIcon(ICON("document-open-folder-sm"));
    browseProton->setAutoRaise(true);

    auto dirCompleter = new DirCompleter(this);
    m_steamRoot = new QLineEdit(settings.value(QStringLiteral("SteamRoot")).toString(), this);
    m_steamRoot->setCompleter(dirCompleter);
    auto lblSteamPath = new QLabel(tr("&Steam root Path:"), this);
    lblSteamPath->setBuddy(m_steamRoot);
    auto browseSteam = new QToolButton(this);
    browseSteam->setIcon(ICON("document-open-folder-sm"));
    browseSteam->setAutoRaise(true);
#endif

    m_chips2Path = new QLineEdit(settings.value(QStringLiteral("Chips2Exe")).toString(), this);
    m_chips2Path->setCompleter(winExeCompleter);
    auto lblChips2Path = new QLabel(tr("&Chips2.exe Path:"), this);
    lblChips2Path->setBuddy(m_chips2Path);
    auto browseChips2 = new QToolButton(this);
    browseChips2->setIcon(ICON("document-open-folder-sm"));
    browseChips2->setAutoRaise(true);

    m_lexyUrl = new QLineEdit(settings.value(QStringLiteral("LexyUrl"),
                                             DEFAULT_LEXY_URL).toString(), this);
    auto lblLexyUrl = new QLabel(tr("&Lexy's Labyrinth URL:"), this);
    lblLexyUrl->setBuddy(m_lexyUrl);
    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
#ifndef Q_OS_WIN
    layout->addWidget(lblProtonPath, row, 0);
    layout->addWidget(m_protonPath, row, 1);
    layout->addWidget(browseProton, row, 2);
    layout->addWidget(lblSteamPath, ++row, 0);
    layout->addWidget(m_steamRoot, row, 1);
    layout->addWidget(browseSteam, row, 2);
    auto protonLabel = new QLabel(
            tr("Note: Leave Steam-related paths empty to use system-installed locations."),
            this);
    protonLabel->setWordWrap(true);
    layout->addWidget(protonLabel, ++row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum),
                    ++row, 0, 1, 3);
#endif
    layout->addWidget(lblChips2Path, ++row, 0);
    layout->addWidget(m_chips2Path, row, 1);
    layout->addWidget(browseChips2, row, 2);
    layout->addWidget(lblLexyUrl, ++row, 0);
    layout->addWidget(m_lexyUrl, row, 1, 1, 2);
    auto chips1Label = new QLabel(tr("Notes for playtesting in CC2: <ul>"
            "<li>The Steam version of Chip's Challenge 1 (chips1.exe) may also be used.</li>"
            "<li>Steam must be <b>running</b> and <b>logged in</b> for playtesting to work properly.</li>"
            "</ul>"), this);
    chips1Label->setWordWrap(true);
    layout->addWidget(chips1Label, ++row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 3);
    resize(400, sizeHint().height());

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &TestSetupDialog::onSaveSettings);
#ifndef Q_OS_WIN
    connect(browseProton, &QToolButton::clicked, this, &TestSetupDialog::onBrowseProton);
    connect(browseSteam, &QToolButton::clicked, this, &TestSetupDialog::onBrowseSteamRoot);
#endif
    connect(browseChips2, &QToolButton::clicked, this, &TestSetupDialog::onBrowseChips2);
}

void TestSetupDialog::onSaveSettings()
{
    QSettings settings;
#ifndef Q_OS_WIN
    settings.setValue(QStringLiteral("ProtonExe"), m_protonPath->text());
    settings.setValue(QStringLiteral("SteamRoot"), m_steamRoot->text());
#endif
    settings.setValue(QStringLiteral("Chips2Exe"), m_chips2Path->text());
    settings.setValue(QStringLiteral("LexyUrl"), m_lexyUrl->text());
    accept();
}

void TestSetupDialog::onBrowseProton()
{
#ifndef Q_OS_WIN
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Proton executable"),
                                m_protonPath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_protonPath->setText(path);
#else
    qCritical("onBrowseProton: Not supported on Windows platforms");
#endif
}

void TestSetupDialog::onBrowseSteamRoot()
{
#ifndef Q_OS_WIN
    QString path = QFileDialog::getExistingDirectory(this, tr("Browse for Steam Root"),
                                m_steamRoot->text());
    if (!path.isEmpty())
        m_steamRoot->setText(path);
#else
    qCritical("onBrowseSteamRoot: Not supported on Windows platforms");
#endif
}

void TestSetupDialog::onBrowseChips2()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Chips2 executable"),
                                m_chips2Path->text(), WINEXE_FILTER);
    if (!path.isEmpty())
        m_chips2Path->setText(path);
}
