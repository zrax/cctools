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
#include <QDialogButtonBox>
#include <QToolButton>
#include <QGridLayout>
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
    auto exeCompleter = new FileCompleter(EXE_LIST, this);
    auto winExeCompleter = new FileCompleter(WINEXE_LIST, this);

    m_winePath = new QLineEdit(settings.value(QStringLiteral("WineExe")).toString(), this);
    m_winePath->setCompleter(exeCompleter);
    auto lblWinePath = new QLabel(
#                           ifdef Q_OS_WIN
                                  tr("&WineVDM Path:"),
#                           else
                                  tr("&WINE Path:"),
#                           endif
                                  this);
    lblWinePath->setBuddy(m_winePath);
    auto browseWine = new QToolButton(this);
    browseWine->setIcon(ICON("document-open-folder-sm"));
    browseWine->setAutoRaise(true);

    m_msccPath = new QLineEdit(settings.value(QStringLiteral("ChipsExe")).toString(), this);
    m_msccPath->setCompleter(winExeCompleter);
    auto lblMsccPath = new QLabel(tr("MS&CC Path:"), this);
    lblMsccPath->setBuddy(m_msccPath);
    auto browseChips = new QToolButton(this);
    browseChips->setIcon(ICON("document-open-folder-sm"));
    browseChips->setAutoRaise(true);
    m_tworldPath = new QLineEdit(settings.value(QStringLiteral("TWorldExe")).toString(), this);
    m_tworldPath->setCompleter(exeCompleter);
    auto lblTWorldPath = new QLabel(tr("&Tile World Path:"), this);
    lblTWorldPath->setBuddy(m_tworldPath);
    auto browseTWorld = new QToolButton(this);
    browseTWorld->setIcon(ICON("document-open-folder-sm"));
    browseTWorld->setAutoRaise(true);
    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    m_useCCPatch = new QCheckBox(tr("MSCC: Use CCPatch"), this);
    m_useCCPatch->setChecked(settings.value(QStringLiteral("TestCCPatch"), true).toBool());
    m_usePGPatch = new QCheckBox(tr("MSCC: Use PGChip (Ice Blocks)"), this);
    m_usePGPatch->setChecked(settings.value(QStringLiteral("TestPGPatch"), false).toBool());

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
    layout->addWidget(lblWinePath, row, 0);
    layout->addWidget(m_winePath, row, 1);
    layout->addWidget(browseWine, row, 2);
    layout->addWidget(lblMsccPath, ++row, 0);
    layout->addWidget(m_msccPath, row, 1);
    layout->addWidget(browseChips, row, 2);
    layout->addWidget(m_useCCPatch, ++row, 1);
    layout->addWidget(m_usePGPatch, ++row, 1);
    layout->addWidget(lblTWorldPath, ++row, 0);
    layout->addWidget(m_tworldPath, row, 1);
    layout->addWidget(browseTWorld, row, 2);
#ifndef Q_OS_WIN
    layout->addWidget(new QLabel(
            tr("Note: Leave WINE or Tile World paths empty to use system-installed locations"),
            this), ++row, 0, 1, 3);
#else
    auto winevdmLabel = new QLabel(
            tr("Note: <a href=\"https://github.com/otya128/winevdm\">WineVDM</a>"
               " is required to run MSCC on 64-bit Windows platforms"),
            this);
    winevdmLabel->setOpenExternalLinks(true);
    layout->addWidget(winevdmLabel, ++row, 0, 1, 3);
#endif
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 3);
    resize(400, sizeHint().height());

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &TestSetupDialog::onSaveSettings);
    connect(browseWine, &QToolButton::clicked, this, &TestSetupDialog::onBrowseWine);
    connect(browseChips, &QToolButton::clicked, this, &TestSetupDialog::onBrowseChips);
    connect(browseTWorld, &QToolButton::clicked, this, &TestSetupDialog::onBrowseTWorld);
}

void TestSetupDialog::onSaveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("WineExe"), m_winePath->text());
    settings.setValue(QStringLiteral("ChipsExe"), m_msccPath->text());
    settings.setValue(QStringLiteral("TWorldExe"), m_tworldPath->text());
    settings.setValue(QStringLiteral("TestCCPatch"), m_useCCPatch->isChecked());
    settings.setValue(QStringLiteral("TestPGPatch"), m_usePGPatch->isChecked());
    accept();
}

void TestSetupDialog::onBrowseWine()
{
    QString path = QFileDialog::getOpenFileName(this,
#ifdef Q_OS_WIN
                                tr("Browse for WineVDM executable"),
#else
                                tr("Browse for Wine executable"),
#endif
                                m_winePath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_winePath->setText(path);
}

void TestSetupDialog::onBrowseChips()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for MSCC executable"),
                                m_msccPath->text(), WINEXE_FILTER);
    if (!path.isEmpty())
        m_msccPath->setText(path);
}

void TestSetupDialog::onBrowseTWorld()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Tile World executable"),
                                m_tworldPath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_tworldPath->setText(path);
}
