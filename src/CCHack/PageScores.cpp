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

#include "PageScores.h"
#include "CommonWidgets/CCTools.h"

#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

CCHack::PageScores::PageScores(QWidget* parent)
    : HackPage(parent)
{
    auto scroll = new QScrollArea(this);
    auto content = new QWidget(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_cbFirstTry = new QCheckBox(tr("First Try:"), this);
    m_firstTry = new QLineEdit(this);
    m_firstTry->setEnabled(false);
    m_firstTry->setMaxLength(18);
    m_defFirstTry = new QLineEdit(this);
    m_defFirstTry->setEnabled(false);

    m_cbThirdTry = new QCheckBox(tr("2nd-3rd Try:"), this);
    m_thirdTry = new QLineEdit(this);
    m_thirdTry->setEnabled(false);
    m_thirdTry->setMaxLength(14);
    m_defThirdTry = new QLineEdit(this);
    m_defThirdTry->setEnabled(false);

    m_cbFifthTry = new QCheckBox(tr("4th-5th Try:"), this);
    m_fifthTry = new QLineEdit(this);
    m_fifthTry->setEnabled(false);
    m_fifthTry->setMaxLength(20);
    m_defFifthTry = new QLineEdit(this);
    m_defFifthTry->setEnabled(false);

    m_cbFinalTry = new QCheckBox(tr("Further Try:"), this);
    m_finalTry = new QLineEdit(this);
    m_finalTry->setEnabled(false);
    m_finalTry->setMaxLength(20);
    m_defFinalTry = new QLineEdit(this);
    m_defFinalTry->setEnabled(false);

    m_cbEstTimeRecord = new QCheckBox(tr("New Time Record:"), this);
    m_estTimeRecord = new QLineEdit(this);
    m_estTimeRecord->setEnabled(false);
    m_estTimeRecord->setMaxLength(50);
    m_defEstTimeRecord = new QLineEdit(this);
    m_defEstTimeRecord->setEnabled(false);

    m_cbBeatTimeRecord = new QCheckBox(tr("Beat Time Record:"), this);
    m_beatTimeRecord = new QLineEdit(this);
    m_beatTimeRecord->setEnabled(false);
    m_beatTimeRecord->setMaxLength(49);
    m_defBeatTimeRecord = new QLineEdit(this);
    m_defBeatTimeRecord->setEnabled(false);

    m_cbIncreasedScore = new QCheckBox(tr("Improved Score:"), this);
    m_increasedScore = new QLineEdit(this);
    m_increasedScore->setEnabled(false);
    m_increasedScore->setMaxLength(54);
    m_defIncreasedScore = new QLineEdit(this);
    m_defIncreasedScore->setEnabled(false);

    m_cbEndgameScore = new QCheckBox(tr("Endgame Score:"), this);
    m_endgameScore = new LLTextEdit(this);
    m_endgameScore->setEnabled(false);
    m_endgameScore->setMaxLength(381);
    m_defEndgameScore = new QPlainTextEdit(this);
    m_defEndgameScore->setEnabled(false);

    auto layout = new QGridLayout(content);
    int row = 0;
    layout->addWidget(new QLabel(tr("Override"), this), row, 1);
    layout->addWidget(new QLabel(tr("Default"), this), row, 2);
    layout->addWidget(m_cbFirstTry, ++row, 0);
    layout->addWidget(m_firstTry, row, 1);
    layout->addWidget(m_defFirstTry, row, 2);
    layout->addWidget(m_cbThirdTry, ++row, 0);
    layout->addWidget(m_thirdTry, row, 1);
    layout->addWidget(m_defThirdTry, row, 2);
    layout->addWidget(m_cbFifthTry, ++row, 0);
    layout->addWidget(m_fifthTry, row, 1);
    layout->addWidget(m_defFifthTry, row, 2);
    layout->addWidget(m_cbFinalTry, ++row, 0);
    layout->addWidget(m_finalTry, row, 1);
    layout->addWidget(m_defFinalTry, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(m_cbEstTimeRecord, ++row, 0);
    layout->addWidget(m_estTimeRecord, row, 1);
    layout->addWidget(m_defEstTimeRecord, row, 2);
    layout->addWidget(m_cbBeatTimeRecord, ++row, 0);
    layout->addWidget(m_beatTimeRecord, row, 1);
    layout->addWidget(m_defBeatTimeRecord, row, 2);
    layout->addWidget(m_cbIncreasedScore, ++row, 0);
    layout->addWidget(m_increasedScore, row, 1);
    layout->addWidget(m_defIncreasedScore, row, 2);
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Maximum, QSizePolicy::Fixed), ++row, 0, 1, 3);
    layout->addWidget(m_cbEndgameScore, ++row, 0);
    layout->addWidget(m_endgameScore, row, 1);
    layout->addWidget(m_defEndgameScore, row, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), ++row, 0, 1, 3);

    auto topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(scroll);

    connect(m_cbFirstTry, &QCheckBox::toggled, m_firstTry, &QWidget::setEnabled);
    connect(m_cbThirdTry, &QCheckBox::toggled, m_thirdTry, &QWidget::setEnabled);
    connect(m_cbFifthTry, &QCheckBox::toggled, m_fifthTry, &QWidget::setEnabled);
    connect(m_cbFinalTry, &QCheckBox::toggled, m_finalTry, &QWidget::setEnabled);
    connect(m_cbEstTimeRecord, &QCheckBox::toggled, m_estTimeRecord, &QWidget::setEnabled);
    connect(m_cbBeatTimeRecord, &QCheckBox::toggled, m_beatTimeRecord, &QWidget::setEnabled);
    connect(m_cbIncreasedScore, &QCheckBox::toggled, m_increasedScore, &QWidget::setEnabled);
    connect(m_cbEndgameScore, &QCheckBox::toggled, m_endgameScore, &QWidget::setEnabled);
}

void CCHack::PageScores::setValues(HackSettings* settings)
{
    m_cbFirstTry->setChecked(settings->have_firstTryMsg());
    m_firstTry->setText(ccl::fromLatin1(settings->get_firstTryMsg()));
    m_cbThirdTry->setChecked(settings->have_thirdTryMsg());
    m_thirdTry->setText(ccl::fromLatin1(settings->get_thirdTryMsg()));
    m_cbFifthTry->setChecked(settings->have_fifthTryMsg());
    m_fifthTry->setText(ccl::fromLatin1(settings->get_fifthTryMsg()));
    m_cbFinalTry->setChecked(settings->have_finalTryMsg());
    m_finalTry->setText(ccl::fromLatin1(settings->get_finalTryMsg()));
    m_cbEstTimeRecord->setChecked(settings->have_estTimeRecordMsg());
    m_estTimeRecord->setText(ccl::fromLatin1(settings->get_estTimeRecordMsg()));
    m_cbBeatTimeRecord->setChecked(settings->have_beatTimeRecordMsg());
    m_beatTimeRecord->setText(ccl::fromLatin1(settings->get_beatTimeRecordMsg()));
    m_cbIncreasedScore->setChecked(settings->have_increasedScoreMsg());
    m_increasedScore->setText(ccl::fromLatin1(settings->get_increasedScoreMsg()));
    m_cbEndgameScore->setChecked(settings->have_endgameScoreMsg());
    m_endgameScore->setPlainText(ccl::fromLatin1(settings->get_endgameScoreMsg()));
}

void CCHack::PageScores::setDefaults(HackSettings* settings)
{
    m_defFirstTry->setText(ccl::fromLatin1(settings->get_firstTryMsg()));
    m_defFirstTry->setCursorPosition(0);
    m_defThirdTry->setText(ccl::fromLatin1(settings->get_thirdTryMsg()));
    m_defThirdTry->setCursorPosition(0);
    m_defFifthTry->setText(ccl::fromLatin1(settings->get_fifthTryMsg()));
    m_defFifthTry->setCursorPosition(0);
    m_defFinalTry->setText(ccl::fromLatin1(settings->get_finalTryMsg()));
    m_defFinalTry->setCursorPosition(0);
    m_defEstTimeRecord->setText(ccl::fromLatin1(settings->get_estTimeRecordMsg()));
    m_defEstTimeRecord->setCursorPosition(0);
    m_defBeatTimeRecord->setText(ccl::fromLatin1(settings->get_beatTimeRecordMsg()));
    m_defBeatTimeRecord->setCursorPosition(0);
    m_defIncreasedScore->setText(ccl::fromLatin1(settings->get_increasedScoreMsg()));
    m_defIncreasedScore->setCursorPosition(0);
    m_defEndgameScore->setPlainText(ccl::fromLatin1(settings->get_endgameScoreMsg()));
}

void CCHack::PageScores::saveTo(HackSettings* settings)
{
    if (m_cbFirstTry->isChecked())
        settings->set_firstTryMsg(ccl::toLatin1(m_firstTry->text()));
    if (m_cbThirdTry->isChecked())
        settings->set_thirdTryMsg(ccl::toLatin1(m_thirdTry->text()));
    if (m_cbFifthTry->isChecked())
        settings->set_fifthTryMsg(ccl::toLatin1(m_fifthTry->text()));
    if (m_cbFinalTry->isChecked())
        settings->set_finalTryMsg(ccl::toLatin1(m_finalTry->text()));
    if (m_cbEstTimeRecord->isChecked())
        settings->set_estTimeRecordMsg(ccl::toLatin1(m_estTimeRecord->text()));
    if (m_cbBeatTimeRecord->isChecked())
        settings->set_beatTimeRecordMsg(ccl::toLatin1(m_beatTimeRecord->text()));
    if (m_cbIncreasedScore->isChecked())
        settings->set_increasedScoreMsg(ccl::toLatin1(m_increasedScore->text()));
    if (m_cbEndgameScore->isChecked())
        settings->set_endgameScoreMsg(ccl::toLatin1(m_endgameScore->toPlainText()));
}
