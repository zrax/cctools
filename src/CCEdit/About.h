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

#ifndef _ABOUT_CCEDIT_H
#define _ABOUT_CCEDIT_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QDialogButtonBox>

class AboutDialog : public QDialog {
public:
    AboutDialog()
    {
        // Inline because it's small
        setWindowTitle(tr("About CCEdit"));
        setWindowIcon(QIcon(":/res/help-about.png"));

        QLabel* lblIcon = new QLabel(this);
        lblIcon->setPixmap(QPixmap(":/icons/boot-48.png"));
        lblIcon->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

        QLabel* lblLicense = new QLabel(this);
        lblLicense->setText(tr("\
CCTools is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>."));

        QDialogButtonBox* buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok, Qt::Horizontal, this);

        QGridLayout* layout = new QGridLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setVerticalSpacing(8);
        layout->setHorizontalSpacing(8);
        layout->addWidget(lblIcon, 0, 0, 3, 1);
        layout->addWidget(new QLabel("CCEdit 1.99.0", this), 0, 1);
        layout->addWidget(new QLabel("Part of CCTools 2.0 BETA", this), 1, 1);
        layout->addWidget(new QLabel(tr("Copyright (C) 2010  Michael Hansen"), this), 2, 1);
        layout->addWidget(lblLicense, 4, 0, 1, 2);
        layout->addWidget(buttons, 6, 0, 1, 2);

        connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    }
};

#endif
