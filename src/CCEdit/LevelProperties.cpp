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

#include "LevelProperties.h"

#include "CommonWidgets/CCTools.h"
#include "CommonWidgets/LLTextEdit.h"
#include "libcc1/Levelset.h"

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QGridLayout>

LevelProperties::LevelProperties(QWidget* parent)
    : QWidget(parent)
{
    m_nameEdit = new QLineEdit(this);
    auto nameLabel = new QLabel(tr("&Name:"), this);
    nameLabel->setBuddy(m_nameEdit);
    m_nameEdit->setMaxLength(254);
    m_authorEdit = new QLineEdit(this);
    auto authorLabel = new QLabel(tr("&Author:"), this);
    authorLabel->setBuddy(m_authorEdit);
    m_authorEdit->setMaxLength(254);
    m_passwordEdit = new QLineEdit(this);
    auto passLabel = new QLabel(tr("&Password:"), this);
    passLabel->setBuddy(m_passwordEdit);
    m_passwordEdit->setMaxLength(9);
    auto passwordButton = new QToolButton(this);
    passwordButton->setIcon(ICON("view-refresh-sm"));
    passwordButton->setStatusTip(tr("Generate new random level password"));
    passwordButton->setAutoRaise(true);
    m_chipEdit = new QSpinBox(this);
    auto chipLabel = new QLabel(tr("&Chips:"), this);
    chipLabel->setBuddy(m_chipEdit);
    m_chipEdit->setRange(0, 32767);
    auto chipsButton = new QToolButton(this);
    chipsButton->setIcon(ICON("view-refresh-sm"));
    chipsButton->setStatusTip(tr("Count all chips in the selected level"));
    chipsButton->setAutoRaise(true);
    m_timeEdit = new QSpinBox(this);
    auto timeLabel = new QLabel(tr("&Time:"), this);
    timeLabel->setBuddy(m_timeEdit);
    m_timeEdit->setRange(0, 32767);
    m_hintEdit = new LLTextEdit(this);
    auto hintLabel = new QLabel(tr("&Hint Text:"), this);
    hintLabel->setBuddy(m_hintEdit);
    m_hintEdit->setMaxLength(254);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setVerticalSpacing(2);
    layout->setHorizontalSpacing(4);
    int row = 0;
    layout->addWidget(nameLabel, row, 0);
    layout->addWidget(m_nameEdit, row, 1, 1, 2);
    layout->addWidget(authorLabel, ++row, 0);
    layout->addWidget(m_authorEdit, row, 1, 1, 2);
    layout->addWidget(passLabel, ++row, 0);
    layout->addWidget(m_passwordEdit, row, 1);
    layout->addWidget(passwordButton, row, 2);
    layout->addWidget(chipLabel, ++row, 0);
    layout->addWidget(m_chipEdit, row, 1);
    layout->addWidget(chipsButton, row, 2);
    layout->addWidget(timeLabel, ++row, 0);
    layout->addWidget(m_timeEdit, row, 1);
    layout->addItem(new QSpacerItem(0, 8), ++row, 0, 1, 3);
    layout->addWidget(hintLabel, ++row, 0);
    layout->addWidget(m_hintEdit, row, 1, 1, 2);
    m_hintEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString& value) {
        nameChanged(ccl::toLatin1(value));
    });
    connect(m_authorEdit, &QLineEdit::textChanged, this, [this](const QString& value) {
        authorChanged(ccl::toLatin1(value));
    });
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [this](const QString& value) {
        passwordChanged(ccl::toLatin1(value));
    });
    connect(passwordButton, &QToolButton::clicked, this, [this] {
        m_passwordEdit->setText(ccl::fromLatin1(ccl::Levelset::RandomPassword()));
    });
    connect(m_chipEdit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LevelProperties::chipsChanged);
    connect(chipsButton, &QToolButton::clicked, this, &LevelProperties::chipCountRequested);
    connect(m_timeEdit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LevelProperties::timerChanged);
    connect(m_hintEdit, &QPlainTextEdit::textChanged, this, [this] {
        hintChanged(ccl::toLatin1(m_hintEdit->toPlainText()));
    });
}

void LevelProperties::clearAll()
{
    m_nameEdit->setText(QString());
    m_authorEdit->setText(QString());
    m_passwordEdit->setText(QString());
    m_chipEdit->setValue(0);
    m_timeEdit->setValue(0);
    m_hintEdit->setPlainText(QString());
}

void LevelProperties::updateLevelProperties(ccl::LevelData* level)
{
    m_nameEdit->setText(ccl::fromLatin1(level->name()));
    m_authorEdit->setText(ccl::fromLatin1(level->author()));
    m_passwordEdit->setText(ccl::fromLatin1(level->password()));
    m_chipEdit->setValue(level->chips());
    m_timeEdit->setValue(level->timer());
    m_hintEdit->setPlainText(ccl::fromLatin1(level->hint()));
}

void LevelProperties::countChips(const ccl::LevelMap& map)
{
    int chips = 0;
    for (int x = 0; x < 32; ++x) {
        for (int y = 0; y < 32; ++y) {
            if (map.getFG(x, y) == ccl::TileChip)
                ++chips;
            if (map.getBG(x, y) == ccl::TileChip)
                ++chips;
        }
    }
    m_chipEdit->setValue(chips);
}
