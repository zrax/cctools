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

#include "AdvancedMechanics.h"

#include <QTabWidget>
#include <QToolBar>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGridLayout>
#include "libcc1/Tileset.h"

class OnePointDialog : public QDialog {
public:
    OnePointDialog(QWidget* parent) : QDialog(parent)
    {
        setWindowTitle(tr("Monster Position"));

        m_x = new QSpinBox(this);
        m_x->setRange(-128, 127);
        m_y = new QSpinBox(this);
        m_y->setRange(-128, 127);
        auto buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                Qt::Horizontal, this);

        auto layout = new QGridLayout(this);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setVerticalSpacing(8);
        layout->setHorizontalSpacing(8);
        layout->addWidget(new QLabel(tr("Position:"), this), 0, 0);
        layout->addWidget(m_x, 0, 1);
        layout->addWidget(m_y, 0, 2);
        layout->addWidget(buttons, 1, 0, 1, 3);

        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    void set(const ccl::Point& point)
    {
        m_x->setValue(point.X);
        m_y->setValue(point.Y);
    }

    ccl::Point point() const
    {
        ccl::Point pt;
        pt.X = m_x->value();
        pt.Y = m_y->value();
        return pt;
    }

private:
    QSpinBox* m_x;
    QSpinBox* m_y;
};


class TwoPointDialog : public QDialog {
public:
    TwoPointDialog(const QString& type, QWidget* parent) : QDialog(parent)
    {
        setWindowTitle(tr("%1 Connection").arg(type));

        m_x[0] = new QSpinBox(this);
        m_x[0]->setRange(-32768, 32767);
        m_y[0] = new QSpinBox(this);
        m_y[0]->setRange(-32768, 32767);
        m_x[1] = new QSpinBox(this);
        m_x[1]->setRange(-32768, 32767);
        m_y[1] = new QSpinBox(this);
        m_y[1]->setRange(-32768, 32767);
        auto buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                Qt::Horizontal, this);

        auto layout = new QGridLayout(this);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setVerticalSpacing(8);
        layout->setHorizontalSpacing(8);
        layout->addWidget(new QLabel(tr("Button:"), this), 0, 0);
        layout->addWidget(m_x[0], 0, 1);
        layout->addWidget(m_y[0], 0, 2);
        layout->addWidget(new QLabel(type, this), 1, 0);
        layout->addWidget(m_x[1], 1, 1);
        layout->addWidget(m_y[1], 1, 2);
        layout->addWidget(buttons, 2, 0, 1, 3);

        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    void set(const ccl::Point& p1, const ccl::Point& p2)
    {
        m_x[0]->setValue(p1.X);
        m_y[0]->setValue(p1.Y);
        m_x[1]->setValue(p2.X);
        m_y[1]->setValue(p2.Y);
    }

    ccl::Point point1() const
    {
        ccl::Point pt;
        pt.X = m_x[0]->value();
        pt.Y = m_y[0]->value();
        return pt;
    }

    ccl::Point point2() const
    {
        ccl::Point pt;
        pt.X = m_x[1]->value();
        pt.Y = m_y[1]->value();
        return pt;
    }

private:
    QSpinBox* m_x[2];
    QSpinBox* m_y[2];
};


static QString pointToStr(const ccl::Point& point)
{
    return QString("(%1, %2)").arg(point.X).arg(point.Y);
}

static bool isValidPoint(const ccl::Point& point)
{
    return (point.X >= 0 && point.X < 32 && point.Y >= 0 && point.Y < 32);
}


AdvancedMechanicsDialog::AdvancedMechanicsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Advanced Level Mechanics"));

    // Editing Actions
    m_actions[ActionAddTrap] = new QAction(QIcon(":/res/list-add.png"), tr("Add Trap Connection"), this);
    m_actions[ActionDelTrap] = new QAction(QIcon(":/res/list-remove.png"), tr("Remove Trap Connection"), this);
    m_actions[ActionTrapUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move Up"), this);
    m_actions[ActionTrapDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move Down"), this);
    m_actions[ActionAddClone] = new QAction(QIcon(":/res/list-add.png"), tr("Add Cloner Connection"), this);
    m_actions[ActionDelClone] = new QAction(QIcon(":/res/list-remove.png"), tr("Remove Cloner Connection"), this);
    m_actions[ActionCloneUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move Up"), this);
    m_actions[ActionCloneDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move Down"), this);
    m_actions[ActionAddMove] = new QAction(QIcon(":/res/list-add.png"), tr("Add Moving Monster"), this);
    m_actions[ActionDelMove] = new QAction(QIcon(":/res/list-remove.png"), tr("Remove Moving Monster"), this);
    m_actions[ActionMoveUp] = new QAction(QIcon(":/res/arrow-up.png"), tr("Move Up"), this);
    m_actions[ActionMoveDown] = new QAction(QIcon(":/res/arrow-down.png"), tr("Move Down"), this);

    // Dialog contents
    QTabWidget* tabs = new QTabWidget(this);
    QWidget* tabTraps = new QWidget(tabs);
    m_trapList = new QTreeWidget(tabTraps);
    m_trapList->setRootIsDecorated(false);
    m_trapList->setColumnCount(4);
    m_trapList->setHeaderLabels(QStringList() << "#" << tr("Button")
                                << tr("Trap") << tr("Status"));
    QToolBar* tbarTraps = new QToolBar(tabTraps);
    tbarTraps->addAction(m_actions[ActionAddTrap]);
    tbarTraps->addAction(m_actions[ActionDelTrap]);
    tbarTraps->addSeparator();
    tbarTraps->addAction(m_actions[ActionTrapUp]);
    tbarTraps->addAction(m_actions[ActionTrapDown]);
    QGridLayout* layTraps = new QGridLayout(tabTraps);
    layTraps->setContentsMargins(4, 4, 4, 4);
    layTraps->setVerticalSpacing(4);
    layTraps->addWidget(m_trapList, 0, 0);
    layTraps->addWidget(tbarTraps, 1, 0);
    tabs->addTab(tabTraps, tr("&Traps"));

    QWidget* tabClones = new QWidget(tabs);
    m_cloneList = new QTreeWidget(tabClones);
    m_cloneList->setRootIsDecorated(false);
    m_cloneList->setColumnCount(4);
    m_cloneList->setHeaderLabels(QStringList() << "#" << tr("Button")
                                 << tr("Cloner") << tr("Status"));
    QToolBar* tbarClones = new QToolBar(tabClones);
    tbarClones->addAction(m_actions[ActionAddClone]);
    tbarClones->addAction(m_actions[ActionDelClone]);
    tbarClones->addSeparator();
    tbarClones->addAction(m_actions[ActionCloneUp]);
    tbarClones->addAction(m_actions[ActionCloneDown]);
    QGridLayout* layClones = new QGridLayout(tabClones);
    layClones->setContentsMargins(4, 4, 4, 4);
    layClones->setVerticalSpacing(4);
    layClones->addWidget(m_cloneList, 0, 0);
    layClones->addWidget(tbarClones, 1, 0);
    tabs->addTab(tabClones, tr("&Cloners"));

    QWidget* tabMovers = new QWidget(tabs);
    m_moveOrderList = new QTreeWidget(tabMovers);
    m_moveOrderList->setRootIsDecorated(false);
    m_moveOrderList->setColumnCount(3);
    m_moveOrderList->setHeaderLabels(QStringList() << "#" << tr("Position")
                                     << tr("Contents"));
    QToolBar* tbarMovers = new QToolBar(tabMovers);
    tbarMovers->addAction(m_actions[ActionAddMove]);
    tbarMovers->addAction(m_actions[ActionDelMove]);
    tbarMovers->addSeparator();
    tbarMovers->addAction(m_actions[ActionMoveUp]);
    tbarMovers->addAction(m_actions[ActionMoveDown]);
    QGridLayout* layMovers = new QGridLayout(tabMovers);
    layMovers->setContentsMargins(4, 4, 4, 4);
    layMovers->setVerticalSpacing(4);
    layMovers->addWidget(m_moveOrderList, 0, 0);
    layMovers->addWidget(tbarMovers, 1, 0);
    tabs->addTab(tabMovers, tr("&Move Order"));

    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);
    auto layout = new QGridLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setVerticalSpacing(8);
    layout->addWidget(tabs, 0, 0);
    layout->addWidget(buttons, 1, 0);

    // Adjust columns
    int numWidth = m_trapList->fontMetrics().boundingRect("000").width() + 10;
    int pointWidth = m_trapList->fontMetrics().boundingRect("(00, 00)").width() + 20;
    m_trapList->setColumnWidth(0, numWidth);
    m_trapList->setColumnWidth(1, pointWidth);
    m_trapList->setColumnWidth(2, pointWidth);
    m_cloneList->setColumnWidth(0, numWidth);
    m_cloneList->setColumnWidth(1, pointWidth);
    m_cloneList->setColumnWidth(2, pointWidth);
    m_moveOrderList->setColumnWidth(0, numWidth);
    m_moveOrderList->setColumnWidth(1, pointWidth);

    connect(buttons, &QDialogButtonBox::accepted, this, &AdvancedMechanicsDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_trapList, &QTreeWidget::currentItemChanged,
            this, &AdvancedMechanicsDialog::onTrapSelect);
    connect(m_cloneList, &QTreeWidget::currentItemChanged,
            this, &AdvancedMechanicsDialog::onCloneSelect);
    connect(m_moveOrderList, &QTreeWidget::currentItemChanged,
            this, &AdvancedMechanicsDialog::onMoverSelect);
    connect(m_actions[ActionAddTrap], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onTrapAdd);
    connect(m_actions[ActionDelTrap], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onTrapDel);
    connect(m_actions[ActionTrapUp], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onTrapUp);
    connect(m_actions[ActionTrapDown], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onTrapDown);
    connect(m_actions[ActionAddClone], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onCloneAdd);
    connect(m_actions[ActionDelClone], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onCloneDel);
    connect(m_actions[ActionCloneUp], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onCloneUp);
    connect(m_actions[ActionCloneDown], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onCloneDown);
    connect(m_actions[ActionAddMove], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onMoverAdd);
    connect(m_actions[ActionDelMove], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onMoverDel);
    connect(m_actions[ActionMoveUp], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onMoverUp);
    connect(m_actions[ActionMoveDown], &QAction::triggered,
            this, &AdvancedMechanicsDialog::onMoverDown);
}

QTreeWidgetItem* AdvancedMechanicsDialog::addTrapItem(const ccl::Trap& trap)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(m_trapList);
    item->setText(0, QString("%1").arg(m_trapList->topLevelItemCount()));
    item->setText(1, pointToStr(trap.button));
    item->setText(2, pointToStr(trap.trap));
    if (isValidPoint(trap.button) && isValidPoint(trap.trap)
        && (m_levelData->map().getFG(trap.button.X, trap.button.Y) == ccl::TileTrapButton
            || m_levelData->map().getBG(trap.button.X, trap.button.Y) == ccl::TileTrapButton)
        && (m_levelData->map().getFG(trap.trap.X, trap.trap.Y) == ccl::TileTrap
            || m_levelData->map().getBG(trap.trap.X, trap.trap.Y) == ccl::TileTrap))
        item->setText(3, tr("OK"));
    else
        item->setText(3, tr("INVALID"));
    return item;
}

QTreeWidgetItem* AdvancedMechanicsDialog::addCloneItem(const ccl::Clone& clone)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(m_cloneList);
    item->setText(0, QString("%1").arg(m_cloneList->topLevelItemCount()));
    item->setText(1, pointToStr(clone.button));
    item->setText(2, pointToStr(clone.clone));
    if (isValidPoint(clone.button) && isValidPoint(clone.clone)
        && (m_levelData->map().getFG(clone.button.X, clone.button.Y) == ccl::TileCloneButton
            || m_levelData->map().getBG(clone.button.X, clone.button.Y) == ccl::TileCloneButton)
        && (m_levelData->map().getFG(clone.clone.X, clone.clone.Y) == ccl::TileCloner
            || m_levelData->map().getBG(clone.clone.X, clone.clone.Y) == ccl::TileCloner))
        item->setText(3, tr("OK"));
    else
        item->setText(3, tr("INVALID"));
    return item;
}

QTreeWidgetItem* AdvancedMechanicsDialog::addMoverItem(const ccl::Point& mover)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(m_moveOrderList);
    item->setText(0, QString("%1").arg(m_moveOrderList->topLevelItemCount()));
    item->setText(1, pointToStr(mover));
    if (isValidPoint(mover)) {
        if (m_levelData->map().getBG(mover.X, mover.Y) == ccl::TileFloor) {
            item->setText(2, CCETileset::TileName(m_levelData->map().getFG(mover.X, mover.Y)));
        } else {
            item->setText(2, QString("%1 / %2")
                    .arg(CCETileset::TileName(m_levelData->map().getFG(mover.X, mover.Y)))
                    .arg(CCETileset::TileName(m_levelData->map().getBG(mover.X, mover.Y))));
        }
    } else {
        item->setText(2, tr("INVALID"));
    }
    return item;
}

void AdvancedMechanicsDialog::setFrom(ccl::LevelData* level)
{
    m_levelData = level;
    m_trapList->clear();
    m_cloneList->clear();
    m_moveOrderList->clear();

    m_traps.clear();
    m_traps.reserve(MAX_TRAPS);
    m_clones.clear();
    m_clones.reserve(MAX_CLONES);
    m_moveOrder.clear();
    m_moveOrder.reserve(MAX_MOVERS);

    std::list<ccl::Trap>::const_iterator trap_iter;
    for (trap_iter = level->traps().begin(); trap_iter != level->traps().end(); ++trap_iter) {
        m_traps.push_back(*trap_iter);
        addTrapItem(*trap_iter);
    }
    m_actions[ActionAddTrap]->setEnabled(m_traps.size() < MAX_TRAPS);
    onTrapSelect(0, 0);

    std::list<ccl::Clone>::const_iterator clone_iter;
    for (clone_iter = level->clones().begin(); clone_iter != level->clones().end(); ++clone_iter) {
        m_clones.push_back(*clone_iter);
        addCloneItem(*clone_iter);
    }
    m_actions[ActionAddClone]->setEnabled(m_clones.size() < MAX_CLONES);
    onCloneSelect(0, 0);

    std::list<ccl::Point>::const_iterator move_iter;
    for (move_iter = level->moveList().begin(); move_iter != level->moveList().end(); ++move_iter) {
        m_moveOrder.push_back(*move_iter);
        addMoverItem(*move_iter);
    }
    m_actions[ActionAddMove]->setEnabled(m_moveOrder.size() < MAX_MOVERS);
    onMoverSelect(0, 0);
}

void AdvancedMechanicsDialog::onAccept()
{
    m_levelData->traps() = std::list<ccl::Trap>(m_traps.begin(), m_traps.end());
    m_levelData->clones() = std::list<ccl::Clone>(m_clones.begin(), m_clones.end());
    m_levelData->moveList() = std::list<ccl::Point>(m_moveOrder.begin(), m_moveOrder.end());
    accept();
}

void AdvancedMechanicsDialog::onTrapSelect(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    if (item == 0) {
        m_actions[ActionTrapUp]->setEnabled(false);
        m_actions[ActionTrapDown]->setEnabled(false);
        m_actions[ActionDelTrap]->setEnabled(false);
    } else {
        int idx = m_trapList->indexOfTopLevelItem(item);
        m_actions[ActionTrapUp]->setEnabled(idx > 0);
        m_actions[ActionTrapDown]->setEnabled(idx < m_trapList->topLevelItemCount() - 1);
        m_actions[ActionDelTrap]->setEnabled(true);
    }
}

void AdvancedMechanicsDialog::onCloneSelect(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    if (item == 0) {
        m_actions[ActionCloneUp]->setEnabled(false);
        m_actions[ActionCloneDown]->setEnabled(false);
        m_actions[ActionDelClone]->setEnabled(false);
    } else {
        int idx = m_cloneList->indexOfTopLevelItem(item);
        m_actions[ActionCloneUp]->setEnabled(idx > 0);
        m_actions[ActionCloneDown]->setEnabled(idx < m_cloneList->topLevelItemCount() - 1);
        m_actions[ActionDelClone]->setEnabled(true);
    }
}

void AdvancedMechanicsDialog::onMoverSelect(QTreeWidgetItem* item, QTreeWidgetItem*)
{
    if (item == 0) {
        m_actions[ActionMoveUp]->setEnabled(false);
        m_actions[ActionMoveDown]->setEnabled(false);
        m_actions[ActionDelMove]->setEnabled(false);
    } else {
        int idx = m_moveOrderList->indexOfTopLevelItem(item);
        m_actions[ActionMoveUp]->setEnabled(idx > 0);
        m_actions[ActionMoveDown]->setEnabled(idx < m_moveOrderList->topLevelItemCount() - 1);
        m_actions[ActionDelMove]->setEnabled(true);
    }
}

void AdvancedMechanicsDialog::onTrapAdd()
{
    if (m_traps.size() >= MAX_TRAPS)
        return;

    TwoPointDialog dlg(tr("Trap"), this);
    if (dlg.exec() == QDialog::Accepted) {
        ccl::Trap trap;
        trap.button = dlg.point1();
        trap.trap = dlg.point2();
        m_traps.push_back(trap);
        m_trapList->setCurrentItem(addTrapItem(trap));
        m_actions[ActionAddTrap]->setEnabled(m_traps.size() < MAX_TRAPS);
        m_actions[ActionDelTrap]->setEnabled(m_traps.size() > 0);
    }
}

void AdvancedMechanicsDialog::onTrapDel()
{
    QTreeWidgetItem* item = m_trapList->currentItem();
    if (item == 0 || m_trapList->indexOfTopLevelItem(item) < 0)
        return;

    m_traps.erase(m_traps.begin() + m_trapList->indexOfTopLevelItem(item));
    delete item;
    for (int i=0; i<m_trapList->topLevelItemCount(); ++i)
        m_trapList->topLevelItem(i)->setText(0, QString("%1").arg(i+1));
    m_actions[ActionAddTrap]->setEnabled(m_traps.size() < MAX_TRAPS);
    m_actions[ActionDelTrap]->setEnabled(m_traps.size() > 0);
}

void AdvancedMechanicsDialog::onTrapUp()
{
    QTreeWidgetItem* item = m_trapList->currentItem();
    if (item == 0)
        return;
    int idx = m_trapList->indexOfTopLevelItem(item);
    if (idx < 1)
        return;

    std::swap(m_traps[idx], m_traps[idx-1]);
    m_trapList->takeTopLevelItem(idx);
    m_trapList->insertTopLevelItem(idx-1, item);
    m_trapList->topLevelItem(idx-1)->setText(0, QString("%1").arg(idx));
    m_trapList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_trapList->setCurrentItem(item);
}

void AdvancedMechanicsDialog::onTrapDown()
{
    QTreeWidgetItem* item = m_trapList->currentItem();
    if (item == 0)
        return;
    int idx = m_trapList->indexOfTopLevelItem(item);
    if (idx < 0 || idx >= (int)m_traps.size() - 1)
        return;

    std::swap(m_traps[idx], m_traps[idx+1]);
    m_trapList->takeTopLevelItem(idx);
    m_trapList->insertTopLevelItem(idx+1, item);
    m_trapList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_trapList->topLevelItem(idx+1)->setText(0, QString("%1").arg(idx+2));
    m_trapList->setCurrentItem(item);
}

void AdvancedMechanicsDialog::onCloneAdd()
{
    if (m_clones.size() >= MAX_CLONES)
        return;

    TwoPointDialog dlg(tr("Cloner"), this);
    if (dlg.exec() == QDialog::Accepted) {
        ccl::Clone clone;
        clone.button = dlg.point1();
        clone.clone = dlg.point2();
        m_clones.push_back(clone);
        m_cloneList->setCurrentItem(addCloneItem(clone));
        m_actions[ActionAddClone]->setEnabled(m_clones.size() < MAX_CLONES);
        m_actions[ActionDelClone]->setEnabled(m_clones.size() > 0);
    }
}

void AdvancedMechanicsDialog::onCloneDel()
{
    QTreeWidgetItem* item = m_cloneList->currentItem();
    if (item == 0 || m_cloneList->indexOfTopLevelItem(item) < 0)
        return;

    m_clones.erase(m_clones.begin() + m_cloneList->indexOfTopLevelItem(item));
    delete item;
    for (int i=0; i<m_cloneList->topLevelItemCount(); ++i)
        m_cloneList->topLevelItem(i)->setText(0, QString("%1").arg(i+1));
    m_actions[ActionAddClone]->setEnabled(m_clones.size() < MAX_CLONES);
    m_actions[ActionDelClone]->setEnabled(m_clones.size() > 0);
}

void AdvancedMechanicsDialog::onCloneUp()
{
    QTreeWidgetItem* item = m_cloneList->currentItem();
    if (item == 0)
        return;
    int idx = m_cloneList->indexOfTopLevelItem(item);
    if (idx < 1)
        return;

    std::swap(m_clones[idx], m_clones[idx-1]);
    m_cloneList->takeTopLevelItem(idx);
    m_cloneList->insertTopLevelItem(idx-1, item);
    m_cloneList->topLevelItem(idx-1)->setText(0, QString("%1").arg(idx));
    m_cloneList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_cloneList->setCurrentItem(item);
}

void AdvancedMechanicsDialog::onCloneDown()
{
    QTreeWidgetItem* item = m_cloneList->currentItem();
    if (item == 0)
        return;
    int idx = m_cloneList->indexOfTopLevelItem(item);
    if (idx < 0 || idx >= (int)m_clones.size() - 1)
        return;

    std::swap(m_clones[idx], m_clones[idx+1]);
    m_cloneList->takeTopLevelItem(idx);
    m_cloneList->insertTopLevelItem(idx+1, item);
    m_cloneList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_cloneList->topLevelItem(idx+1)->setText(0, QString("%1").arg(idx+2));
    m_cloneList->setCurrentItem(item);
}

void AdvancedMechanicsDialog::onMoverAdd()
{
    if (m_moveOrder.size() >= MAX_MOVERS)
        return;

    OnePointDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        ccl::Point mover = dlg.point();
        m_moveOrder.push_back(mover);
        m_moveOrderList->setCurrentItem(addMoverItem(mover));
        m_actions[ActionAddMove]->setEnabled(m_moveOrder.size() < MAX_MOVERS);
        m_actions[ActionDelMove]->setEnabled(m_moveOrder.size() > 0);
    }
}

void AdvancedMechanicsDialog::onMoverDel()
{
    QTreeWidgetItem* item = m_moveOrderList->currentItem();
    if (item == 0 || m_moveOrderList->indexOfTopLevelItem(item) < 0)
        return;

    m_moveOrder.erase(m_moveOrder.begin() + m_moveOrderList->indexOfTopLevelItem(item));
    delete item;
    for (int i=0; i<m_moveOrderList->topLevelItemCount(); ++i)
        m_moveOrderList->topLevelItem(i)->setText(0, QString("%1").arg(i+1));
    m_actions[ActionAddMove]->setEnabled(m_moveOrder.size() < MAX_MOVERS);
    m_actions[ActionDelMove]->setEnabled(m_moveOrder.size() > 0);
}

void AdvancedMechanicsDialog::onMoverUp()
{
    QTreeWidgetItem* item = m_moveOrderList->currentItem();
    if (item == 0)
        return;
    int idx = m_moveOrderList->indexOfTopLevelItem(item);
    if (idx < 1)
        return;

    std::swap(m_moveOrder[idx], m_moveOrder[idx-1]);
    m_moveOrderList->takeTopLevelItem(idx);
    m_moveOrderList->insertTopLevelItem(idx-1, item);
    m_moveOrderList->topLevelItem(idx-1)->setText(0, QString("%1").arg(idx));
    m_moveOrderList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_moveOrderList->setCurrentItem(item);
}

void AdvancedMechanicsDialog::onMoverDown()
{
    QTreeWidgetItem* item = m_moveOrderList->currentItem();
    if (item == 0)
        return;
    int idx = m_moveOrderList->indexOfTopLevelItem(item);
    if (idx < 0 || idx >= (int)m_moveOrder.size() - 1)
        return;

    std::swap(m_moveOrder[idx], m_moveOrder[idx+1]);
    m_moveOrderList->takeTopLevelItem(idx);
    m_moveOrderList->insertTopLevelItem(idx+1, item);
    m_moveOrderList->topLevelItem(idx)->setText(0, QString("%1").arg(idx+1));
    m_moveOrderList->topLevelItem(idx+1)->setText(0, QString("%1").arg(idx+2));
    m_moveOrderList->setCurrentItem(item);
}
