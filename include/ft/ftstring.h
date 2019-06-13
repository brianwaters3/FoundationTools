////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftstring_h_included
#define __ftstring_h_included

class FTString : public std::string
{
public:
    FTString& format(cpChar pszFormat, ...);

    operator cpChar() { return c_str(); }
    FTString& operator = (cpChar s) { *(std::string*)this = s; return *this; }
    FTString& operator = (const std::string s) { *(std::string*)this = s; return *this; }
    Int icompare(FTString& str)
    {
        return ft_strnicmp(c_str(), str.c_str(), length() > str.length() ? length() : str.length());
    }
    Int icompare(cpStr str)
    {
        size_t len = strlen(str);
        return ft_strnicmp(c_str(), str, length() > len ? length() : len);
    }
};

#endif // #define __ftstring_h_included
