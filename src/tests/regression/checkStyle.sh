#!/bin/sh

if which indent > /dev/null 2>&1 ; then
    echo "Running style check";
else
    echo "indent not found; exiting.";
    exit 1;
fi

outfile="`pwd`/style.out";

pushd  ../../..  > /dev/null 2>&1

/bin/rm -rf $outfile
touch $outfile
for file in `find . -name '*.[ch]' -print`; do 
    indent -kr $file -o /tmp/tmpfile >> $outfile 2>&1; 
    if ! diff $file /tmp/tmpfile > /dev/null 2>&1; then 
	echo "$file does not conform to K&R" >> $outfile; 
    fi; 
done

popd > /dev/null 2>&1
