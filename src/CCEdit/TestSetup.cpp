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
#include "CommonWidgets/PathCompleter.h"

#ifdef Q_OS_WIN
#   define EXE_FILTER tr("Executables (*.exe)")
#   define WINEXE_FILTER tr("Executables (*.exe)")
#   define EXE_LIST QStringList{ QStringLiteral("*.exe") }
#else
#   define EXE_FILTER tr("Executables (*)")
#   define WINEXE_FILTER tr("Windows Executables (*.exe *.EXE)")
#   define EXE_LIST QStringList()
#endif

TestSetupDialog::TestSetupDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Setup testing parameters"));

    QSettings settings("CCTools", "CCEdit");
    auto exeCompleter = new FileCompleter(EXE_LIST, this);
    auto winExeCompleter = new FileCompleter(QStringList{ "*.exe" }, this);

#ifndef Q_OS_WIN
    m_winePath = new QLineEdit(settings.value("WineExe").toString(), this);
    m_winePath->setCompleter(exeCompleter);
    QLabel* lblWinePath = new QLabel(tr("&WINE Path:"), this);
    lblWinePath->setBuddy(m_winePath);
    QToolButton* browseWine = new QToolButton(this);
    browseWine->setIcon(QIcon(":/res/document-open-folder-sm.png"));
    browseWine->setAutoRaise(true);
#endif

    m_msccPath = new QLineEdit(settings.value("ChipsExe").toString(), this);
    m_msccPath->setCompleter(winExeCompleter);
    auto lblMsccPath = new QLabel(tr("MS&CC Path:"), this);
    lblMsccPath->setBuddy(m_msccPath);
    auto browseChips = new QToolButton(this);
    browseChips->setIcon(QIcon(":/res/document-open-folder-sm.png"));
    browseChips->setAutoRaise(true);
    m_tworldPath = new QLineEdit(settings.value("TWorldExe").toString(), this);
    m_tworldPath->setCompleter(exeCompleter);
    auto lblTWorldPath = new QLabel(tr("&Tile World Path:"), this);
    lblTWorldPath->setBuddy(m_tworldPath);
    auto browseTWorld = new QToolButton(this);
    browseTWorld->setIcon(QIcon(":/res/document-open-folder-sm.png"));
    browseTWorld->setAutoRaise(true);
    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    m_useCCPatch = new QCheckBox(tr("MSCC: Use CCPatch"), this);
    m_useCCPatch->setChecked(settings.value("TestCCPatch", true).toBool());
    m_usePGPatch = new QCheckBox(tr("MSCC: Use PGChip (Ice Blocks)"), this);
    m_usePGPatch->setChecked(settings.value("TestPGPatch", false).toBool());

    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
#ifndef Q_OS_WIN
    layout->addWidget(lblWinePath, row, 0);
    layout->addWidget(m_winePath, row, 1);
    layout->addWidget(browseWine, row, 2);
#endif
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
    layout->addWidget(new QLabel(
            tr("Note: MSCC will not work on 64-bit Windows platforms"),
            this), ++row, 0, 1, 3);
#endif
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 3);
    resize(400, sizeHint().height());

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &TestSetupDialog::onSaveSettings);
#ifndef Q_OS_WIN
    connect(browseWine, &QToolButton::clicked, this, &TestSetupDialog::onBrowseWine);
#endif
    connect(browseChips, &QToolButton::clicked, this, &TestSetupDialog::onBrowseChips);
    connect(browseTWorld, &QToolButton::clicked, this, &TestSetupDialog::onBrowseTWorld);
}

void TestSetupDialog::onSaveSettings()
{
    QSettings settings("CCTools", "CCEdit");
#ifndef Q_OS_WIN
    settings.setValue("WineExe", m_winePath->text());
#endif
    settings.setValue("ChipsExe", m_msccPath->text());
    settings.setValue("TWorldExe", m_tworldPath->text());
    settings.setValue("TestCCPatch", m_useCCPatch->isChecked());
    settings.setValue("TestPGPatch", m_usePGPatch->isChecked());
    accept();
}

#ifndef Q_OS_WIN
void TestSetupDialog::onBrowseWine()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Wine executable"),
                                m_winePath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_winePath->setText(path);
}
#endif

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
