#include "tdr_prefixMap.h"
#include "tdr/tdr.h"

TTrans_prefixMap* TTrans_prefixMap::instance = NULL;

TTrans_prefixMap::TTrans_prefixMap()
{
    this->sprefixes[ TDR_TAG_UNION      ] = "st";
    this->mprefixes[ TDR_TAG_UNION      ] = "ast";

    this->sprefixes[ TDR_TAG_STRUCT     ] = "st";
    this->mprefixes[ TDR_TAG_STRUCT     ] = "ast";

    this->sprefixes[ TDR_TAG_TINYINT    ] = "ch";
    this->mprefixes[ TDR_TAG_TINYINT    ] = "sz";

    this->sprefixes[ TDR_TAG_TINYUINT   ] = "b";
    this->mprefixes[ TDR_TAG_TINYUINT   ] = "sz";

    this->sprefixes[ TDR_TAG_SMALLINT   ] = "n";
    this->mprefixes[ TDR_TAG_SMALLINT   ] = "";

    this->sprefixes[ TDR_TAG_SMALLUINT  ] = "w";
    this->mprefixes[ TDR_TAG_SMALLUINT  ] = "";

    this->sprefixes[ TDR_TAG_INT        ] = "i";
    this->mprefixes[ TDR_TAG_INT        ] = "";

    this->sprefixes[ TDR_TAG_UINT       ] = "dw";
    this->mprefixes[ TDR_TAG_UINT       ] = "";

    this->sprefixes[ TDR_TAG_BIGINT     ] = "ll";
    this->mprefixes[ TDR_TAG_BIGINT     ] = "";

    this->sprefixes[ TDR_TAG_BIGUINT    ] = "ull";
    this->mprefixes[ TDR_TAG_BIGUINT    ] = "";

    this->sprefixes[ TDR_TAG_INT8       ] = "ch";
    this->mprefixes[ TDR_TAG_INT8       ] = "sz";

    this->sprefixes[ TDR_TAG_UINT8      ] = "b";
    this->mprefixes[ TDR_TAG_UINT8      ] = "sz";

    this->sprefixes[ TDR_TAG_INT16      ] = "n";
    this->mprefixes[ TDR_TAG_INT16      ] = "";

    this->sprefixes[ TDR_TAG_UINT16     ] = "w";
    this->mprefixes[ TDR_TAG_UINT16     ] = "";

    this->sprefixes[ TDR_TAG_INT32      ] = "i";
    this->mprefixes[ TDR_TAG_INT32      ] = "";

    this->sprefixes[ TDR_TAG_UINT32     ] = "dw";
    this->mprefixes[ TDR_TAG_UINT32     ] = "";

    this->sprefixes[ TDR_TAG_INT64      ] = "ll";
    this->mprefixes[ TDR_TAG_INT64      ] = "";

    this->sprefixes[ TDR_TAG_UINT64     ] = "ull";
    this->mprefixes[ TDR_TAG_UINT64     ] = "";

    this->sprefixes[ TDR_TAG_FLOAT      ] = "f";
    this->mprefixes[ TDR_TAG_FLOAT      ] = "";

    this->sprefixes[ TDR_TAG_DOUBLE     ] = "d";
    this->mprefixes[ TDR_TAG_DOUBLE     ] = "";

    this->sprefixes[ TDR_TAG_DECIMAL    ] = "f";
    this->mprefixes[ TDR_TAG_DECIMAL    ] = "";

    this->sprefixes[ TDR_TAG_DATE       ] = "t";
    this->mprefixes[ TDR_TAG_DATE       ] = "";

    this->sprefixes[ TDR_TAG_TIME       ] = "t";
    this->mprefixes[ TDR_TAG_TIME       ] = "";

    this->sprefixes[ TDR_TAG_DATETIME   ] = "t";
    this->mprefixes[ TDR_TAG_DATETIME   ] = "";

    this->sprefixes[ TDR_TAG_STRING     ] = "sz";
    this->mprefixes[ TDR_TAG_STRING     ] = "asz";

    this->sprefixes[ TDR_TAG_BYTE       ] = "b";
    this->mprefixes[ TDR_TAG_BYTE       ] = "sz";

    this->sprefixes[ TDR_TAG_IP         ] = "ul";
    this->mprefixes[ TDR_TAG_IP         ] = "";

    this->sprefixes[ TDR_TAG_WCHAR      ] = "w";
    this->mprefixes[ TDR_TAG_WCHAR      ] = "";

    this->sprefixes[ TDR_TAG_WSTRING    ] = "sz";
    this->mprefixes[ TDR_TAG_WSTRING    ] = "asz";

    this->sprefixes[ TDR_TAG_VOID       ] = "";
    this->mprefixes[ TDR_TAG_VOID       ] = "";

    this->sprefixes[ TDR_TAG_CHAR       ] = "ch";
    this->mprefixes[ TDR_TAG_CHAR       ] = "sz";

    this->sprefixes[ TDR_TAG_UCHAR      ] = "b";
    this->mprefixes[ TDR_TAG_UCHAR      ] = "sz";

    this->sprefixes[ TDR_TAG_SHORT      ] = "n";
    this->mprefixes[ TDR_TAG_SHORT      ] = "";

    this->sprefixes[ TDR_TAG_USHORT     ] = "w";
    this->mprefixes[ TDR_TAG_USHORT     ] = "";

    this->sprefixes[ TDR_TAG_LONG       ] = "l";
    this->mprefixes[ TDR_TAG_LONG       ] = "";

    this->sprefixes[ TDR_TAG_ULONG      ] = "ul";
    this->mprefixes[ TDR_TAG_ULONG      ] = "";

    this->sprefixes[ TDR_TAG_LONGLONG   ] = "ll";
    this->mprefixes[ TDR_TAG_LONGLONG   ] = "";

    this->sprefixes[ TDR_TAG_ULONGLONG  ] = "ull";
    this->mprefixes[ TDR_TAG_ULONGLONG  ] = "";

    this->sprefixes[ TDR_TAG_MONEY      ] = "m";
    this->mprefixes[ TDR_TAG_MONEY      ] = "";
}

const string
TTrans_prefixMap::getSinglePrefix(const string type)
{
	std::map<string, string>::iterator it;
    it = this->sprefixes.find(type);
    if (it != this->sprefixes.end())
        return this->sprefixes[type];
    else
        throw string("not supported type <") + type + ">";
}

const string
TTrans_prefixMap::getArrayPrefix(const string type)
{
	std::map<string, string>::iterator it;
    it = this->mprefixes.find(type);
    if (it != this->mprefixes.end())
        return this->mprefixes[type];
    else
        throw string("not supported type <") + type + ">";
}
