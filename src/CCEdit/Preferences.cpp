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

#include "Preferences.h"

#include <QSettings>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));

    QSettings settings;

    m_useDefaultAuthor = new QCheckBox(tr("Use default &author name: "));
    m_useDefaultAuthor->setChecked(settings.value(QStringLiteral("UseDefaultAuthor"), false).toBool());

    m_authorName = new QLineEdit(settings.value(QStringLiteral("AuthorName")).toString(), this);
    m_authorName->setEnabled(m_useDefaultAuthor->isChecked());

    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setVerticalSpacing(4);
    layout->setHorizontalSpacing(4);
    int row = 0;

    layout->addWidget(m_useDefaultAuthor, row, 0);
    layout->addWidget(m_authorName, row, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding),
                    ++row, 0, 1, 3);
    layout->addWidget(buttons, ++row, 0, 1, 3);
    resize(400, sizeHint().height());

    connect(m_useDefaultAuthor, &QCheckBox::toggled, this, &PreferencesDialog::onUseDefaultAuthorChanged);

    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &PreferencesDialog::onSaveSettings);
}

void PreferencesDialog::onSaveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("UseDefaultAuthor"), m_useDefaultAuthor->isChecked());
    settings.setValue(QStringLiteral("AuthorName"), m_authorName->text());
    accept();
}

void PreferencesDialog::onUseDefaultAuthorChanged() {
    m_authorName->setEnabled(m_useDefaultAuthor->isChecked());
}
