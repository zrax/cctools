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

#include "EditorTabWidget.h"

#include <QToolButton>
#include <QMouseEvent>
#include <QProxyStyle>
#include <QStyleOption>
#include <QPainter>

class FloatingTabStyle : public QProxyStyle {
public:
    explicit FloatingTabStyle(QTabBar* bar)
        : m_floatingTab(-1), m_tabBar(bar) { }

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override
    {
        if (element == QStyle::CE_TabBarTab) {
            if (m_floatingTab >= 0 && m_tabBar->tabAt(option->rect.center()) == m_floatingTab) {
                QFont font = widget->font();
                font.setItalic(true);
                painter->save();
                painter->setFont(font);
                QProxyStyle::drawControl(element, option, painter, widget);
                painter->restore();
            } else {
                QProxyStyle::drawControl(element, option, painter, widget);
            }
        } else {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }

    void setFloatingTab(int tab) { m_floatingTab = tab; }
    int floatingTab() const { return m_floatingTab; }

private:
    int m_floatingTab;
    QTabBar* m_tabBar;
};

class EditorTabBar : public QTabBar {
public:
    explicit EditorTabBar(QWidget* parent = nullptr)
        : QTabBar(parent), m_style(this)
    {
        setStyle(&m_style);
        connect(this, &QTabBar::tabMoved, this, [this](int from, int to) {
            if (from == floatingTab())
                setFloatingTab(to);
            else if (from > floatingTab() && to <= floatingTab())
                setFloatingTab(floatingTab() + 1);
            else if (from < floatingTab() && to >= floatingTab())
                setFloatingTab(floatingTab() - 1);
        });
    }

    void setFloatingTab(int tab) { m_style.setFloatingTab(tab); }
    int floatingTab() const { return m_style.floatingTab(); }

protected:
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        int tab = tabAt(event->pos());
        if (tab >= 0 && event->button() == Qt::MiddleButton)
            emit tabCloseRequested(tab);

        QTabBar::mouseReleaseEvent(event);
    }

    QSize tabSizeHint(int index) const override
    {
        // Add a little bit of extra space for italicized tab captions.
        // We do this always, so tabs don't change size after being promoted.
        const QSize size = QTabBar::tabSizeHint(index);
        const QFontMetrics metrics(font());
        return QSize(size.width() + metrics.averageCharWidth(), size.height());
    }

    void tabInserted(int index) override
    {
        if (floatingTab() >= index)
            setFloatingTab(floatingTab() + 1);
        update();
        QTabBar::tabInserted(index);
    }

    void tabRemoved(int index) override
    {
        if (floatingTab() == index)
            setFloatingTab(-1);
        else if (floatingTab() > index)
            setFloatingTab(floatingTab() - 1);
        update();
        QTabBar::tabRemoved(index);
    }

private:
    FloatingTabStyle m_style;
};

EditorTabWidget::EditorTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    setTabBar(new EditorTabBar(this));
    setMovable(true);
    setTabsClosable(true);
}

void EditorTabWidget::addFloatingTab(QWidget *tabWidget, const QString &label)
{
    auto tabs = static_cast<EditorTabBar*>(tabBar());
    int floatingIndex = tabs->floatingTab();
    if (floatingIndex >= 0 && floatingIndex < count())
        delete widget(floatingIndex);

    int newIndex = addTab(tabWidget, label);
    tabs->setFloatingTab(newIndex);
    if (floatingIndex >= 0 && floatingIndex < count())
        tabs->moveTab(newIndex, floatingIndex);
}

void EditorTabWidget::promoteTab()
{
    auto tabs = static_cast<EditorTabBar*>(tabBar());
    if (tabs->floatingTab() >= 0) {
        tabs->setFloatingTab(-1);
        tabs->update();
    }
}
