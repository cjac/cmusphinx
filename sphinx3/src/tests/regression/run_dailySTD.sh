#!/bin/sh

loopUntilSuccess () {
    cmd=$@
    # start loop to download code
    count=0;

    while ! $cmd; do
	count=`expr $count + 1`
	if [ $count -gt 50 ]; then
	    # not successful, and we attempted it too many times. Clean up and leave.
	    return $count
	fi
    done
}

export PATH=${PATH}:/usr/pbs/bin

if ! SVN=`command -v svn 2>&1`; then exit 1; fi
if ! QSUB=`command -v qsub 2>&1`; then exit 1; fi

TMPSCRIPT=/tmp/daily$$

TMPDIR=${HOME}/project/SourceForge/regression/daily$$
mkdir ${TMPDIR}
cd ${TMPDIR}

if loopUntilSuccess ${SVN} co https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinxbase > /dev/null ; then

if loopUntilSuccess ${SVN} co https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinx3 > /dev/null ; then

cat > ${TMPSCRIPT} <<EOF &&
#!/bin/sh

cd $TMPDIR

cd sphinxbase && 
./autogen.sh && 
./autogen.sh && 
make 

cd ../sphinx3 && 
./autogen.sh && 
./autogen.sh DISTCHECK_CONFIGURE_FLAGS=--with-sphinxbase=$TMPDIR/sphinxbase && 
make && 
chmod a+x ./src/tests/regression/dailySTD.sh && 
./src/tests/regression/dailySTD.sh

EOF

${QSUB} -l host=elroy -j oe -o ${HOME}/project/SourceForge/regression/daily.latest ${TMPSCRIPT} > jobid &&

cat > ${TMPSCRIPT} <<EOF &&
#!/bin/sh

/bin/rm -rf ${TMPDIR}
EOF

${QSUB} -W depend=afterany:`sed -e 's/.scrappy//' jobid` -j oe -o ${HOME}/project/SourceForge/regression/daily.latest ${TMPSCRIPT} > /dev/null

else
/bin/rm -rf ${TMPDIR}
fi
fi

/bin/rm ${TMPSCRIPT}
