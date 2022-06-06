#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <exception>
#include "tdr_base.h"
#include "tdr_debug.h"
#include "pal/pal.h"
#include "../../lib_src/tdr/tdr_metalib_kernel_i.h"

#if defined(_WIN32) || defined(_WIN64)
#define	OS_DIRSEP '\\'
#else
#define OS_DIRSEP '/'
#endif

using std::string;
using std::ofstream;
using std::stringstream;
using std::ostream_iterator;

TTransBase::TTransBase(const string& indentStr, const string& outputDir, const string& commentStr)
{
    //    TDEBUG_PRINTLN(indentStr.length());
    //    TDEBUG_PRINTLN(commentStr);
    this->outputDir = outputDir;
    if (OS_DIRSEP != this->outputDir[this->outputDir.length() - 1])
    {
        this->outputDir += OS_DIRSEP;
    }
    this->commentStr = commentStr;
    this->commentsMap[0] = commentStr;
    for (int i = 1; i <= 10; i++)
        this->commentsMap[i] = indentStr + this->commentsMap[i - 1];

    this->indentStr = indentStr;
    this->indentsMap[0] = "";
    for (int j = 1; j <= 10; j++)
        this->indentsMap[j] = this->indentsMap[j - 1] + indentStr;
}

TTransBase::~TTransBase()
{
}

const string&
TTransBase::getIndentStr(unsigned int indentLevel)
{
    // indent by progress
    if (indentLevel > this->indentsMap.size())
    {
        throw string("indent-level error");
    } else if (indentLevel == this->indentsMap.size())
    {
        this->indentsMap[indentLevel] = this->indentsMap[indentLevel - 1] + this->indentStr;
    }

    return this->indentsMap[indentLevel];
}

const string&
TTransBase::getCommentStr(unsigned int indentLevel)
{
    // indent by progress
    if (indentLevel > this->commentsMap.size())
    {
        throw string("indent-level error");
    } else if (indentLevel == this->commentsMap.size())
    {
        this->commentsMap[indentLevel] = this->indentStr + this->commentsMap[indentLevel - 1];
    }

    return this->commentsMap[indentLevel];
}

void
TTransBase::output(const list<string>& lines, const string& file, ios_base::openmode mode)
{
    string outputPath = this->outputDir + file;
    ofstream outfile(outputPath.c_str(), mode);
    if (outfile.fail())
    {
        throw string("failed to open file \"") + outputPath + "\" for write";
    }
    ostream_iterator<string> outIter(outfile, "\n");
	std::copy(lines.begin(), lines.end(), outIter);
    outfile.close();
}
