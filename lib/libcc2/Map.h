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

#ifndef _CC2MAP_H
#define _CC2MAP_H

#include "libcc1/Stream.h"

namespace cc2 {

class MapOption {
public:
    enum Viewport {
        View10x10,
        View9x9,
        ViewSplit,
    };

    enum BlobPattern {
        BlobsDeterministic,
        Blobs4Pattern,
        BlobsExtraRandom,
    };

    MapOption();

    Viewport view() const { return m_view; }
    BlobPattern blobPattern() const { return m_blobPattern; }
    uint16_t timeLimit() const { return m_timeLimit; }
    const uint8_t* solutionMD5() const { return m_solutionMD5; }
    bool solutionValid() const { return m_solutionValid; }
    bool hidden() const { return m_hidden; }
    bool readOnly() const { return m_readOnly; }
    bool hideLogic() const { return m_hideLogic; }
    bool cc1Boots() const { return m_cc1Boots; }

    void setView(Viewport view) { m_view = view; }
    void setBlobPattern(BlobPattern pattern) { m_blobPattern = pattern; }
    void setTimeLimit(uint16_t limit) { m_timeLimit = limit; }
    void setSolutionMD5(const uint8_t* md5);
    void setSolutionValid(bool valid) { m_solutionValid = valid; }
    void setHidden(bool hidden) { m_hidden = hidden; }
    void setReadOnly(bool ro) { m_readOnly = ro; }
    void setHideLogic(bool hide) { m_hideLogic = hide; }
    void setCc1Boots(bool cc1Boots) { m_cc1Boots = cc1Boots; }

    void read(ccl::Stream* stream, long size);
    long write(ccl::Stream* stream) const;

private:
    Viewport m_view;
    BlobPattern m_blobPattern;
    uint16_t m_timeLimit;
    uint8_t m_solutionMD5[16];
    bool m_solutionValid;
    bool m_hidden;
    bool m_readOnly;
    bool m_hideLogic;
    bool m_cc1Boots;
};

class MapData {
public:
    MapData();
    ~MapData();

    void read(ccl::Stream* stream, long size, bool packed);
    long write(ccl::Stream* stream, bool pack) const;

private:
    // TODO
};

class ReplayData {
public:
    ReplayData();
    ~ReplayData();

    void read(ccl::Stream* stream, long size, bool packed);
    long write(ccl::Stream* stream, bool pack) const;

private:
    // TODO
};

class Map {
public:
    Map() { }

    void read(ccl::Stream* stream);
    void write(ccl::Stream* stream) const;

private:
    std::string m_version;
    std::string m_lock;
    std::string m_title;
    std::string m_author;
    std::string m_editorVersion;
    std::string m_clue;
    std::string m_note;
    MapOption m_option;
    MapData m_mapData;
    uint8_t m_key[16];
    ReplayData m_replay;
    bool m_readOnly;
};

}

#endif // _CC2MAP_H
