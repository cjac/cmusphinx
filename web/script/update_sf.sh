#!/bin/bash

# Make the user id configurable by the use of an environmental
# variable SF_USER. Set it to current user, if not defined.
if test x$SF_USER == x; then SF_USER=`whoami`,cmusphinx; fi

TMP=build$$

mkdir $TMP

pushd $TMP > /dev/null

svn export https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/web  > /dev/null

#pushd web/cgi-bin > /dev/null
#svn log -v https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx > svn_history
#gzip -9f svn_history
#rsync -e ssh -auv --progress * $SF_USER@web.sourceforge.net:/home/groups/c/cm/cmusphinx/cgi-bin/ > /dev/null
#popd > /dev/null

pushd web/htdocs/html > /dev/null
rsync -e ssh -auv --delete . $SF_USER@web.sourceforge.net:/home/groups/c/cm/cmusphinx/htdocs/html > /dev/null
rsync -e ssh -lptgoDuv * www.speech.cs.cmu.edu:/usr1/httpd/html/sphinx > /dev/null
popd > /dev/null

for module in cmuclmtk sphinx2 sphinx3 sphinxbase SphinxTrain; do
    (
    svn export https://cmusphinx.svn.sourceforge.net/svnroot/cmusphinx/trunk/$module/doc $module > /dev/null
    cd $module > /dev/null
    rsync -e ssh -auv --delete . $SF_USER@web.sourceforge.net:/home/groups/c/cm/cmusphinx/htdocs/doc/$module > /dev/null
)
done

popd > /dev/null

/bin/rm -rf $TMP

# Just in case, change group ownership and permissions

#ssh $SF_USER@web.sourceforge.net chgrp -R cmusphinx /home/groups/c/cm/cmusphinx/ > /dev/null 2>&1
#ssh $SF_USER@web.sourceforge.net chmod g+w -R cmusphinx /home/groups/c/cm/cmusphinx/ > /dev/null 2>&1

# revision 1.7 2006/08/02 15:30:54 egouvea
# Disabled section about sphinx-4.

# revision 1.6 2006/07/13 22:05:20 egouvea
# Made path absolute

# revision 1.5 2006/05/26 21:26:17 egouvea
# Copies all files in current dir ('*') rather than only the dir ('.')

# revision 1.4 2005/11/10 21:58:39 egouvea
# Make the rsync not recursive for the fife location

# revision 1.3 2005/11/07 17:14:46 egouvea
# Updated update script

# revision 1.2 2005/09/19 16:15:24 egouvea
# Sidebar now uses <ul> added notice saying people should contact the authors directly for external links script deletes files that are different

# revision 1.1 2005/06/17 18:27:50 egouvea
# Added file to update the sf.net site
