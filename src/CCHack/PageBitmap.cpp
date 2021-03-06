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
    : HackPage(parent), m_which(which)
{
    m_stateLabel = new QLabel(this);

    m_exportButton = new QPushButton(tr("&Export..."), this);
    m_exportButton->setEnabled(false);
    m_importButton = new QPushButton(tr("&Import..."), this);
    m_resetButton = new QPushButton(tr("&Reset"), this);

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
    layout->addWidget(m_resetButton, 0, 4);
    layout->addWidget(scroll, 1, 0, 1, 5);

    connect(m_exportButton, &QPushButton::pressed, this, &PageBitmap::onExport);
    connect(m_importButton, &QPushButton::pressed, this, &PageBitmap::onImport);
    connect(m_resetButton, &QPushButton::pressed, this, [this] {
        m_bitmap = QByteArray();
        updateGraphic();
    });
}

struct BitmapCoreHeader
{
    uint32_t bcSize;
    uint16_t bcWidth;
    uint16_t bcHeight;
    uint16_t bcPlanes;
    uint16_t bcBitCount;

    BitmapCoreHeader()
        : bcSize(), bcWidth(), bcHeight(), bcPlanes(), bcBitCount()
    { }
};
static_assert(sizeof(BitmapCoreHeader) == 12,
              "Invalid compiler packing for BitmapCoreHeader");

struct BitmapInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

    BitmapInfoHeader()
        : biSize(), biWidth(), biHeight(), biPlanes(), biBitCount(),
          biCompression(), biSizeImage(), biXPelsPerMeter(), biYPelsPerMeter(),
          biClrUsed(), biClrImportant()
    { }
};
static_assert(sizeof(BitmapInfoHeader) == 40,
              "Invalid compiler packing for BitmapInfoHeader");

static uint32_t bitmapDataSize(const BitmapInfoHeader& bmih)
{
    if (bmih.biCompression > 0)
        return bmih.biSizeImage;

    uint32_t stride = (bmih.biWidth * bmih.biBitCount + 7) / 8;
    uint32_t padding = 4 - (stride % 4);
    if (padding != 4)
        stride += padding;
    return std::abs(bmih.biHeight) * stride;
}

void CCHack::PageBitmap::setValues(HackSettings* settings)
{
    QByteArray bmpData;
    switch (m_which) {
    case VgaTileset:
        bmpData = settings->get_vgaTileset();
        break;
    case EgaTileset:
        bmpData = settings->get_egaTileset();
        break;
    case MonoTileset:
        bmpData = settings->get_monoTileset();
        break;
    case Background:
        bmpData = settings->get_background();
        break;
    case Digits:
        bmpData = settings->get_digits();
        break;
    case InfoBox:
        bmpData = settings->get_infoBox();
        break;
    case ChipEnd:
        bmpData = settings->get_chipEnd();
        break;
    default:
        Q_UNREACHABLE();
    }

    if (bmpData.isEmpty()) {
        m_bitmap = QByteArray();
    } else {
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);
        buffer.write("BM", 2);

        BitmapInfoHeader bmih;
        uint32_t paletteSize = 0;
        memcpy(&bmih.biSize, bmpData.constData(), sizeof(bmih.biSize));
        if (bmih.biSize >= 40 && bmih.biSize != 64) {
            memcpy(&bmih, bmpData.constData(), sizeof(bmih));
            if (bmih.biBitCount <= 8) {
                memcpy(&paletteSize, bmpData.constData() + 32, sizeof(paletteSize));
                if (paletteSize == 0)
                    paletteSize = 1u << bmih.biBitCount;
            }
        } else {
            BitmapCoreHeader bmch;
            memcpy(&bmch, bmpData.constData(), sizeof(bmch));
            if (bmch.bcBitCount <= 8)
                paletteSize = 1u << bmch.bcBitCount;

            bmih.biWidth = bmch.bcWidth;
            bmih.biHeight = bmch.bcHeight;
            bmih.biPlanes = bmch.bcPlanes;
            bmih.biBitCount = bmch.bcBitCount;
        }
        uint32_t ofImageData = BITMAPFILEHEADER_SIZE + bmih.biSize + (paletteSize * 4);
        uint32_t cbDataSize = bitmapDataSize(bmih);
        if (cbDataSize == 0)
            cbDataSize = bmpData.size();

        uint32_t cbFileSize = ofImageData + cbDataSize;
        buffer.write((const char *)&cbFileSize, sizeof(cbFileSize));
        uint16_t reserved[2] = { 0, 0 };
        buffer.write((const char *)&reserved, sizeof(reserved));
        buffer.write((const char *)&ofImageData, sizeof(ofImageData));
        buffer.write(bmpData.left(int(cbFileSize - BITMAPFILEHEADER_SIZE)));
        m_bitmap = buffer.data();
    }

    updateGraphic();
}

void CCHack::PageBitmap::saveTo(HackSettings* settings)
{
    if (m_image.isNull())
        return;

    // Strip off the bitmap file header
    QByteArray bmp = m_bitmap.mid(BITMAPFILEHEADER_SIZE);

    switch (m_which) {
    case VgaTileset:
        settings->set_vgaTileset(bmp);
        break;
    case EgaTileset:
        settings->set_egaTileset(bmp);
        break;
    case MonoTileset:
        settings->set_monoTileset(bmp);
        break;
    case Background:
        settings->set_background(bmp);
        break;
    case Digits:
        // TODO: We already convert any imported bitmap to 8-bit indexed,
        // but we should probably do another sanity check here to make sure
        // what we write to the executable is still valid...
        settings->set_digits(bmp);
        break;
    case InfoBox:
        settings->set_infoBox(bmp);
        break;
    case ChipEnd:
        settings->set_chipEnd(bmp);
        break;
    default:
        Q_UNREACHABLE();
    }
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

    // Cache the bitmap data and update the UI
    QBuffer bmpBuffer;
    bmpBuffer.open(QIODevice::ReadWrite);
    bmp.save(&bmpBuffer, "BMP");
    bmpBuffer.seek(0);
    m_bitmap = bmpBuffer.readAll();

    updateGraphic();
}

void CCHack::PageBitmap::updateGraphic()
{
    if (m_bitmap.isEmpty()) {
        m_image = QImage();
        m_stateLabel->setText(tr("No Graphic Loaded"));
        m_exportButton->setEnabled(false);
    } else {
        m_image.loadFromData(m_bitmap, "BMP");
        m_stateLabel->setText(tr("Stored Bitmap"));
        m_exportButton->setEnabled(true);
    }

    m_preview->setPixmap(QPixmap::fromImage(m_image));
    m_preview->resize(m_image.size());
}
