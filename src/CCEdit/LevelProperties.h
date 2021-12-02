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

#ifndef _LEVEL_PROPERTIES_H
#define _LEVEL_PROPERTIES_H

#include <QWidget>

class QLineEdit;
class QSpinBox;

class LLTextEdit;

namespace ccl
{
    class LevelData;
    class LevelMap;
}

class LevelProperties : public QWidget
{
    Q_OBJECT

public:
    LevelProperties(QWidget* parent);

    void clearAll();
    void updateLevelProperties(ccl::LevelData* level);
    void countChips(const ccl::LevelMap& map);

Q_SIGNALS:
    void nameChanged(const std::string&);
    void authorChanged(const std::string&);
    void passwordChanged(const std::string&);
    void chipsChanged(int);
    void timerChanged(int);
    void hintChanged(const std::string&);
    void chipCountRequested();

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_authorEdit;
    QLineEdit* m_passwordEdit;
    QSpinBox* m_chipEdit;
    QSpinBox* m_timeEdit;
    LLTextEdit* m_hintEdit;
};

#endif // _LEVEL_PROPERTIES_H

