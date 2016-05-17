#include "avm/string.hpp"

#include <cstdlib>
#include <cmath>

NS_AVM_BEGIN

String::String(const char* bytes, int32_t n)
{
    m_length = n;
    m_bytes = new char[n];
    memcpy((void*)m_bytes, bytes, n);
}

String::~String()
{
    delete[] m_bytes;
}

std::string String::to_string() const
{
    return m_bytes;
}

double String::to_number() const
{
    return string_to_number(m_bytes);
}

static bool is_white(int c)
{
    return c == 0x9 || c == 0xB || c == 0xC || c == 0x20 || c == 0xA0 || c == 0xFEFF;
}

static bool is_newline(int c)
{
    return c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

double String::string_to_number(const char* s)
{
    // skip prefix and suffix white space and newline indicators
    while(is_white(*s) || is_newline(*s)) ++s;

    double n;
    char* endptr = nullptr;
    if( s[0] == '0' && (s[1] == 'x' || s[1] == 'X') && s[2] != 0 )
        n = strtol(s+2, &endptr, 16);
    else if( !strncmp(s, "Infinity", 8) )
        n = INFINITY, endptr = (char*)s + 8;
    else if( !strncmp(s, "+Infinity", 9) )
        n = INFINITY, endptr = (char*)s + 9;
    else if( !strncmp(s, "-Infinity", 9) )
        n = -INFINITY, endptr = (char*)s + 9;
    else
        n = strtod(s, &endptr);

    while(is_white(*endptr) || is_newline(*endptr)) ++endptr;
    if( *endptr ) return NAN;

    return n;
}

const char* String::number_to_string(char buf[32], double f)
{
    if( std::isnan(f) ) return "NaN";
    if( std::isinf(f) ) return f < 0 ? "-Infinity" : "Infinity";
    if( f == 0 ) return "0";

    snprintf((char*)buf, sizeof(char)*32, "%g", f);
    return buf;
}

NS_AVM_END