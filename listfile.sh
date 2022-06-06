#!/bin/sh
# generate tag file for lookupfile plugin
echo -e "!_TAG_FILE_SORTED\t2\t/2=foldcase/" > filenametags
find . \( -name "*.sh" -or -name "*.c" -or -name "*.h" -or -name "*.cpp" -or -name "*.cxx" -or -name "*.xml" \) -type f -printf "%f\t%p\t1\n" | \
    sort -f >> filenametags
