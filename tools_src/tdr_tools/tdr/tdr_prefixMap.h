#ifndef TTRANS_PREFIX_MAP_H
#define TTRANS_PREFIX_MAP_H

#include <map>
#include <string>

using std::map;
using std::string;

class TTrans_prefixMap
{
    public:
        static TTrans_prefixMap* getInstance()
        {
            if (!TTrans_prefixMap::instance)
                TTrans_prefixMap::instance = new TTrans_prefixMap();

            return TTrans_prefixMap::instance;
        }
        const string getSinglePrefix(const string type);
        const string getArrayPrefix(const string type);
    private:
        TTrans_prefixMap();
        map<string, string> sprefixes;
        map<string, string> mprefixes;
        static TTrans_prefixMap* instance;
};

#endif
