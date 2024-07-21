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

#include "CCTools.h"

#include <QIcon>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSettings>
#include <QFileInfo>

AboutWidget::AboutWidget(const QString& name, const QPixmap& icon, QWidget *parent)
    : QWidget(parent)
{
    auto lblIcon = new QLabel(this);
    lblIcon->setPixmap(icon);
    lblIcon->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    auto lblLicense = new QLabel(this);
    lblLicense->setTextFormat(Qt::RichText);
    lblLicense->setOpenExternalLinks(true);
    lblLicense->setWordWrap(true);
    lblLicense->setText(tr(
        "CCTools is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or "
        "(at your option) any later version.<br />"
        "<br />"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details.<br />"
        "<br />"
        "You should have received a copy of the GNU General Public License "
        "along with this program.  If not, see "
        "&lt;<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>&gt;."));

    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(8);
    layout->addWidget(lblIcon, 0, 0, 3, 1);
    layout->addWidget(new QLabel(name + QStringLiteral(" " CCTOOLS_APP_VER), this), 0, 1);
    layout->addWidget(new QLabel(tr("Part of CCTools %1").arg(QStringLiteral(CCTOOLS_VERSION)), this), 1, 1);
    layout->addWidget(new QLabel(tr("Copyright (C) 2024  Michael Hansen"), this), 2, 1);
    layout->addWidget(lblLicense, 4, 0, 1, 2);
}

AboutDialog::AboutDialog(const QString& name, const QPixmap& icon, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About %1").arg(name));
    setWindowIcon(ICON("help-about"));

    auto aboutWidget = new AboutWidget(name, icon, this);
    auto buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok, Qt::Horizontal, this);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);
    layout->addWidget(aboutWidget);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

#define NUM_RECENT_FILES 10

QStringList recentFiles(QSettings& settings)
{
    QStringList recentList;
    recentList.reserve(NUM_RECENT_FILES);
    for (int i = 0; i < NUM_RECENT_FILES; ++i) {
        const auto key = QStringLiteral("RecentFiles/File_%1").arg(i, 2, 10, QLatin1Char('0'));
        if (settings.contains(key))
            recentList << settings.value(key).toString();
    }
    return recentList;
}

void addRecentFile(QSettings& settings, const QString& filename)
{
    QStringList recent = recentFiles(settings);

    const QString absFilename = QFileInfo(filename).absoluteFilePath();
    auto iter = recent.begin();
    while (iter != recent.end()) {
        if (iter->compare(absFilename, FILE_COMPARE_CS) == 0)
            iter = recent.erase(iter);
        else
            ++iter;
    }
    recent.prepend(absFilename);

    for (int i = 0; i < NUM_RECENT_FILES; ++i) {
        if (i >= recent.size())
            break;
        const auto &filePath = recent.at(i);
        const auto key = QStringLiteral("RecentFiles/File_%1").arg(i, 2, 10, QLatin1Char('0'));
        settings.setValue(key, filePath);
    }
}

void clearRecentFiles(QSettings& settings)
{
    for (int i = 0; i < NUM_RECENT_FILES; ++i) {
        const auto key = QStringLiteral("RecentFiles/File_%1").arg(i, 2, 10, QLatin1Char('0'));
        settings.remove(key);
    }
}
