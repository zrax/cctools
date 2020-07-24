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

#include <QMouseEvent>
#include <QProxyStyle>
#include <QStyleOption>
#include <QPainter>

enum {
    TabNormal = 0,
    TabFloating = 1,
};

class FloatingTabStyle : public QProxyStyle {
public:
    explicit FloatingTabStyle(QTabBar* bar) : m_tabBar(bar) { }

    void drawControl(ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override
    {
        if (element == QStyle::CE_TabBarTab) {
            const int tabIndex = m_tabBar->tabAt(option->rect.center());
            if (m_tabBar->tabData(tabIndex).toInt() == TabFloating) {
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

private:
    QTabBar* m_tabBar;
};

class EditorTabBar : public QTabBar {
public:
    explicit EditorTabBar(QWidget* parent = nullptr)
        : QTabBar(parent), m_style(this)
    {
        setStyle(&m_style);
    }

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
    int floatingTab = -1;
    for (int i = 0; i < count(); ++i) {
        if (tabBar()->tabData(i).toInt() == TabFloating) {
            floatingTab = i;
            break;
        }
    }
    if (floatingTab >= 0)
        delete widget(floatingTab);

    int newIndex = addTab(tabWidget, label);
    tabBar()->setTabData(newIndex, TabFloating);
    if (floatingTab >= 0)
        tabBar()->moveTab(newIndex, floatingTab);
}

void EditorTabWidget::promoteTab()
{
    if (tabBar()->tabData(currentIndex()).toInt() == TabFloating) {
        tabBar()->setTabData(currentIndex(), TabNormal);
        tabBar()->update();
    }
}
