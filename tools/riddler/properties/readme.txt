This directory is where your machine-specific properties go.  An environment variable
called COMPUTERNAME is required to be defined in the shell that launches Ant.

At least all the properties present in malkovich.properties should be included.
Settings specific to your Glassfish installation are contained in a file
referenced by the first property in <computername>.properties,
deploy.ant.properties.file.  It's a good idea to leave this file in your home
directory rather than committing it to Subversion because it contains the
administrative password to your local Glassfish instance.  My glassfish.properties
file has the following values (password omitted):

sjsas.port=4848
sjsas.password=*****
sjsas.host=localhost
sjsas.url=http\://localhost\:null
sjsas.username=admin
sjsas.root=C\:\\tool\\glassfish
