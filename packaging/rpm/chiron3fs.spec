Name:           chiron3fs
Version:        3.0.2
Release:        1%{?dist}
Summary:        Fault-tolerant replicated filesystem facade built on FUSE3

License:        GPL-3.0-only
URL:            https://github.com/steigr/chiron3fs
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  fuse3-devel
BuildRequires:  pkgconfig

Requires:       fuse3

%description
chiron3fs replicates filesystem operations to multiple replica directories
through a mounted facade filesystem.

%prep
%autosetup

%build
./configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{_mandir}
%make_build

%install
%make_install

%files
%license %{_datadir}/doc/chiron3fs/copyright
%{_bindir}/chiron3fs
%{_bindir}/chiron3ctl
%{_mandir}/man8/chiron3fs.8*
%{_datadir}/doc/chiron3fs/*

%changelog
* Sat Apr 18 2026 steigr <chiron3fs@stei.gr> - 3.0.2-1
- Initial Fedora RPM packaging for chiron3fs.

