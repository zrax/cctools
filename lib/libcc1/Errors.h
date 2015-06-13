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

#include <exception>
#include <string>

namespace ccl {

class Exception : public std::exception {
public:
    Exception(const char* msg) : m_msg(msg) { }
    virtual ~Exception() throw() { }

    virtual const char* what() const throw() { return m_msg.c_str(); }

private:
    std::string m_msg;
};

class IOException : public ccl::Exception {
public:
    IOException(const char* msg) : ccl::Exception(msg) { }
};

class FormatException : public ccl::Exception {
public:
    FormatException(const char* msg) : ccl::Exception(msg) { }
};

}

#endif
