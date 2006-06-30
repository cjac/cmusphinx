#!/bin/sh

# Define a path, just in case
export PATH="/usr/local/bin:/bin:/usr/bin:/usr/ccs/bin"

if ! which indent > /dev/null 2>&1 ; then
    echo "indent not found; exiting.";
    exit 1;
fi

pushd  `(cd src/tests/regression/../../.. && pwd)`  > /dev/null 2>&1

# outfile="`pwd`/style.out";

# /bin/rm -rf $outfile
# touch $outfile
for file in `find include -name '*.[ch]' -print && find src -name '*.[ch]' -print`; do 
    indent -i8 -kr -psl -nce -nut $file -o /tmp/tmpfile 2>&1; 
    if ! diff $file /tmp/tmpfile > /dev/null 2>&1; then 
	echo "$file does not conform to style"; 
    fi; 
done

popd > /dev/null 2>&1

echo "Report finished."
