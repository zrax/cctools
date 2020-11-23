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

#ifndef _ABOUT_CCTOOLS_H
#define _ABOUT_CCTOOLS_H

#include <QDialog>

#define CCTOOLS_VERSION "3.0-beta2"
#define CCTOOLS_APP_VER "2.96"

#define ICON(name)  QIcon(QStringLiteral(":/res/" name ".png"))

class AboutWidget : public QWidget {
public:
    AboutWidget(const QString& name, const QPixmap& icon, QWidget *parent = nullptr);
};

class AboutDialog : public QDialog {
public:
    AboutDialog(const QString& name, const QPixmap& icon, QWidget* parent = nullptr);
};

namespace ccl {

inline QString fromLatin1(const std::string& text)
{
    return QString::fromLatin1(text.data(), int(text.size()));
}

inline std::string toLatin1(const QString& text)
{
    return text.toLatin1().toStdString();
}

}

#endif
