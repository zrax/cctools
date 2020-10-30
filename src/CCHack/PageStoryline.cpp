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

#include "PageStoryline.h"
#include "CommonWidgets/CCTools.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

#define INIT_STORY_PART(part, label, maxLength)                 \
    m_cbStory##part = new QCheckBox(label, this);               \
    m_story##part = new LLTextEdit(this);                       \
    m_story##part->setEnabled(false);                           \
    m_story##part->setMaxLength(maxLength);                     \
    m_defStory##part = new QPlainTextEdit(this);                \
    m_defStory##part->setEnabled(false);                        \
    layout->addWidget(m_cbStory##part, ++row, 0);               \
    layout->addWidget(m_story##part, row, 1);                   \
    layout->addWidget(m_defStory##part, row, 2);                \
    connect(m_cbStory##part, &QCheckBox::toggled,               \
            m_story##part, &QWidget::setEnabled)

CCHack::PageStoryline::PageStoryline(QWidget* parent)
    : HackPage(parent)
{
    auto scroll = new QScrollArea(this);
    auto content = new QWidget(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto layout = new QGridLayout(content);
    int row = 0;
    layout->addWidget(new QLabel(tr("Override"), this), row, 1);
    layout->addWidget(new QLabel(tr("Default"), this), row, 2);

    INIT_STORY_PART(Part1, tr("Part 1:"), 129);
    INIT_STORY_PART(Part2, tr("Part 2:"), 114);
    INIT_STORY_PART(Part3, tr("Part 3:"), 121);
    INIT_STORY_PART(Part4, tr("Part 4:"), 123);
    INIT_STORY_PART(Part5, tr("Part 5:"), 120);
    INIT_STORY_PART(Part6, tr("Part 6:"), 114);
    INIT_STORY_PART(Part7, tr("Part 7:"), 125);
    INIT_STORY_PART(Part8, tr("Part 8:"), 101);
    INIT_STORY_PART(Part9, tr("Part 9:"), 131);
    INIT_STORY_PART(Part10, tr("Part 10:"), 60);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    INIT_STORY_PART(End1, tr("Endgame 1:"), 57);
    INIT_STORY_PART(End2, tr("Endgame 2:"), 154);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), ++row, 0, 1, 3);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(scroll);
}

void CCHack::PageStoryline::setValues(HackSettings* settings)
{
    m_cbStoryPart1->setChecked(settings->have_progressMsg_1());
    m_storyPart1->setPlainText(ccl::fromLatin1(settings->get_progressMsg_1()));
    m_cbStoryPart2->setChecked(settings->have_progressMsg_2());
    m_storyPart2->setPlainText(ccl::fromLatin1(settings->get_progressMsg_2()));
    m_cbStoryPart3->setChecked(settings->have_progressMsg_3());
    m_storyPart3->setPlainText(ccl::fromLatin1(settings->get_progressMsg_3()));
    m_cbStoryPart4->setChecked(settings->have_progressMsg_4());
    m_storyPart4->setPlainText(ccl::fromLatin1(settings->get_progressMsg_4()));
    m_cbStoryPart5->setChecked(settings->have_progressMsg_5());
    m_storyPart5->setPlainText(ccl::fromLatin1(settings->get_progressMsg_5()));
    m_cbStoryPart6->setChecked(settings->have_progressMsg_6());
    m_storyPart6->setPlainText(ccl::fromLatin1(settings->get_progressMsg_6()));
    m_cbStoryPart7->setChecked(settings->have_progressMsg_7());
    m_storyPart7->setPlainText(ccl::fromLatin1(settings->get_progressMsg_7()));
    m_cbStoryPart8->setChecked(settings->have_progressMsg_8());
    m_storyPart8->setPlainText(ccl::fromLatin1(settings->get_progressMsg_8()));
    m_cbStoryPart9->setChecked(settings->have_progressMsg_9());
    m_storyPart9->setPlainText(ccl::fromLatin1(settings->get_progressMsg_9()));
    m_cbStoryPart10->setChecked(settings->have_progressMsg_10());
    m_storyPart10->setPlainText(ccl::fromLatin1(settings->get_progressMsg_10()));
    m_cbStoryEnd1->setChecked(settings->have_endgameMsg_1());
    m_storyEnd1->setPlainText(ccl::fromLatin1(settings->get_endgameMsg_1()));
    m_cbStoryEnd2->setChecked(settings->have_endgameMsg_2());
    m_storyEnd2->setPlainText(ccl::fromLatin1(settings->get_endgameMsg_2()));
}

void CCHack::PageStoryline::setDefaults(HackSettings* settings)
{
    m_defStoryPart1->setPlainText(ccl::fromLatin1(settings->get_progressMsg_1()));
    m_defStoryPart2->setPlainText(ccl::fromLatin1(settings->get_progressMsg_2()));
    m_defStoryPart3->setPlainText(ccl::fromLatin1(settings->get_progressMsg_3()));
    m_defStoryPart4->setPlainText(ccl::fromLatin1(settings->get_progressMsg_4()));
    m_defStoryPart5->setPlainText(ccl::fromLatin1(settings->get_progressMsg_5()));
    m_defStoryPart6->setPlainText(ccl::fromLatin1(settings->get_progressMsg_6()));
    m_defStoryPart7->setPlainText(ccl::fromLatin1(settings->get_progressMsg_7()));
    m_defStoryPart8->setPlainText(ccl::fromLatin1(settings->get_progressMsg_8()));
    m_defStoryPart9->setPlainText(ccl::fromLatin1(settings->get_progressMsg_9()));
    m_defStoryPart10->setPlainText(ccl::fromLatin1(settings->get_progressMsg_10()));
    m_defStoryEnd1->setPlainText(ccl::fromLatin1(settings->get_endgameMsg_1()));
    m_defStoryEnd2->setPlainText(ccl::fromLatin1(settings->get_endgameMsg_2()));
}

void CCHack::PageStoryline::saveTo(HackSettings* settings)
{
    if (m_cbStoryPart1->isChecked())
        settings->set_progressMsg_1(ccl::toLatin1(m_storyPart1->toPlainText()));
    if (m_cbStoryPart2->isChecked())
        settings->set_progressMsg_2(ccl::toLatin1(m_storyPart2->toPlainText()));
    if (m_cbStoryPart3->isChecked())
        settings->set_progressMsg_3(ccl::toLatin1(m_storyPart3->toPlainText()));
    if (m_cbStoryPart4->isChecked())
        settings->set_progressMsg_4(ccl::toLatin1(m_storyPart4->toPlainText()));
    if (m_cbStoryPart5->isChecked())
        settings->set_progressMsg_5(ccl::toLatin1(m_storyPart5->toPlainText()));
    if (m_cbStoryPart6->isChecked())
        settings->set_progressMsg_6(ccl::toLatin1(m_storyPart6->toPlainText()));
    if (m_cbStoryPart7->isChecked())
        settings->set_progressMsg_7(ccl::toLatin1(m_storyPart7->toPlainText()));
    if (m_cbStoryPart8->isChecked())
        settings->set_progressMsg_8(ccl::toLatin1(m_storyPart8->toPlainText()));
    if (m_cbStoryPart9->isChecked())
        settings->set_progressMsg_9(ccl::toLatin1(m_storyPart9->toPlainText()));
    if (m_cbStoryPart10->isChecked())
        settings->set_progressMsg_10(ccl::toLatin1(m_storyPart10->toPlainText()));
    if (m_cbStoryEnd1->isChecked())
        settings->set_endgameMsg_1(ccl::toLatin1(m_storyEnd1->toPlainText()));
    if (m_cbStoryEnd2->isChecked())
        settings->set_endgameMsg_2(ccl::toLatin1(m_storyEnd2->toPlainText()));
}
