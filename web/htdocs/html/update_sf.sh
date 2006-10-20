#!/bin/sh

make
rsync -auv --progress --delete . shell.sf.net:/home/groups/c/cm/cmusphinx/htdocs/html
rsync -lptgoDuv --progress * fife.speech.cs.cmu.edu:/usr1/httpd/html/sphinx

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
