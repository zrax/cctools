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

#ifndef _PAGE_BITMAP_H
#define _PAGE_BITMAP_H

#include "HackSettings.h"

class QLabel;
class QPushButton;

namespace CCHack {

class PageBitmap : public HackPage {
    Q_OBJECT

public:
    enum BitmapType {
        VgaTileset, EgaTileset, MonoTileset, Background, Digits,
        InfoBox, ChipEnd,
    };

    PageBitmap(int which, QWidget* parent);
    void setValues(HackSettings* settings) override;
    void setDefaults(HackSettings*) override { }

private slots:
    void onExport();
    void onImport();
    void onRevert();

private:
    int m_which;
    bool m_modified;
    QLabel* m_stateLabel;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    QPushButton* m_revertButton;
    QLabel* m_preview;
    QByteArray m_bitmap;
    QPixmap m_pixmap;

    void writeBitmap(QIODevice* stream);
};

}

#endif
