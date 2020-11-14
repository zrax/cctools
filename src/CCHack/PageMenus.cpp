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

#include "PageMenus.h"
#include "libcc1/Win16Rsrc.h"
#include "CommonWidgets/CCTools.h"

#include <QAction>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

/* Enable drag-n-drop of QTreeWidgetItems with RcMenuItem data */
namespace {
    QDataStream& operator<<(QDataStream& out, const Win16::RcMenuItem&)
    {
        return out;
    }

    QDataStream& operator>>(QDataStream& in, const Win16::RcMenuItem&)
    {
        return in;
    }

    QDataStream& operator<<(QDataStream& out, const Win16::Accelerator&)
    {
        return out;
    }

    QDataStream& operator>>(QDataStream& in, const Win16::Accelerator&)
    {
        return in;
    }

    struct RegisterStreamOps {
        RegisterStreamOps()
        {
            qRegisterMetaTypeStreamOperators<Win16::RcMenuItem>();
            qRegisterMetaTypeStreamOperators<Win16::Accelerator>();
        }
    };
    RegisterStreamOps streamOps;
}

Q_DECLARE_METATYPE(Win16::RcMenuItem)
Q_DECLARE_METATYPE(Win16::Accelerator)

enum {
    MenuItemRole = Qt::UserRole,
    AccelRole,
};

static const QString SeparatorLine = QStringLiteral("----------------");

CCHack::PageMenus::PageMenus(QWidget* parent)
    : HackPage(parent)
{
    m_menuGroup = new QGroupBox(tr("Replace &Menus"), this);
    m_menuGroup->setCheckable(true);

    m_menuTree = new QTreeWidget(this);
    m_menuTree->setHeaderHidden(true);
    m_menuTree->setDragDropMode(QAbstractItemView::InternalMove);
    m_menuTree->setContextMenuPolicy(Qt::ActionsContextMenu);

    m_addMenuAction = new QAction(ICON("list-add"), tr("Add Item"), this);
    m_delMenuAction = new QAction(ICON("list-remove"), tr("Remove Item"), this);
    m_delMenuAction->setEnabled(false);
    m_menuTree->addAction(m_addMenuAction);
    m_menuTree->addAction(m_delMenuAction);

    m_menuItemProps = new QWidget(this);
    m_menuItemName = new QLineEdit(this);
    m_menuItemAccelName = new QLineEdit(this);
    m_menuItemId = new QSpinBox(this);
    m_menuItemId->setRange(0, 65535);
    m_menuItemGrayed = new QCheckBox(tr("&Grayed"), this);
    m_menuItemDisabled = new QCheckBox(tr("&Disabled"), this);
    m_menuItemChecked = new QCheckBox(tr("&Checked"), this);

    m_accelGroup = new QGroupBox(tr("Accelerator"), this);
    m_accelGroup->setCheckable(true);
    m_accelGroup->setChecked(false);

    m_accelKeyCode = new QSpinBox(this);
    m_accelKeyCode->setRange(0, 65535);
    m_accelVirtKey = new QCheckBox(tr("Virtual Key"), this);
    auto vkeyReference = new QLabel(tr("(<a href=\"https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes\">reference</a>)"), this);
    vkeyReference->setOpenExternalLinks(true);
    m_accelNoHighlight = new QCheckBox(tr("No Highlight"), this);
    m_accelShift = new QCheckBox(tr("SHIFT"), this);
    m_accelCtrl = new QCheckBox(tr("CTRL"), this);
    m_accelAlt = new QCheckBox(tr("ALT"), this);

    auto accelItemLayout = new QGridLayout(m_accelGroup);
    int row = 0;
    accelItemLayout->addWidget(new QLabel(tr("Key Code:"), this), ++row, 0);
    accelItemLayout->addWidget(m_accelKeyCode, row, 1, 1, 2);
    accelItemLayout->addWidget(new QLabel(tr("Flags:"), this), ++row, 0);
    accelItemLayout->addWidget(m_accelVirtKey, row, 1);
    accelItemLayout->addWidget(vkeyReference, row, 2);
    accelItemLayout->addWidget(m_accelNoHighlight, ++row, 1);
    accelItemLayout->addWidget(m_accelShift, row, 2);
    accelItemLayout->addWidget(m_accelCtrl, ++row, 1);
    accelItemLayout->addWidget(m_accelAlt, row, 2);

    auto menuItemLayout = new QGridLayout(m_menuItemProps);
    menuItemLayout->setContentsMargins(0, 0, 0, 0);
    row = 0;
    menuItemLayout->addWidget(new QLabel(tr("Text:"), this), row, 0);
    menuItemLayout->addWidget(m_menuItemName, row, 1);
    menuItemLayout->addWidget(new QLabel(tr("Shortcut:"), this), ++row, 0);
    menuItemLayout->addWidget(m_menuItemAccelName, row, 1);
    menuItemLayout->addWidget(new QLabel(tr("ID:"), this), ++row, 0);
    menuItemLayout->addWidget(m_menuItemId, row, 1);
    menuItemLayout->addWidget(new QLabel(tr("Flags:"), this), ++row, 0);
    menuItemLayout->addWidget(m_menuItemGrayed, row, 1);
    menuItemLayout->addWidget(m_menuItemDisabled, ++row, 1);
    menuItemLayout->addWidget(m_menuItemChecked, ++row, 1);
    menuItemLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                            QSizePolicy::Expanding), ++row, 0, 1, 3);
    menuItemLayout->addWidget(m_accelGroup, ++row, 0, 1, 3);

    auto menuLayout = new QHBoxLayout(m_menuGroup);
    menuLayout->addWidget(m_menuTree);
    menuLayout->addWidget(m_menuItemProps);
    m_menuItemProps->setEnabled(false);

    m_cbIgnorePasswords = new QCheckBox(tr("Cheat Menu:"), this);
    m_ignorePasswords = new QLineEdit(this);
    m_ignorePasswords->setEnabled(false);
    m_ignorePasswords->setMaxLength(17);
    m_defIgnorePasswords = new QLineEdit(this);
    m_defIgnorePasswords->setEnabled(false);

    auto layout = new QGridLayout(this);
    row = 0;
    layout->addWidget(m_menuGroup, row, 0, 1, 3);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(new QLabel(tr("Override"), this), ++row, 1);
    layout->addWidget(new QLabel(tr("Default"), this), row, 2);
    layout->addWidget(m_cbIgnorePasswords, ++row, 0);
    layout->addWidget(m_ignorePasswords, row, 1);
    layout->addWidget(m_defIgnorePasswords, row, 2);

    connect(m_menuTree, &QTreeWidget::currentItemChanged,
            this, &PageMenus::menuItemChanged);
    connect(m_cbIgnorePasswords, &QCheckBox::toggled,
            m_ignorePasswords, &QWidget::setEnabled);

    connect(m_addMenuAction, &QAction::triggered, this, [this] {
        QTreeWidgetItem* parent = m_menuTree->currentItem();
        if (!parent)
            parent = m_menuTree->invisibleRootItem();

        Win16::RcMenuItem item;
        item.setName("New Item");
        auto menuItem = new QTreeWidgetItem(parent, QStringList{ccl::fromLatin1(item.name())});
        menuItem->setData(0, MenuItemRole, QVariant::fromValue(item));
        parent->setExpanded(true);
        m_menuTree->setCurrentItem(menuItem);
    });
    connect(m_delMenuAction, &QAction::triggered, this, [this] {
        QTreeWidgetItem* selected = m_menuTree->currentItem();
        if (!selected)
            return;
        delete selected;
    });
}

static void addMenuItems(const std::vector<Win16::RcMenuItem>& menu,
                         const Win16::AccelResource& accels,
                         QTreeWidgetItem* parent)
{
    for (const Win16::RcMenuItem& item : menu) {
        QString itemName = ccl::fromLatin1(item.name());
        if (itemName.isEmpty())
            itemName = SeparatorLine;
        auto treeItem = new QTreeWidgetItem(parent, QStringList{itemName});
        treeItem->setData(0, MenuItemRole, QVariant::fromValue(item));
        if (!item.children().empty()) {
            addMenuItems(item.children(), accels, treeItem);
            treeItem->setExpanded(true);
        }

        // Associate accelerators
        for (const Win16::Accelerator& accel : accels.accelerators()) {
            if (accel.id() == item.id()) {
                treeItem->setData(0, AccelRole, QVariant::fromValue(accel));
                break;
            }
        }
    }
}

static std::vector<Win16::RcMenuItem>
buildMenu(Win16::AccelResource& accels, QTreeWidgetItem* parent)
{
    std::vector<Win16::RcMenuItem> menuItems;

    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem* item = parent->child(i);
        menuItems.emplace_back(item->data(0, MenuItemRole).value<Win16::RcMenuItem>());
        menuItems.back().children() = buildMenu(accels, item);

        if (!item->data(0, AccelRole).isNull())
            accels.accelerators().emplace_back(item->data(0, AccelRole).value<Win16::Accelerator>());
    }

    return menuItems;
}

void CCHack::PageMenus::setValues(HackSettings* settings)
{
    m_cbIgnorePasswords->setChecked(settings->have_ignorePasswords());
    m_ignorePasswords->setText(ccl::fromLatin1(settings->get_ignorePasswords()));

    QByteArray blob = settings->get_chipsMenu();
    m_menuGroup->setChecked(!blob.isEmpty());
    m_menuTree->clear();
    if (!blob.isEmpty()) {
        ccl::BufferStream stream;
        stream.setFrom(blob.data(), blob.size());
        Win16::MenuResource menu;
        try {
            menu.read(&stream);
        } catch (const ccl::RuntimeError& error) {
            QMessageBox::critical(this, tr("Error reading menus"), error.message());
        }

        blob = settings->get_chipsMenuAccel();
        Win16::AccelResource accels;
        if (!blob.isEmpty()) {
            stream.setFrom(blob.data(), blob.size());
            try {
                accels.read(&stream);
            } catch (const ccl::RuntimeError& error) {
                QMessageBox::critical(this, tr("Error reading accelerators"), error.message());
            }
        }

        addMenuItems(menu.menus(), accels, m_menuTree->invisibleRootItem());
    }
}

void CCHack::PageMenus::setDefaults(HackSettings* settings)
{
    m_defIgnorePasswords->setText(ccl::fromLatin1(settings->get_ignorePasswords()));
}

void CCHack::PageMenus::saveTo(HackSettings *settings)
{
    if (m_menuTree->currentItem())
        updateMenuItem(m_menuTree->currentItem());

    if (m_menuGroup->isChecked()) {
        // Generate new Menu and Accelerator structures
        Win16::MenuResource menu;
        Win16::AccelResource accels;
        menu.menus() = buildMenu(accels, m_menuTree->invisibleRootItem());

        std::sort(accels.accelerators().begin(), accels.accelerators().end(),
                  [](const Win16::Accelerator& first, const Win16::Accelerator& second) {
            // Attempt to preserve the original order from the file whenever possible
            // New items (order == -1) should go at the end.
            if (second.order() < 0)
                return true;
            if (first.order() < 0)
                return false;
            return first.order() < second.order();
        });

        ccl::BufferStream stream;
        try {
            menu.write(&stream);
            settings->set_chipsMenu(QByteArray((const char*)stream.buffer(), stream.size()));
        } catch (const ccl::RuntimeError& error) {
            QMessageBox::critical(this, tr("Error writing menus"), error.message());
        }

        stream.setFrom(nullptr, 0);
        try {
            accels.write(&stream);
            settings->set_chipsMenuAccel(QByteArray((const char*)stream.buffer(), stream.size()));
        } catch (const ccl::RuntimeError& error) {
            QMessageBox::critical(this, tr("Error writing accelerators"), error.message());
        }
    }

    if (m_cbIgnorePasswords->isChecked())
        settings->set_ignorePasswords(ccl::toLatin1(m_ignorePasswords->text()));
}

void CCHack::PageMenus::menuItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (previous)
        updateMenuItem(previous);

    m_menuItemProps->setEnabled(current != nullptr);
    m_delMenuAction->setEnabled(current != nullptr);
    if (current) {
        auto item = current->data(0, MenuItemRole).value<Win16::RcMenuItem>();
        const QString itemName = ccl::fromLatin1(item.name());
        const int accelTab = itemName.lastIndexOf(QLatin1Char('\t'));
        if (accelTab >= 0) {
            m_menuItemName->setText(itemName.left(accelTab));
            m_menuItemAccelName->setText(itemName.mid(accelTab + 1));
        } else {
            m_menuItemName->setText(itemName);
            m_menuItemAccelName->setText(QString());
        }
        m_menuItemId->setValue(item.id());
        m_menuItemId->setEnabled(current->childCount() == 0);
        m_menuItemGrayed->setChecked((item.flags() & Win16::RcMenuItem::MF_GRAYED) != 0);
        m_menuItemDisabled->setChecked((item.flags() & Win16::RcMenuItem::MF_DISABLED) != 0);
        m_menuItemChecked->setChecked((item.flags() & Win16::RcMenuItem::MF_CHECKED) != 0);
        m_accelGroup->setChecked(current->data(0, AccelRole).isValid());
    } else {
        m_menuItemName->setText(QString());
        m_menuItemAccelName->setText(QString());
        m_menuItemId->setValue(0);
        m_menuItemGrayed->setChecked(false);
        m_menuItemDisabled->setChecked(false);
        m_menuItemChecked->setChecked(false);
        m_accelGroup->setChecked(false);
    }

    if (current && m_accelGroup->isChecked()) {
        auto accel = current->data(0, AccelRole).value<Win16::Accelerator>();
        m_accelKeyCode->setValue(accel.event());
        m_accelVirtKey->setChecked((accel.flags() & Win16::Accelerator::FVIRTKEY) != 0);
        m_accelNoHighlight->setChecked((accel.flags() & Win16::Accelerator::FNOINVERT) != 0);
        m_accelShift->setChecked((accel.flags() & Win16::Accelerator::FSHIFT) != 0);
        m_accelCtrl->setChecked((accel.flags() & Win16::Accelerator::FCONTROL) != 0);
        m_accelAlt->setChecked((accel.flags() & Win16::Accelerator::FALT) != 0);
    } else {
        m_accelKeyCode->setValue(0);
        m_accelVirtKey->setChecked(false);
        m_accelNoHighlight->setChecked(false);
        m_accelShift->setChecked(false);
        m_accelCtrl->setChecked(false);
        m_accelAlt->setChecked(false);
    }
}

template <typename FlagType>
static void updateFlag(FlagType& flags, bool isChecked, unsigned value)
{
    flags = isChecked ? (flags | value) : (flags & ~value);
}

void CCHack::PageMenus::updateMenuItem(QTreeWidgetItem* current)
{
    auto item = current->data(0, MenuItemRole).value<Win16::RcMenuItem>();
    QString itemName = m_menuItemName->text();
    if (!m_menuItemAccelName->text().isEmpty())
        itemName += QLatin1Char('\t') + m_menuItemAccelName->text();
    item.setName(ccl::toLatin1(itemName));
    item.setId(m_menuItemId->value());
    uint16_t flags = item.flags();
    updateFlag(flags, m_menuItemGrayed->isChecked(), Win16::RcMenuItem::MF_GRAYED);
    updateFlag(flags, m_menuItemDisabled->isChecked(), Win16::RcMenuItem::MF_DISABLED);
    updateFlag(flags, m_menuItemChecked->isChecked(), Win16::RcMenuItem::MF_CHECKED);
    item.setFlags(flags);

    current->setText(0, itemName.isEmpty() ? SeparatorLine : itemName);
    current->setData(0, MenuItemRole, QVariant::fromValue(item));

    if (m_accelGroup->isChecked()) {
        Win16::Accelerator accel;
        if (!current->data(0, AccelRole).isNull())
            accel = current->data(0, AccelRole).value<Win16::Accelerator>();

        accel.setId(m_menuItemId->value());
        accel.setEvent(m_accelKeyCode->value());
        uint8_t accelFlags = accel.flags();
        updateFlag(accelFlags, m_accelVirtKey->isChecked(), Win16::Accelerator::FVIRTKEY);
        updateFlag(accelFlags, m_accelNoHighlight->isChecked(), Win16::Accelerator::FNOINVERT);
        updateFlag(accelFlags, m_accelShift->isChecked(), Win16::Accelerator::FSHIFT);
        updateFlag(accelFlags, m_accelCtrl->isChecked(), Win16::Accelerator::FCONTROL);
        updateFlag(accelFlags, m_accelAlt->isChecked(), Win16::Accelerator::FALT);
        accel.setFlags(accelFlags);
        current->setData(0, AccelRole, QVariant::fromValue(accel));
    } else {
        current->setData(0, AccelRole, QVariant());
    }
}
