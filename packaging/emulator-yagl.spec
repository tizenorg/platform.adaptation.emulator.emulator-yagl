%bcond_with wayland

Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.0
Release:    18
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
#URL:        http://www.khronos.org
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(libdrm)
%if %{with wayland}
BuildRequires:  pkgconfig(gbm)
%endif

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
%if %{with wayland}
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr -DINSTALL_LIB_DIR=lib/yagl -DPLATFORM_GBM=1
%else
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr -DINSTALL_LIB_DIR=lib/yagl
%endif
make

%install
make install
ln -s libGLESv2.so.2.0 %{buildroot}/usr/lib/yagl/libGLESv2.so.1.0
ln -s libGLESv2.so.1.0 %{buildroot}/usr/lib/yagl/libGLESv2.so.1
mkdir -p %{buildroot}/usr/lib/systemd/system
cp packaging/emul-opengl-yagl.service %{buildroot}/usr/lib/systemd/system
mkdir -p %{buildroot}/etc/emulator
cp packaging/virtgl.sh %{buildroot}/etc/emulator
mkdir -p %{buildroot}/usr/lib/udev/rules.d
cp packaging/95-tizen-emulator.rules %{buildroot}/usr/lib/udev/rules.d

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
/usr/lib/udev/rules.d/95-tizen-emulator.rules
%attr(777,root,root)/etc/emulator/virtgl.sh

%files devel
%defattr(-,root,root,-)
/usr/include/EGL
/usr/include/GL
/usr/include/GLES
/usr/include/GLES2
/usr/include/KHR
