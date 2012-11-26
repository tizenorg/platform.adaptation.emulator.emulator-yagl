Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.0
Release:    11
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
#URL:        http://www.khronos.org
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xext)
Requires: gawk
Requires: eet-tools
Requires: e17-misc

%description 
YaGL - OpenGLES acceleration module for emulator.
This package contains shared libraries libEGL, libGLES_CM, libGLESv2.

#%package devel
#Summary:    OpenGLES acceleration module for emulator (devel)
#Group:      TO_BE/FILLED_IN
#Requires:   %{name} = %{version}-%{release}
#Requires:   pkgconfig(x11)
#
#%description devel
#YaGL - OpenGLES acceleration module for emulator. (devel)

%prep
%setup -q

%build
make

%install
mkdir -p %{buildroot}/usr/lib/yagl/
cp -r build/lib/*.so %{buildroot}/usr/lib/yagl/
mkdir -p %{buildroot}/usr/share/yagl/
cp -r install/* %{buildroot}/usr/share/yagl/

%post
eet -d /opt/home/root/.e/e/config/samsung/e.cfg config /usr/share/yagl/e.txt
cd /usr/share/yagl/
/usr/share/yagl/replace.sh /usr/share/yagl/replace.awk /usr/share/yagl/e.txt /usr/share/yagl/e1.txt
eet -e /opt/home/root/.e/e/config/samsung/e.cfg config e1.txt 1

%files
%defattr(-,root,root,-)
/usr/lib/yagl/*
/usr/share/yagl/replace.awk
%attr(777,root,root)/usr/share/yagl/replace.sh

%changelog
