Summary: A Client of DC with Ignore User&few Others Features
Name: bmdc
Version: 0.1.9
Release: 1
License: LGPL
Group: Applications/Internet
Source: http://launchpad.net/bmdc++/%{name}%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-buildroot
BuildRequires:	glib2-devel >= 2.4
BuildRequires:	gtk+3-devel >= 3.6
BuildRequires:	bzip2-devel
BuildRequires:	zlib-devel
BuildRequires:	libgeoip-devel
BuildRequires:	openssl-devel
BuildRequires:	scons
BuildRequires:	libtar-devel

%description
A BMDC++ Client Mod of FreeDC++ ported to GTK3.
Media Spam, Higlitings Words & Few Others Features

%prep
%setup -q

%build
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT
scons PREFIX=/usr/

%install
scons FAKE_ROOT=$RPM_BUILD_ROOT release=1 install

%clean

%files
%defattr(-,root,root)
%{_bindir}/bmdc
%{_datadir}/bmdc/*
%{_datadir}/bmdc/country/*
%{_datadir}/doc/*
%{_datadir}/applications/bmdc.desktop
%{_datadir}/icons/*
%{_datadir}/locale/*

%changelog
* Fri Jan 29 2015 Mank <freedcpp@seznam.cz> 0.1.8-1
- intial version of spec file
 	
