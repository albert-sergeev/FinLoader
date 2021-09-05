#ifndef TRIMUTILS_H
#define TRIMUTILS_H

#include <string>

//--------------------------------------------------------------------------------------------------------
// trimming functions
static inline std::string & ltrim(std::string &s){
    s.erase(begin(s),std::find_if(s.begin(),s.end(),[](const unsigned char t){
        return !std::isspace(t);
    }));
    return s;
}
//
static inline std::string & ltrim(std::string &&s){ return  ltrim(s);}
//
static inline std::string & rtrim(std::string &s){
    s.erase(std::find_if(s.rbegin(),s.rend(),[](unsigned char ch){
        return !std::isspace(ch);
    }).base(),s.end()
            );
    return s;
}
//
static inline std::string & rtrim(std::string &&s){ return  rtrim(s);}
//
static inline std::string & trim(std::string &s){
    ltrim(s);
    rtrim(s);
    return s;
}
//
static inline std::string & trim(std::string &&s){ return  trim(s);}
//
//static inline std::string ltrim_copy(std::string s) {
//    ltrim(s);
//    return s;
//}

//static inline std::string rtrim_copy(std::string s) {
//    rtrim(s);
//    return s;
//}

//static inline std::string trim_copy(std::string s) {
//    trim(s);
//    return s;
//}


#endif // TRIMUTILS_H
