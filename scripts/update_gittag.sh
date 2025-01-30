#!/bin/bash -e

if [ -z $1 ]; then
    echo "Usage: $(basename $0) <gittag.h>"
    exit 1
fi

gittagfile="$1"

TMPFILE=$(mktemp build/gittag.XXXXXX)
if [[ ! -e $TMPFILE ]]; then
    echo "Can't create tempfile!"
    exit 1
fi

GITTAG=$(git describe --tags --dirty=+ 2> /dev/null || echo "NOTAG")

echo -e "#ifndef GIT_TAG\n#define GIT_TAG \"G&W Bootloader "${GITTAG}"\"\n#endif" > "${TMPFILE}"

if ! diff -q ${TMPFILE} ${gittagfile} > /dev/null 2> /dev/null; then
    echo "Updating git tag file ${gittagfile}"
    cp -f "${TMPFILE}" "${gittagfile}"
fi

rm -f "${TMPFILE}"
