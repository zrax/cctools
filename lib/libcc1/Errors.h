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
#include <QCoreApplication>

namespace ccl {

class RuntimeError : public std::exception {
    Q_DECLARE_TR_FUNCTIONS(ccl::Exception);

public:
    explicit RuntimeError(QString msg) noexcept : m_message(std::move(msg)) { }

    const char* what() const noexcept override { return "ccl::RuntimeError"; }
    const QString& message() const noexcept { return m_message; }

private:
    QString m_message;
};

class IOError : public RuntimeError {
public:
    explicit IOError(QString msg) noexcept : RuntimeError(std::move(msg)) { }

    const char* what() const noexcept override { return "ccl::IOError"; }
};

class FormatError : public RuntimeError {
public:
    explicit FormatError(QString msg) noexcept : RuntimeError(std::move(msg)) { }

    const char* what() const noexcept override { return "ccl::FormatError"; }
};

}

#endif
