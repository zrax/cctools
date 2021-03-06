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

#ifndef _PAGE_STORYLINE_H
#define _PAGE_STORYLINE_H

#include <QCheckBox>
#include "HackSettings.h"
#include "CommonWidgets/LLTextEdit.h"

#define DECL_STORY_PART(part)           \
    QCheckBox* m_cbStory##part;         \
    LLTextEdit* m_story##part;          \
    QPlainTextEdit* m_defStory##part

namespace CCHack {

class PageStoryline : public HackPage {
    Q_OBJECT

public:
    explicit PageStoryline(QWidget* parent = nullptr);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings* settings) override;
    void saveTo(HackSettings* settings) override;

private:
    DECL_STORY_PART(Part1);
    DECL_STORY_PART(Part2);
    DECL_STORY_PART(Part3);
    DECL_STORY_PART(Part4);
    DECL_STORY_PART(Part5);
    DECL_STORY_PART(Part6);
    DECL_STORY_PART(Part7);
    DECL_STORY_PART(Part8);
    DECL_STORY_PART(Part9);
    DECL_STORY_PART(Part10);
    DECL_STORY_PART(End1);
    DECL_STORY_PART(End2);
};

}

#undef DECL_STORY_PART

#endif
