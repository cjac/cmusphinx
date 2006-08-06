#!/bin/sh

TMPSCRIPT=/tmp/PERFFULL$$

TMPDIR=${HOME}/project/SourceForge/regression/PERFFULL$$
mkdir ${TMPDIR}
cd ${TMPDIR}

if ${HOME}/script/loopUntilSuccess.sh /usr/local/bin/svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinxbase > /dev/null ; then

if ${HOME}/script/loopUntilSuccess.sh /usr/local/bin/svn co https://svn.sourceforge.net/svnroot/cmusphinx/trunk/sphinx3 > /dev/null ; then

cat > ${TMPSCRIPT} <<EOF &&
#!/bin/sh

cd $TMPDIR

cd sphinxbase && 
./autogen.sh && 
./autogen.sh && 
make 

cd ../sphinx3 && 
./autogen.sh && 
./autogen.sh DISTCHECK_CONFIGURE_FLAGS=--with-sphinxbase=$root/sphinxbase && 
make && 
chmod a+x ./src/tests/regression/PERFFULL.sh && 
./src/tests/regression/PERFFULL.sh

EOF

/usr/pbs/bin/qsub -l host=astro -j oe -o ${HOME}/project/SourceForge/regression/perffull.latest ${TMPSCRIPT} > jobid &&

cat > ${TMPSCRIPT} <<EOF &&
#!/bin/sh

/bin/rm -rf ${TMPDIR}
EOF

/usr/pbs/bin/qsub -W depend=afterany:`sed -e 's/.scrappy//' jobid` -j oe -o ${HOME}/project/SourceForge/regression/perffull.latest ${TMPSCRIPT} > /dev/null

else
/bin/rm -rf ${TMPDIR}
fi
fi

/bin/rm ${TMPSCRIPT}
