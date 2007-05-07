#!/usr/bin/perl

use strict;

1;

# The .regression file has the following format. Each line has three
# fields separated by ":". The first field is an arbitrary label, the
# second is a user in the muysql server, and the last is the password:
#
# admin:c1904admin:<passwd>
# rw:c1904rw:<passwd>
# ro:c1904ro:<passwd>
#
# The passwords are available to the project's admins in the mysql
# page under the admin link.

sub getUserPwd {
  my $tag = shift;
  my $pwdfn = "$ENV{HOME}/.regression";
  my $user = "";
  my $passwd = "";
  if (open (PWD, "< $pwdfn")) {
    while (<PWD>) {
      next unless m/^$tag:/;
      chomp();
      ($tag, $user, $passwd) = split /:/;
    }
    close(PWD);
  }
  return ($user, $passwd);
}

# Little hack. Let's use the function "getUserPwd()" to get the
# server/default database" from the .regression file, or set a
# meaningful default. The server info, if present in the .regression
# file, will have the format:
#
# server:host:database
#
# e.g.: server:mysql4-c:c1904_regression

getServer {
    my ($server, $db) = getUserPwd('server');

    $server = "mysql4-c" if ($server =~ m/^\s*$/);
    $db = "c1904_regression" if ($db =~ m/^\s*$/);
    return ($server, $db);
}

