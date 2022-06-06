#!/bin/bash
##============================================================================
## @Id:       $Id: tdr_xml.sh 15031 2010-11-22 01:00:30Z flyma $
## @Author:   $Author: flyma $
## @Date:     $Date: 2010-11-22 09:00:30 +0800 (星期一, 2010-11-22) $
## @Revision: $Revision: 15031 $
## @HeadURL:  $HeadURL: https://tc-svn.tencent.com/ied/ied_tsf4g_rep/tsf4g_proj/trunk/lib_src/tmng/tdr_xml.sh $
##----------------------------------------------------------------------------


cd `dirname $0`/xml

TDR= ../../tools/tdr

[ "X$1" == "X-h" ] && {
    echo "Usage: $0 [-r]"
    exit 1
}

[ "X$1" == "X-r" ] && {
    rm -f *.{c,h,tdr}

} || {
    for i in `ls *.xml`
    do
        $TDR --xml2dr -o ${i%xml}tdr $i
        $TDR --xml2h  $i
        $TDR --xml2c  -o ${i%xml}c $i
    done
    echo
    ls -l *.{c,h,tdr}
}

exit 0


##----------------------------------------------------------------------------
## THE END
##============================================================================
