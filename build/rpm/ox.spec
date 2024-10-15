Name:           ox
Version:        0.0.1
Release:        1%{?dist}
Summary:        OX script language

License:        MIT
URL:            https://gitee.com/gongke1978/ox
Group:          Development/Languages
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc, clang, make, gettext, which, libffi-devel, libcurl-devel, libarchive-devel, sdl12-compat-devel, ncurses-devel, openssl-devel, glib2-devel
Requires:       libffi, libcurl, libarchive, sdl12-compat, ncurses-libs, openssl-libs, glib2

%global so_version 0
%global lib_arch lib64
%global arch x86_64-pc-linux-gnu

%description
OX script language running environment.

%prep
%autosetup

%build
TOP=`pwd`
make basic LIB_ARCH=%{lib_arch} DEBUG=1 -j32
make env OX_EXE="$TOP/out/bin/ox.sh" DEBUG=1 -j32 LIB_ARCH=%{lib_arch}
make packages OX_EXE="$TOP/out/bin/ox.sh" DEBUG=1 PB_LIBS="-L$TOP/out" LIB_ARCH=%{lib_arch} INTERNAL_PKGS=1

%install
TOP=`pwd`
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/ox/pkg/all
mkdir -p %{buildroot}/usr/share/ox/pkg/%{arch}
mkdir -p %{buildroot}/usr/share/ox/doc
mkdir -p %{buildroot}/usr/share/ox/server
mkdir -p %{buildroot}/usr/share/locale
mkdir -p %{buildroot}/usr/%{lib_arch}
$TOP/out/bin/ox.sh -r pm -S out/oxp -I %{buildroot}/usr --libdir lib64 --no-dep -s ox std json curl archive oxp pm
install -m 755 out/bin/ox %{buildroot}/usr/bin
rm -rf %{buildroot}/usr/share/ox/oxp

%files
/usr/bin/ox
/usr/bin/ox-cli
/usr/%{lib_arch}/libox.so.%{so_version}
/usr/share/locale/zh_CN/LC_MESSAGES/ox.mo
/usr/share/ox/server/main.ox
/usr/share/ox/doc
/usr/share/ox/pkg/%{arch}
/usr/share/ox/pkg/all

%changelog
* Fri Apr 25 2025 Super User
- 
