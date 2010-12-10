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

#include "Organizer.h"

#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPainter>

LevelListWidget::LevelListWidget(QWidget* parent)
               : QListWidget(parent)
{
    setIconSize(QSize(128, 128));
    setSpacing(2);
    setDragDropMode(InternalMove);
}

void LevelListWidget::paintEvent(QPaintEvent* event)
{
    int pos = 0;
    while (pos < height()) {
        QListWidgetItem* item = itemAt(4, pos + 4);
        if (item != 0 && item->icon().isNull()) {
            emit loadLevelImage(row(item));
            pos += 128;
        } else {
            pos += 4;
        }
    }

    QListView::paintEvent(event);
}


OrganizerDialog::OrganizerDialog(QWidget* parent)
               : QDialog(parent)
{
    setWindowTitle(tr("Level Organizer"));

    m_levels = new LevelListWidget(this);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                                                     Qt::Horizontal, this);
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    layout->addWidget(m_levels, 0, 0);
    layout->addWidget(buttons, 1, 0);

    // The "Close" button is linked to the reject action
    connect(buttons, SIGNAL(rejected()), SLOT(close()));
    connect(m_levels, SIGNAL(loadLevelImage(int)), SLOT(loadLevelImage(int)));

    resize(500, 400);
}

void OrganizerDialog::loadLevelset(ccl::Levelset* levelset)
{
    m_levelset = levelset;
    m_levels->clear();

    for (int i=0; i<levelset->levelCount(); ++i) {
        ccl::LevelData* level = levelset->level(i);
        QString infoText = tr("%1 - %2\nPassword: %3\nChips: %4\nTime: %5\n%6")
                           .arg(i + 1).arg(level->name().c_str()).arg(level->password().c_str())
                           .arg(level->chips()).arg(level->timer()).arg(level->hint().c_str());
        new QListWidgetItem(infoText, m_levels);
    }
}

void OrganizerDialog::loadLevelImage(int level)
{
    QPixmap levelBuffer(32 * m_tileset->size(), 32 * m_tileset->size());
    QPainter tilePainter(&levelBuffer);

    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            m_tileset->draw(tilePainter, x, y, m_levelset->level(level)->map().getFG(x, y),
                            m_levelset->level(level)->map().getBG(x, y));

    m_levels->item(level)->setIcon(QIcon(levelBuffer.scaled(128, 128)));
}
