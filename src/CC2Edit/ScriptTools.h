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

#ifndef _SCRIPTTOOLS_H
#define _SCRIPTTOOLS_H

#include <QObject>

class ScriptMapLoader : public QObject {
    Q_OBJECT

public:
    ScriptMapLoader(QObject* parent = nullptr)
        : QObject(parent) { }

    bool loadScript(const QString& filename);

signals:
    void gameName(const QString& name);
    void mapAdded(int levelNum, const QString& path);
};

#endif
