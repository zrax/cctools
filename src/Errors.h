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

}

#endif
