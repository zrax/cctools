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

#include "MapProperties.h"

#include "libcc2/Map.h"
#include "CommonWidgets/CCTools.h"
#include "ScriptEditor.h"

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>

MapProperties::MapProperties(QWidget* parent)
    : QWidget(parent)
{
    m_title = new QLineEdit(this);
    auto titleLabel = new QLabel(tr("&Title:"), this);
    titleLabel->setBuddy(m_title);
    m_author = new QLineEdit(this);
    auto authorLabel = new QLabel(tr("&Author:"), this);
    authorLabel->setBuddy(m_author);
    m_lockText = new QLineEdit(this);
    auto lockLabel = new QLabel(tr("&Lock:"), this);
    lockLabel->setBuddy(m_lockText);
    m_editorVersion = new QLineEdit(this);
    auto editorVersionLabel = new QLabel(tr("&Version:"), this);
    editorVersionLabel->setBuddy(m_editorVersion);

    m_mapSize = new QLineEdit(this);
    m_mapSize->setEnabled(false);
    auto mapSizeLabel = new QLabel(tr("Map &Size:"), this);
    mapSizeLabel->setBuddy(m_mapSize);
    auto resizeButton = new QPushButton(tr("&Resize..."), this);
    resizeButton->setStatusTip(tr("Resize the level"));
    m_chipCounter = new QLineEdit(this);
    m_chipCounter->setEnabled(false);
    auto chipLabel = new QLabel(tr("&Chips:"), this);
    chipLabel->setBuddy(m_chipCounter);
    m_pointCounter = new QLineEdit(this);
    m_pointCounter->setEnabled(false);
    auto pointLabel = new QLabel(tr("&Points:"), this);
    pointLabel->setBuddy(m_pointCounter);
    m_timeLimit = new QSpinBox(this);
    m_timeLimit->setRange(0, 32767);
    auto timeLabel = new QLabel(tr("Ti&me:"), this);
    timeLabel->setBuddy(m_timeLimit);
    m_viewport = new QComboBox(this);
    m_viewport->addItem(tr("10 x 10"), (int)cc2::MapOption::View10x10);
    m_viewport->addItem(tr("9 x 9"), (int)cc2::MapOption::View9x9);
    m_viewport->addItem(tr("Split"), (int)cc2::MapOption::ViewSplit);
    auto viewLabel = new QLabel(tr("Vie&w:"), this);
    viewLabel->setBuddy(m_viewport);
    m_blobPattern = new QComboBox(this);
    m_blobPattern->addItem(tr("Deterministic"), (int)cc2::MapOption::BlobsDeterministic);
    m_blobPattern->addItem(tr("4 Patterns"), (int)cc2::MapOption::Blobs4Pattern);
    m_blobPattern->addItem(tr("Extra Random"), (int)cc2::MapOption::BlobsExtraRandom);
    auto blobPatternLabel = new QLabel(tr("&Blobs:"), this);
    blobPatternLabel->setBuddy(m_blobPattern);
    m_hideLogic = new QCheckBox(tr("&Hide Logic"), this);
    m_cc1Boots = new QCheckBox(tr("CC&1 Boots"), this);
    m_readOnly = new QCheckBox(tr("&Read-Only"), this);
    auto optionsLabel = new QLabel(tr("Options:"), this);

    m_clue = new CC2ScriptEditor(CC2ScriptEditor::PlainMode, this);
    auto clueLabel = new QLabel(tr("C&lue:"), this);
    clueLabel->setBuddy(m_clue);
    m_note = new CC2ScriptEditor(CC2ScriptEditor::NotesMode, this);
    auto noteLabel = new QLabel(tr("&Notes:"), this);
    noteLabel->setBuddy(m_note);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
    layout->addWidget(titleLabel, row, 0);
    layout->addWidget(m_title, row, 1, 1, 2);
    layout->addWidget(authorLabel, ++row, 0);
    layout->addWidget(m_author, row, 1, 1, 2);
    layout->addWidget(lockLabel, ++row, 0);
    layout->addWidget(m_lockText, row, 1, 1, 2);
    layout->addWidget(editorVersionLabel, ++row, 0);
    layout->addWidget(m_editorVersion, row, 1, 1, 2);
    layout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed), ++row, 0);
    layout->addWidget(mapSizeLabel, ++row, 0);
    layout->addWidget(m_mapSize, row, 1);
    layout->addWidget(resizeButton, row, 2);
    layout->addWidget(chipLabel, ++row, 0);
    layout->addWidget(m_chipCounter, row, 1, 1, 2);
    layout->addWidget(pointLabel, ++row, 0);
    layout->addWidget(m_pointCounter, row, 1, 1, 2);
    layout->addWidget(timeLabel, ++row, 0);
    layout->addWidget(m_timeLimit, row, 1, 1, 2);
    layout->addWidget(viewLabel, ++row, 0);
    layout->addWidget(m_viewport, row, 1, 1, 2);
    layout->addWidget(blobPatternLabel, ++row, 0);
    layout->addWidget(m_blobPattern, row, 1, 1, 2);
    layout->addWidget(optionsLabel, ++row, 0);
    layout->addWidget(m_hideLogic, row, 1, 1, 2);
    layout->addWidget(m_cc1Boots, ++row, 1, 1, 2);
    layout->addWidget(m_readOnly, ++row, 1, 1, 2);
    layout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed), ++row, 0);
    layout->addWidget(clueLabel, ++row, 0, Qt::AlignTop);
    layout->addWidget(m_clue, row, 1, 1, 2);
    layout->addWidget(noteLabel, ++row, 0, Qt::AlignTop);
    layout->addWidget(m_note, row, 1, 1, 2);

    connect(m_title, &QLineEdit::textChanged, this, [this](const QString& value) {
        titleChanged(ccl::toLatin1(value));
    });
    connect(m_author, &QLineEdit::textChanged, this, [this](const QString& value) {
        authorChanged(ccl::toLatin1(value));
    });
    connect(m_lockText, &QLineEdit::textChanged, this, [this](const QString& value) {
        lockTextChanged(ccl::toLatin1(value));
    });
    connect(m_editorVersion, &QLineEdit::textChanged, this, [this](const QString& value) {
        editorVersionChanged(ccl::toLatin1(value));
    });
    connect(m_timeLimit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MapProperties::timeLimitChanged);
    connect(m_viewport, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        auto value = static_cast<cc2::MapOption::Viewport>(m_viewport->currentData().toInt());
        viewportChanged(value);
    });
    connect(m_blobPattern, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] {
        auto value = static_cast<cc2::MapOption::BlobPattern>(m_blobPattern->currentData().toInt());
        blobPatternChanged(value);
    });
    connect(m_hideLogic, &QCheckBox::toggled, this, &MapProperties::hideLogicChanged);
    connect(m_cc1Boots, &QCheckBox::toggled, this, &MapProperties::cc1BootsChanged);
    connect(m_readOnly, &QCheckBox::toggled, this, &MapProperties::readOnlyChanged);
    connect(m_clue, &QPlainTextEdit::textChanged, this, [this] {
        clueChanged(ccl::toLatin1(m_clue->toPlainText()));
    });
    connect(m_note, &QPlainTextEdit::textChanged, this, [this] {
        noteChanged(ccl::toLatin1(m_note->toPlainText()));
    });
    connect(resizeButton, &QPushButton::clicked, this, &MapProperties::mapResizeRequested);
}

void MapProperties::clearAll()
{
    m_title->setText(QString());
    m_author->setText(QString());
    m_lockText->setText(QString());
    m_editorVersion->setText(QString());
    m_mapSize->setText(QString());
    m_chipCounter->setText(QString());
    m_pointCounter->setText(QString());
    m_timeLimit->setValue(0);
    m_viewport->setCurrentIndex(0);
    m_blobPattern->setCurrentIndex(0);
    m_hideLogic->setChecked(false);
    m_cc1Boots->setChecked(false);
    m_readOnly->setChecked(false);
    m_clue->setPlainText(QString());
    m_note->setPlainText(QString());
}

void MapProperties::updateCounters(const cc2::MapData& mapData)
{
    m_chipCounter->setText(formatChips(mapData.countChips()));
    m_pointCounter->setText(formatPoints(mapData.countPoints()));
}

void MapProperties::updateMapProperties(cc2::Map* map)
{
    m_title->setText(ccl::fromLatin1(map->title()));
    m_author->setText(ccl::fromLatin1(map->author()));
    m_lockText->setText(ccl::fromLatin1(map->lock()));
    m_editorVersion->setText(ccl::fromLatin1(map->editorVersion()));
    m_mapSize->setText(tr("%1 x %2").arg(map->mapData().width())
                                    .arg(map->mapData().height()));
    updateCounters(map->mapData());
    m_timeLimit->setValue(map->option().timeLimit());
    m_viewport->setCurrentIndex(static_cast<int>(map->option().view()));
    m_blobPattern->setCurrentIndex(static_cast<int>(map->option().blobPattern()));
    m_hideLogic->setChecked(map->option().hideLogic());
    m_cc1Boots->setChecked(map->option().cc1Boots());
    m_readOnly->setChecked(map->readOnly());
    m_clue->setPlainText(ccl::fromLatin1(map->clue()));
    m_note->setPlainText(ccl::fromLatin1(map->note()));
}

QString MapProperties::formatChips(const std::tuple<int, int>& chips)
{
    if (std::get<0>(chips) != std::get<1>(chips))
        return tr("%1 (of %2)").arg(std::get<0>(chips)).arg(std::get<1>(chips));
    else
        return QString::number(std::get<0>(chips));
}

QString MapProperties::formatPoints(const std::tuple<int, int>& points)
{
    // Display >=16 x2 flags as an exponent instead
    const int x2flags = std::get<1>(points);
    if (x2flags >= 16)
        return tr("%1 (x2^%2)").arg(std::get<0>(points)).arg(x2flags);
    if (x2flags != 0)
        return tr("%1 (x%2)").arg(std::get<0>(points)).arg(1u << x2flags);
    return QString::number(std::get<0>(points));
}
