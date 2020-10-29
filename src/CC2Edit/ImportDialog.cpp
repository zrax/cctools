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

#include "ImportDialog.h"
#include "libcc1/Stream.h"
#include "libcc1/Levelset.h"
#include "libcc2/Map.h"

#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QSettings>

ImportDialog::ImportDialog(QWidget* parent)
    : QDialog(parent), m_levelset()
{
    setWindowTitle(tr("Import CC1 Map"));

    QSettings settings;

    auto levelLabel = new QLabel(tr("&Level:"), this);
    m_levelSelect = new QComboBox(this);
    levelLabel->setBuddy(m_levelSelect);

    m_resizeLevel = new QCheckBox(tr("&Resize level to fit non-blank area"), this);
    m_resizeLevel->setChecked(settings.value("Import/AutoResize", false).toBool());

    auto warningLabel = new QLabel(tr(
            "Warning:  Certain Level features, such as non-standard button "
            "connections, custom monster move order, and invalid tile "
            "combinations are not supported by CC2 and will be discarded "
            "in the imported map."), this);
    warningLabel->setWordWrap(true);

    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;
    layout->addWidget(levelLabel, ++row, 0);
    layout->addWidget(m_levelSelect, row, 1);
    layout->addWidget(m_resizeLevel, ++row, 1);
    layout->addWidget(warningLabel, ++row, 0, 1, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 2);
    layout->addWidget(buttons, ++row, 0, 1, 2);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

ImportDialog::~ImportDialog()
{
    QSettings settings;
    settings.setValue("Import/AutoResize", m_resizeLevel->isChecked());

    delete m_levelset;
}

bool ImportDialog::loadLevelset(const QString& filename)
{
    ccl::FileStream fs;
    if (!fs.open(filename.toLocal8Bit().constData(), "rb")) {
        QMessageBox::critical(this, tr("Error importing levelset"),
                tr("Could not open '%1' for reading").arg(filename));
        return false;
    }

    m_levelset = new ccl::Levelset;
    try {
        m_levelset->read(&fs);
    } catch (const ccl::RuntimeError& err) {
        QMessageBox::critical(this, tr("Error importing levelset"),
                tr("Failed to load '%1': %2").arg(filename).arg(err.message()));
        return false;
    }

    for (int i = 0; i < m_levelset->levelCount(); ++i) {
        const ccl::LevelData* level = m_levelset->level(i);
        m_levelSelect->addItem(QStringLiteral("%1 - %2")
                .arg(i + 1, 3, 10, QLatin1Char('0'))
                .arg(QString::fromLatin1(level->name().c_str())));
    }

    return true;
}

cc2::Map* ImportDialog::importMap(int* levelNum)
{
    if (m_levelSelect->count() == 0)
        return nullptr;

    if (levelNum)
        *levelNum = m_levelSelect->currentIndex();
    const ccl::LevelData* level = m_levelset->level(m_levelSelect->currentIndex());
    auto map = new cc2::Map;
    map->importFrom(level, m_resizeLevel->isChecked());
    return map;
}
