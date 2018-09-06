Name: libmyhtml
Version: 4.0.5
Release: 1%{?dist}
Summary: MyHTML is a fast HTML Parser implemented as a pure C99 library.
License: LGPLv2.1
URL: https://github.com/lexborisov/myhtml
Source0: https://github.com/lexborisov/myhtml/archive/%{version}/v%{version}.tar.gz
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

mkdir -p %buildroot/%{_usr}/include
make install prefix=$RPM_BUILD_ROOT PROJECT_INSTALL_LIBRARY=%{_usr}/%{_lib} PROJECT_INSTALL_HEADER=%{_usr}/include

%files
%defattr(-,root,root,-)
%{_libdir}/*.so
%doc README.md
%{!?_licensedir:%global license %doc}

%files devel
%defattr(-,root,root,-)
%{_includedir}
%doc README.md
%{!?_licensedir:%global license %doc}

%changelog
* Thu Sep 06 2018 Alexander Borisov <lex.borisov@gmail.com> 4.0.5-1
- Please, see https://github.com/lexborisov/myhtml/blob/master/CHANGELOG.md
* Mon Jan 08 2018 Alexander Borisov <lex.borisov@gmail.com> 4.0.4-1
- Please, see https://github.com/lexborisov/myhtml/blob/master/CHANGELOG.md
* Tue Mar 21 2017 Alexander Borisov <lex.borisov@gmail.com> 4.0.0-1
- Initial release
