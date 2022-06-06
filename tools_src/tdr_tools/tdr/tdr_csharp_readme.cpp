#include "tdr_csharp.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

const char*
TTransCSharp::readme[] = {
    "Some explanation about TDR feature that generates C# source files:",
    "",
    "1,  For each .xml file, TDR generates one .cs source file",
    "",
    "2,  If .xml file contains <struct></struct> or <union></union>,",
    "    TDR generates one .cs source file for this .xml file.",
    "",
    "3,  All source files refered by 1 and 2 are protected by a namespace,",
    "    with the same name as value of 'name' attribute of xml tag <metalib></metalib>.",
    "",
    "4,  For the same release of TDR, all assistant source files(Tdr*.cs) are independent of xml files.",
    "    That means these assistant source files can be shared by many metalibs.",
    "    For the same reason, one project MUST contains only one set of assistant source files.",
    "    Otherwise, linker will complains like 'symbol redefine'.",
    "",
    "5,  In order to make the feature easy to use,",
    "    'visualize' and 'defaultvalue' for 'wstring' are not supported.",
    "    Otherwise, some third party library is needed.",
    "",
    "6,  In order to reduce the difficulty , two tdr-data types -'pointer' and 'refer' are not supported.",
    "",
    "For more help information, please refer to URL:",
    "    http://ied.oa.com/tsf4g/tsf4gonline/lib/tdr.htm",

};

void
TTransCSharp::makeReadMe()
{
    list<string> lines;

    loadCodeRes(0, lines, readme, sizeof(TTransCSharp::readme)/sizeof(TTransCSharp::readme[0]));

    this->output(lines, "README.txt", false);
}

