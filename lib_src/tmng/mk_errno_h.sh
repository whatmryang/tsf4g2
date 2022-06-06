#!/bin/bash

function usage
{
    echo "Usage: $0 <head_file> <(c/cpp/h) file list>..."
    exit 1
}


PYFILE=${0%.sh}.py
HEAD_FILE=$1

[ "X${HEAD_FILE}" == "X" ] && usage
[ -f ${HEAD_FILE} -a -f ${PYFILE} ] || usage

MOD_NAME=`grep '//-->error_table ' ${HEAD_FILE} \
    | head -1 \
    | sed -e 's/^[^ ]* //' \
    | sed -e 's/ .*$//'`

[ "X${MOD_NAME}" == "X" ] && usage

MOD_HEAD=`echo "${MOD_NAME}_err_" | tr "a-z" "A-Z"`

OLD_CODES=`grep "^    $MOD_HEAD" $HEAD_FILE \
    | sed -e 's/^    \|,.*$//g' \
    | grep -v '_ERR_OK = 0' \
    | tr " \t\r\n" "=" `
OLD_CODES="=${OLD_CODES}="


FILES=${@#[^ ]* }
FILES=${FILES#[^ ]* }


NEW_CODES=`grep ${MOD_HEAD} $FILES \
    | sed -e "s/^[^:]*://" \
    | sed -e "s/^.*${MOD_HEAD}/${MOD_HEAD}/g" \
    | sed -e 's/[^A-Z_].*$//g' | sort | uniq`

CODES=""
for i in $NEW_CODES
do
    [ ${OLD_CODES/=${i}=/} == ${OLD_CODES} ] && {
        CODES="$CODES $i"
    }
done

[ "X$CODES" == "X" ] && exit 0

for i in $CODES
do
    echo "$i"
    read LINE
    echo $LINE
done \
| python ${PYFILE} ${HEAD_FILE} >${HEAD_FILE}.tmp

echo ""
echo -n "Usage new head file replace old file? (y/n) "
read LINE
[ "X$LINE" == "Xy" ] && {
    rm -f ${HEAD_FILE}
    mv ${HEAD_FILE}.tmp ${HEAD_FILE}
} || {
    rm -f ${HEAD_FILE}.tmp
}

