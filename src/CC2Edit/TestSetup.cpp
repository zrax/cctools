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

    QSettings settings("CCTools", "CC2Edit");
    auto exeCompleter = new FileCompleter(EXE_LIST, this);
    auto winExeCompleter = new FileCompleter(QStringList{ "*.exe" }, this);

#ifndef Q_OS_WIN
    m_winePath = new QLineEdit(settings.value("WineExe").toString(), this);
    m_winePath->setCompleter(exeCompleter);
    auto lblWinePath = new QLabel(tr("&WINE Path:"), this);
    lblWinePath->setBuddy(m_winePath);
    auto browseWine = new QToolButton(this);
    browseWine->setIcon(QIcon(":/res/document-open-folder-sm.png"));
    browseWine->setAutoRaise(true);
#endif

    m_chips2Path = new QLineEdit(settings.value("Chips2Exe").toString(), this);
    m_chips2Path->setCompleter(winExeCompleter);
    auto lblChips2Path = new QLabel(tr("&Chips2.exe Path:"), this);
    lblChips2Path->setBuddy(m_chips2Path);
    auto browseChips2 = new QToolButton(this);
    browseChips2->setIcon(QIcon(":/res/document-open-folder-sm.png"));
    browseChips2->setAutoRaise(true);
    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
#ifndef Q_OS_WIN
    layout->addWidget(lblWinePath, row, 0);
    layout->addWidget(m_winePath, row, 1);
    layout->addWidget(browseWine, row, 2);
    auto wineLabel = new QLabel(
            tr("Note: Leave WINE path empty to use system-installed location."),
            this);
    wineLabel->setWordWrap(true);
    layout->addWidget(wineLabel, ++row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum),
                    ++row, 0, 1, 3);
#endif
    layout->addWidget(lblChips2Path, ++row, 0);
    layout->addWidget(m_chips2Path, row, 1);
    layout->addWidget(browseChips2, row, 2);
    auto chips1Label = new QLabel(
            tr("Note: The Steam version of Chip's Challenge 1 (chips1.exe) may also be used."),
            this);
    chips1Label->setWordWrap(true);
    layout->addWidget(chips1Label, ++row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 3);
    resize(400, sizeHint().height());

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &TestSetupDialog::onSaveSettings);
#ifndef Q_OS_WIN
    connect(browseWine, &QToolButton::clicked, this, &TestSetupDialog::onBrowseWine);
#endif
    connect(browseChips2, &QToolButton::clicked, this, &TestSetupDialog::onBrowseChips2);
}

void TestSetupDialog::onSaveSettings()
{
    QSettings settings("CCTools", "CC2Edit");
#ifndef Q_OS_WIN
    settings.setValue("WineExe", m_winePath->text());
#endif
    settings.setValue("Chips2Exe", m_chips2Path->text());
    accept();
}

void TestSetupDialog::onBrowseWine()
{
#ifndef Q_OS_WIN
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Wine executable"),
                                m_winePath->text(), EXE_FILTER);
    if (!path.isEmpty())
        m_winePath->setText(path);
#else
    qCritical("onBrowseWine: Not supported on Windows platforms");
#endif
}

void TestSetupDialog::onBrowseChips2()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Browse for Chips2 executable"),
                                m_chips2Path->text(), WINEXE_FILTER);
    if (!path.isEmpty())
        m_chips2Path->setText(path);
}
