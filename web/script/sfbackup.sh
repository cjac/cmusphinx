#!/bin/bash


loopUntilSuccess () {
    cmd=$@
    # start loop to download code
    count=0;

    while ! $cmd; do
        count=`expr $count + 1`
        if [ $count -gt 50 ]; then
            # not successful, and we attempted it too many times. Clean up and l
eave.
            return $count
        fi
    done
}

# Check that we have all executables
if ! SSH=`command -v ssh 2>&1`; then exit 1; fi
if ! SCP=`command -v scp 2>&1`; then exit 1; fi
if ! RSYNC=`command -v rsync 2>&1`; then exit 1; fi
if ! TAR=`command -v gtar 2>&1`; then
  if ! TAR=`command -v tar 2>&1`; then exit 1; fi
fi

# cd to directory where files are kept
cd ${HOME}/project/SourceForge/backup

# create the web space backup file
${SSH} www.speech.cs.cmu.edu ${TAR} czf cmusphinxWeb.tgz -C project cmusphinx
# copy it locally
${SCP} -B -q www.speech.cs.cmu.edu:cmusphinxWeb.tgz cmusphinxWeb.tgz.new
# remove the backup from sourceforge
${SSH} www.speech.cs.cmu.edu rm cmusphinxWeb.tgz
if test -e cmusphinxWeb.tgz.new; then
# Copy the twiki/web from www.speech.cs
# shift the backup files, so that "-6" is the oldest
rm cmusphinxWeb-6.tgz
mv cmusphinxWeb-5.tgz cmusphinxWeb-6.tgz
mv cmusphinxWeb-4.tgz cmusphinxWeb-5.tgz
mv cmusphinxWeb-3.tgz cmusphinxWeb-4.tgz
mv cmusphinxWeb-2.tgz cmusphinxWeb-3.tgz
mv cmusphinxWeb-1.tgz cmusphinxWeb-2.tgz
mv cmusphinxWeb.tgz cmusphinxWeb-1.tgz
mv cmusphinxWeb.tgz.new cmusphinxWeb.tgz
fi

# backup the repository
# First, rsync the cvs repository, since we still use it for regression test
${RSYNC} -a rsync://cmusphinx.cvs.sourceforge.net/cvsroot/cmusphinx/'*' cvs || echo "CVS rsync failed"
# Then rsync the svn repository
${RSYNC} -a cmusphinx.svn.sourceforge.net::svn/cmusphinx/'*' svn || echo "SVN rsync failed"

# copy the web space locally
loopUntilSuccess $RSYNC -auvz --delete `whoami`,cmusphinx@web.sourceforge.net:/home/groups/c/cm/cmusphinx . && ${TAR} czf sfWeb.tgz.new cmusphinx
if test -e sfWeb.tgz.new; then
# shift the backup files, so that "-6" is the oldest
rm sfWeb-6.tgz
mv sfWeb-5.tgz sfWeb-6.tgz
mv sfWeb-4.tgz sfWeb-5.tgz
mv sfWeb-3.tgz sfWeb-4.tgz
mv sfWeb-2.tgz sfWeb-3.tgz
mv sfWeb-1.tgz sfWeb-2.tgz
mv sfWeb.tgz sfWeb-1.tgz
mv sfWeb.tgz.new sfWeb.tgz
fi

# list the current local directory
/bin/ls -l

