Summary: A Client of DC with Lua
Name: bmdc
Version: 0.0.1
Release: 1
License: GNU GPL
Group: Applications/Internet
Source: http://bmdc.no-ip.sk/%{name}%{version}.tar.gz
Buildroot: %{_tmppath}/%{name}-%{version}-buildroot
BuildRequires:	glib2-devel >= 2.4
BuildRequires:	gtk+2-devel >= 2.6
BuildRequires:	libglade2.0-devel >= 2.4
BuildRequires:	bzip2-devel
BuildRequires:	zlib-devel
BuildRequires:	openssl-devel
BuildRequires:	scons

%description
A BMDC++ Client Mod of FreeDC++.
Media Spam, Lua, Higlitings Words

%prep
%setup

%build
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT
scons PREFIX=/usr

%install
#[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
scons FAKE_ROOT=$RPM_BUILD_ROOT release=1 install

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/bmdc
%{_datadir}/bmdc/*
%{_datadir}/country/*
%{_datadir}/doc/*
%{_datadir}/applications/bmdc.desktop
%{_datadir}/icons/*
%{_datadir}/locale/*

%changelog
* Sat Mar 24 2011 Mank <freedcpp@seznam.cz> 0.0.2-3
- intial version of spec file
 	

