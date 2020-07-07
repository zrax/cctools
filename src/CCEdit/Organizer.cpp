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
#include <QToolBar>
#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMessageBox>

static const QString s_clipboardFormat = QStringLiteral("CHIPEDIT LEVELS");

static QDataStream& operator<<(QDataStream& out, const ccl::LevelData *data)
{
    (void)data;
    return out;
}

static QDataStream& operator>>(QDataStream& in, ccl::LevelData *data)
{
    (void)data;
    return in;
}

LevelListWidget::LevelListWidget(QWidget* parent)
               : QListWidget(parent), m_tileset(0)
{
    setIconSize(QSize(128, 128));
    setSpacing(2);
    setDragDropMode(InternalMove);
    setSelectionMode(ExtendedSelection);

    static bool streamOperatorsRegistered = false;
    if (!streamOperatorsRegistered) {
        qRegisterMetaTypeStreamOperators<ccl::LevelData *>();
        streamOperatorsRegistered = true;
    }
}

LevelListWidget::~LevelListWidget()
{
    for (int i=0; i<count(); ++i)
        level(i)->unref();
}

void LevelListWidget::addLevel(ccl::LevelData* level)
{
    level->ref();

    QListWidgetItem* item = new QListWidgetItem(this);
    item->setData(Qt::UserRole, QVariant::fromValue(level));
    QString infoText = tr("%1\nPassword: %2\nChips: %3\nTime: %4\n%5")
                       .arg(level->name().c_str()).arg(level->password().c_str())
                       .arg(level->chips()).arg(level->timer()).arg(level->hint().c_str());
    item->setText(infoText);
}

void LevelListWidget::insertLevel(int row, ccl::LevelData* level)
{
    level->ref();

    QListWidgetItem* item = new QListWidgetItem();
    item->setData(Qt::UserRole, QVariant::fromValue(level));
    QString infoText = tr("%1\nPassword: %2\nChips: %3\nTime: %4\n%5")
                       .arg(level->name().c_str()).arg(level->password().c_str())
                       .arg(level->chips()).arg(level->timer()).arg(level->hint().c_str());
    item->setText(infoText);
    insertItem(row, item);
}

void LevelListWidget::delLevel(int row)
{
    level(row)->unref();
    delete takeItem(row);
}

void LevelListWidget::paintEvent(QPaintEvent* event)
{
    int pos = 0;
    while (pos < height()) {
        QListWidgetItem* item = itemAt(4, pos + 4);
        if (item != 0 && item->icon().isNull()) {
            loadLevelImage(row(item));
            pos += 128;
        } else {
            pos += 4;
        }
    }

    QListView::paintEvent(event);
}

void LevelListWidget::loadLevelImage(int row)
{
    QPixmap levelBuffer(32 * m_tileset->size(), 32 * m_tileset->size());
    QPainter tilePainter(&levelBuffer);

    ccl::LevelData* levelData = level(row);
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            m_tileset->draw(tilePainter, x, y, levelData->map().getFG(x, y),
                            levelData->map().getBG(x, y));
    item(row)->setIcon(QIcon(levelBuffer.scaled(128, 128)));
}


OrganizerDialog::OrganizerDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Level Organizer"));

    m_actions[ActionCut] = new QAction(QIcon(":/res/edit-cut.png"), tr("Cu&t"), this);
    m_actions[ActionCut]->setShortcut(Qt::CTRL | Qt::Key_X);
    m_actions[ActionCut]->setEnabled(false);
    m_actions[ActionCopy] = new QAction(QIcon(":/res/edit-copy.png"), tr("&Copy"), this);
    m_actions[ActionCopy]->setShortcut(Qt::CTRL | Qt::Key_C);
    m_actions[ActionCopy]->setEnabled(false);
    m_actions[ActionPaste] = new QAction(QIcon(":/res/edit-paste.png"), tr("&Paste"), this);
    m_actions[ActionPaste]->setShortcut(Qt::CTRL | Qt::Key_V);
    m_actions[ActionPaste]->setEnabled(false);
    m_actions[ActionDelete] = new QAction(QIcon(":/res/edit-delete.png"), tr("&Delete"), this);
    m_actions[ActionDelete]->setShortcut(Qt::Key_Delete);
    m_actions[ActionDelete]->setEnabled(false);

    m_levels = new LevelListWidget(this);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel,
                                        Qt::Horizontal, this);

    auto tbar = new QToolBar(this);
    tbar->addAction(m_actions[ActionCut]);
    tbar->addAction(m_actions[ActionCopy]);
    tbar->addAction(m_actions[ActionPaste]);
    tbar->addSeparator();
    tbar->addAction(m_actions[ActionDelete]);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    layout->addWidget(tbar, 0, 0);
    layout->addWidget(m_levels, 1, 0);
    layout->addWidget(buttons, 2, 0);

    connect(m_actions[ActionCut], &QAction::triggered, this, &OrganizerDialog::onCutLevels);
    connect(m_actions[ActionCopy], &QAction::triggered, this, &OrganizerDialog::onCopyLevels);
    connect(m_actions[ActionPaste], &QAction::triggered, this, &OrganizerDialog::onPasteLevels);
    connect(m_actions[ActionDelete], &QAction::triggered, this, &OrganizerDialog::onDeleteLevels);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_levels, &QListWidget::itemSelectionChanged, this, &OrganizerDialog::updateActions);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &OrganizerDialog::onClipboardDataChanged);

    resize(500, 400);
    onClipboardDataChanged();
}

void OrganizerDialog::loadLevelset(ccl::Levelset* levelset)
{
    m_levels->clear();
    for (int i = 0; i < levelset->levelCount(); ++i)
        m_levels->addLevel(levelset->level(i));
}

std::vector<ccl::LevelData*> OrganizerDialog::getLevels() const
{
    std::vector<ccl::LevelData*> levelOrder;
    levelOrder.resize(m_levels->count());

    for (int i = 0; i < m_levels->count(); ++i) {
        ccl::LevelData* level = m_levels->level(i);
        levelOrder[i] = level;
        level->ref();
    }
    return levelOrder;
}

void OrganizerDialog::updateActions()
{
    if (m_levels->selectedItems().size() > 0) {
        m_actions[ActionCut]->setEnabled(true);
        m_actions[ActionCopy]->setEnabled(true);
        m_actions[ActionDelete]->setEnabled(true);
    } else {
        m_actions[ActionCut]->setEnabled(false);
        m_actions[ActionCopy]->setEnabled(false);
        m_actions[ActionDelete]->setEnabled(false);
    }
}

void OrganizerDialog::onCutLevels()
{
    onCopyLevels();
    onDeleteLevels();
}

void OrganizerDialog::onCopyLevels()
{
    try {
        ccl::BufferStream cbStream;
        cbStream.write32(m_levels->selectedItems().count());
        QList<QListWidgetItem*> items = m_levels->selectedItems();
        QList<QListWidgetItem*> itemsReversed;
        while (!items.isEmpty()) {
            cbStream.write16(0);    // Revisit after writing data
            cbStream.write32(0);
            itemsReversed.append(items.takeLast());
        }

        long sizeOffs = 4;
        foreach (QListWidgetItem* item, itemsReversed) {
            long start = cbStream.tell();
            m_levels->level(m_levels->row(item))->write(&cbStream, true);
            long end = cbStream.tell();
            cbStream.seek(sizeOffs, SEEK_SET);
            cbStream.write16(end - start); // Size of data buffer
            cbStream.seek(end, SEEK_SET);
            sizeOffs += 6;
        }
        QByteArray buffer((const char*)cbStream.buffer(), cbStream.size());

        QMimeData* copyData = new QMimeData();
        copyData->setData(s_clipboardFormat, buffer);
        QApplication::clipboard()->setMimeData(copyData);
    } catch (std::exception& e) {
        QMessageBox::critical(this, tr("Error"),
                tr("Error saving clipboard data: %1").arg(e.what()),
                QMessageBox::Ok);
    }
}

void OrganizerDialog::onPasteLevels()
{
    const QMimeData* cbData = QApplication::clipboard()->mimeData();
    if (cbData->hasFormat(s_clipboardFormat)) {
        QByteArray buffer = cbData->data(s_clipboardFormat);
        ccl::BufferStream cbStream;
        cbStream.setFrom(buffer.constData(), buffer.size());

        unsigned int levelCount = cbStream.read32();
        QList<unsigned short> levelSizes;
        for (unsigned int i=0; i<levelCount; ++i) {
            levelSizes << cbStream.read16();
            cbStream.read32();  // Ignored
        }

        for (unsigned int i=0; i<levelCount; ++i) {
            long start = cbStream.tell();
            auto level = new ccl::LevelData();
            try {
                level->read(&cbStream, true);
                if (cbStream.tell() - start != levelSizes[i])
                    throw ccl::IOException("Corrupt Level Data");
            } catch (std::exception& ex) {
                QMessageBox::critical(this, tr("Error"),
                        tr("Error reading clipboard data: %1").arg(ex.what()),
                        QMessageBox::Ok);
                level->unref();
            }

            if (m_levels->currentItem() != 0)
                m_levels->insertLevel(m_levels->currentRow() + 1, level);
            else
                m_levels->addLevel(level);
            level->unref();
        }
    }
}

void OrganizerDialog::onDeleteLevels()
{
    foreach (QListWidgetItem* item, m_levels->selectedItems())
        m_levels->delLevel(m_levels->row(item));
}

void OrganizerDialog::onClipboardDataChanged()
{
    const QMimeData* cbData = QApplication::clipboard()->mimeData();
    m_actions[ActionPaste]->setEnabled(cbData->hasFormat(s_clipboardFormat));
}
