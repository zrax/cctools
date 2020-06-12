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

#ifndef _ABOUT_CCHACK_H
#define _ABOUT_CCHACK_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QDialogButtonBox>

class AboutDialog : public QDialog {
public:
    explicit AboutDialog(QWidget* parent = nullptr) : QDialog(parent)
    {
        // Inline because it's small
        setWindowTitle(tr("About CCHack"));
        setWindowIcon(QIcon(":/res/help-about.png"));

        auto lblIcon = new QLabel(this);
        lblIcon->setPixmap(QPixmap(":/icons/sock-48.png"));
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

        auto buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok, Qt::Horizontal, this);

        auto layout = new QGridLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setVerticalSpacing(8);
        layout->setHorizontalSpacing(8);
        layout->addWidget(lblIcon, 0, 0, 3, 1);
        layout->addWidget(new QLabel("CCHack 2.0.95", this), 0, 1);
        layout->addWidget(new QLabel("Part of CCTools 2.1", this), 1, 1);
        layout->addWidget(new QLabel(tr("Copyright (C) 2020  Michael Hansen"), this), 2, 1);
        layout->addWidget(lblLicense, 4, 0, 1, 2);
        layout->addWidget(buttons, 6, 0, 1, 2);

        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    }
};

#endif
