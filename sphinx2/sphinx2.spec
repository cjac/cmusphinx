# Note that this is NOT a relocatable package
%define ver      0.1.1
%define rel      0
%define prefix   /usr

Summary: CMU Sphinx Recognition System
Name: sphinx2
Version: %ver
Release: %rel
Copyright: BSD-style (see LICENSE)
Group: User Interface
Source: http://www.speech.cs.cmu.edu/sphinx/src/sphinx2-%{ver}.tar.gz
BuildRoot: /tmp/sphinx2-%{ver}-root
Packager: Kevin Lenzo <lenzo@cs.cmu.edu>
URL: http://www.cs.cmu.edu/~lenzo

Docdir: %{prefix}/doc

%description
The CMU Sphinx Recognition System is a library and a set
of examples and utilities for speech recognition.

This package will install the sphinx2 library and some examples.

%changelog

%prep
%setup

%build
# Optimize that damned code all the way
if [ ! -z "`echo -n ${RPM_OPT_FLAGS} | grep pentium`" ]; then
  if [ ! -z "`which egcs`" ]; then
    CC="egcs" 
  else
    if [ ! -z "`which pgcc`" ]; then
      CC="pgcc"
    fi
  fi
  CFLAGS="${RPM_OPT_FLAGS}"
else
  CFLAGS="${RPM_OPT_FLAGS}"
fi
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
fi
make

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%post

%postun

%files
%defattr(-, root, root)

%{prefix}/bin/*
%{prefix}/share/sphinx2/model/lm/turtle/*
%{prefix}/share/sphinx2/model/hmm/4k/*

%doc AUTHORS
%doc COPYING
%doc INSTALL
%doc README
