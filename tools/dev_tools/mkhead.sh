#!/bin/bash

#  @file $RCSfile: mkhead.sh,v $
#  general description of this module
#  $Id: mkhead.sh,v 1.1.1.1 2008-05-28 07:34:59 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008-05-28 07:34:59 $
#  @version $Revision: 1.1.1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

THISFILE=`basename $0`
THISDIR=`dirname $0`
TMPLDIR="mkhead"
DSTDIR="${THISDIR}/${TMPLDIR}"
DEFTMPL="${DSTDIR}/default.tmpl"

if [ $# -eq 0 ] ; then
	echo "Usage: ${THISFILE} file1 file2 ...";
	echo "	  You can use this command to initialize files with standard head."
	exit 0
fi

process_one_file()
{
	cat $1 > $2;
	echo "Use template '$1' for generate file '$2' success.";
}

while test $# -gt 0 ;
do
	if [[ -e $1 ]]; then
		echo "Error: File '$1' exist.";
		exit 1;
	fi

	LEN=`expr length \"$1\"`
	POS=`expr index \"$1\" .`

	if( test ${POS} -eq 0); then
		SUFFIX="";
		TMPL="${DEFTMPL}";
	else
		POS=`expr ${POS} + 1`;
		LEN=`expr ${LEN} - ${POS}`;
		SUFFIX=`expr substr \"$1\" ${POS} ${LEN}`;
		TMPL="${DSTDIR}/${SUFFIX}.tmpl";
	fi

	if [[ -e ${TMPL} ]]; then
		process_one_file ${TMPL} $1;
	elif [[ -e ${DEFTMPL} ]]; then
		process_one_file ${DEFTMPL} $1;
	else
		echo "Error: Template for target file '$1' can not be determined.";
		exit 1;
	fi

	shift;
done

