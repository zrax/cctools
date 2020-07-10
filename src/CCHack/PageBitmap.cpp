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

#include "PageBitmap.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>

CCHack::PageBitmap::PageBitmap(int which, QWidget* parent)
    : HackPage(parent), m_which(which), m_modified()
{
    m_stateLabel = new QLabel(this);

    m_exportButton = new QPushButton(tr("&Export..."), this);
    m_exportButton->setEnabled(false);
    m_importButton = new QPushButton(tr("&Import..."), this);
    m_revertButton = new QPushButton(tr("&Revert"), this);

    m_preview = new QLabel(this);
    m_preview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto scroll = new QScrollArea(this);
    scroll->setWidget(m_preview);
    scroll->setFrameShape(QFrame::StyledPanel);

    auto layout = new QGridLayout(this);
    layout->addWidget(m_stateLabel, 0, 0);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 1);
    layout->addWidget(m_exportButton, 0, 2);
    layout->addWidget(m_importButton, 0, 3);
    layout->addWidget(m_revertButton, 0, 4);
    layout->addWidget(scroll, 1, 0, 1, 5);

    connect(m_exportButton, &QPushButton::pressed, this, &PageBitmap::onExport);
    connect(m_importButton, &QPushButton::pressed, this, &PageBitmap::onImport);
    connect(m_revertButton, &QPushButton::pressed, this, &PageBitmap::onRevert);
}

void CCHack::PageBitmap::setValues(HackSettings* settings)
{
    const QByteArray* bmpData = nullptr;
    switch (m_which) {
    case VgaTileset:
        bmpData = &settings->get_vgaTileset();
        break;
    case EgaTileset:
        bmpData = &settings->get_egaTileset();
        break;
    case MonoTileset:
        bmpData = &settings->get_monoTileset();
        break;
    case Background:
        bmpData = &settings->get_background();
        break;
    case Digits:
        bmpData = &settings->get_digits();
        break;
    case InfoBox:
        bmpData = &settings->get_infoBox();
        break;
    case ChipEnd:
        bmpData = &settings->get_chipEnd();
        break;
    default:
        return;
    }

    if (bmpData->isEmpty()) {
        m_bitmap = QByteArray();
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        buffer.write("BM", 2);

        uint32_t headerSize;
        uint32_t ofImageData;
        memcpy(&headerSize, bmpData->constData(), sizeof(headerSize));
        if (headerSize >= 40 && headerSize != 64) {
            uint16_t bpp;
            memcpy(&bpp, bmpData->constData() + 14, sizeof(bpp));
            if (bpp <= 8) {
                uint32_t paletteSize;
                memcpy(&paletteSize, bmpData->constData() + 32, sizeof(paletteSize));
                if (paletteSize == 0)
                    paletteSize = 1u << bpp;
                ofImageData = 14 + headerSize + (paletteSize * 4);
            } else {
                ofImageData = 14 + headerSize;
            }
        } else {
            uint16_t bpp;
            memcpy(&bpp, bmpData->constData() + 10, sizeof(bpp));
            if (bpp <= 8) {
                uint32_t paletteSize = 1u << bpp;
                ofImageData = 14 + headerSize + (paletteSize * 4);
            } else {
                ofImageData = 14 + headerSize;
            }
        }

        uint32_t cbFileSize = 14 + bmpData->size();
        buffer.write((const char *)&cbFileSize, sizeof(cbFileSize));
        uint16_t reserved[2] = { 0, 0 };
        buffer.write((const char *)&reserved, sizeof(reserved));
        buffer.write((const char *)&ofImageData, sizeof(ofImageData));
        buffer.write(*bmpData);
        m_bitmap = buffer.data();
    }

    // This displays the graphic and updates the UI appropriately
    onRevert();
}

void CCHack::PageBitmap::onExport()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export bitmap"),
                            QString(), tr("Bitmap files (*.bmp)"));
    if (filename.isEmpty())
        return;

    QFile bmpFile(filename);
    if (!bmpFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Export failure"),
                              tr("Could not open %1 for writing").arg(filename));
        return;
    }
    bmpFile.write(m_bitmap);
}

void CCHack::PageBitmap::onImport()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Import bitmap"),
                            QString(), tr("Image files (*.bmp *.jpg *.png);"));
    if (filename.isEmpty())
        return;

    QImage bmp(filename);
    if (bmp.isNull()) {
        QMessageBox::critical(this, tr("Import failure"),
                              tr("Could not load image %1").arg(filename));
        return;
    }

    QSize expectedSize;
    switch (m_which) {
    case VgaTileset:
    case EgaTileset:
    case MonoTileset:
        expectedSize = QSize(416, 512);
        break;
    case Digits:
        expectedSize = QSize(17, 552);
        break;
    case InfoBox:
        expectedSize = QSize(154, 300);
        break;
    default:
        break;
    }
    if (!expectedSize.isEmpty() && bmp.size() != expectedSize) {
        QMessageBox::critical(this, tr("Import failure"),
                tr("The selected image has an incorrect size.\n"
                   "Expected: %1x%2\n"
                   "Provided: %3x%4")
                .arg(expectedSize.width()).arg(expectedSize.height())
                .arg(bmp.width()).arg(bmp.height()));
        return;
    }

    if (m_which == Digits) {
        // The Digits graphic must be 8-bit indexed.  Instead of throwing
        // an error, we just convert it and let the user decide if the
        // result is acceptable.
        bmp = bmp.convertToFormat(QImage::Format_Indexed8);
        if (bmp.isNull()) {
            QMessageBox::critical(this, tr("Import failure"),
                tr("Could not convert %1 to indexed color").arg(filename));
            return;
        }
    }

    m_pixmap = QPixmap::fromImage(std::move(bmp));
    m_stateLabel->setText(tr("From File"));
    m_exportButton->setEnabled(false);

    m_preview->setPixmap(m_pixmap);
    m_preview->resize(m_pixmap.size());
    m_modified = true;
}

void CCHack::PageBitmap::onRevert()
{
    if (m_bitmap.isEmpty()) {
        m_pixmap = QPixmap();
        m_stateLabel->setText(tr("No Graphic Loaded"));
        m_exportButton->setEnabled(false);
    } else {
        m_pixmap.loadFromData(m_bitmap, "BMP");
        m_stateLabel->setText(tr("From Executable"));
        m_exportButton->setEnabled(true);
    }

    m_preview->setPixmap(m_pixmap);
    m_preview->resize(m_pixmap.size());
    m_modified = false;
}
