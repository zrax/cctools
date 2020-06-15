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

#ifndef _ERRORS_H
#define _ERRORS_H

#include <stdexcept>
#include <string>

namespace ccl {

class Exception : public std::runtime_error {
public:
    explicit Exception(const char* msg) : std::runtime_error(msg) { }
};

class IOException : public ccl::Exception {
public:
    explicit IOException(const char* msg) : ccl::Exception(msg) { }
};

class FormatException : public ccl::Exception {
public:
    explicit FormatException(const char* msg) : ccl::Exception(msg) { }
};

}

#endif
