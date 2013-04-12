Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.0
Release:    16
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
#URL:        http://www.khronos.org
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xdamage)

%description
YaGL - OpenGLES acceleration module for emulator.
This package contains shared libraries libEGL, libGLES_CM, libGLESv2.

%package devel
Summary:    YaGL - OpenGLES acceleration module for emulator (devel)
Requires:   %{name} = %{version}-%{release}
Requires: pkgconfig(x11)

%description devel
YaGL - OpenGLES acceleration module for emulator (devel)

%prep
%setup -q

%build
make

%install
make INSTALL_LIB_DIR=%{buildroot}/usr/lib/yagl install
mkdir -p %{buildroot}/usr/lib/systemd/system
cp packaging/emul-opengl-yagl.service %{buildroot}/usr/lib/systemd/system
mkdir -p %{buildroot}/etc
cp packaging/virtgl.sh %{buildroot}/etc

mkdir -p %{buildroot}/usr/include
cp -r include/EGL %{buildroot}/usr/include/
cp -r include/GL %{buildroot}/usr/include/
cp -r include/GLES %{buildroot}/usr/include/
cp -r include/GLES2 %{buildroot}/usr/include/
cp -r include/KHR %{buildroot}/usr/include/

%files
%defattr(-,root,root,-)
/usr/lib/yagl/*
/usr/lib/systemd/system/emul-opengl-yagl.service
%attr(777,root,root)/etc/virtgl.sh

%files devel
%defattr(-,root,root,-)
/usr/include/EGL
/usr/include/GL
/usr/include/GLES
/usr/include/GLES2
/usr/include/KHR
