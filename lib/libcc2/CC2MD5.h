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

#ifndef _CC2MD5_H
#define _CC2MD5_H

// CC2 uses an MD5 algorithm with a non-standard initial state (or perhaps,
// with a pre-seeded initial state whose contents are currently unknown).
// This class lets us start at the desired initial state and also avoids
// pulling in a large crypto library just for MD5.
// Reference: https://gist.github.com/magical/c28c026cf542bc2f7ffba1a334e13f8b

#include <QByteArray>

class CC2MD5
{
public:
    enum InitialState
    {
        StandardMD5 = 0,
        CC2Lock = 1,
    };

    CC2MD5(InitialState state);

    void update(const void* data, size_t size);
    QByteArray finish();

private:
    uint32_t m_context[4];
    uint64_t m_bytes;
    uint32_t m_work[16];

    void md5Transform();
};

#endif // _CC2MD5_H
