Name: libmyhtml
Version: 4.0.0
Release: 1%{?dist}
Summary: MyHTML is a fast HTML Parser implemented as a pure C99 library.
License: LGPLv2.1
URL: https://github.com/lexborisov/myhtml
Source0: https://github.com/lexborisov/myhtml/archive/%{version}/v%{version}.tar.gz
BuildArch: noarch
BuildRequires: make, glibc-devel

%description
 MyHTML is a fast HTML Parser using Threads implemented as
 a pure C99 library with no outside dependencies.

%package devel
Requires: libmyhtml
Summary: MyHTML development files
Provides: libmyhtml-devel

%description devel
 This package contains necessary header files for MyHTML development.

%prep
%setup -q -n %{name}-%{version}

%build
make shared

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p %buildroot/include
cp -r include %buildroot/include
make install prefix=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%doc LICENCE README*
%license LICENSE

%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}
%doc LICENCE README*
%license LICENSE

%changelog
* Tue Mar 21 2017 Alexander Borisov <lex.borisov@gmail.com> 4.0.0-1
- Initial release
