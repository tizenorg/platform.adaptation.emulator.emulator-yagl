%bcond_with wayland
%bcond_with emulator

Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.4
Release:    2
License:    MIT
#URL:        http://www.khronos.org
Source0:    %{name}-%{version}.tar.gz
Source1001:     emulator-yagl.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  flex
BuildRequires:  bison
%if %{with wayland}
BuildRequires:  pkgconfig(gbm)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-server)
%else
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xshmfence)
BuildRequires:  pkgconfig(xcb)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(dri2proto)
%endif
Requires:   libtbm
Provides:   opengl-es-drv

%if %{with emulator}
ExclusiveArch: %{ix86} x86_64
%else
ExclusiveArch:
%endif

%description
YaGL - OpenGLES acceleration module for emulator.
This package contains shared libraries libEGL, libGLES_CM, libGLESv2.

%package devel
Summary:    YaGL - OpenGLES acceleration module for emulator (devel)
Provides:   opengl-es-drv-devel
Requires:   %{name} = %{version}-%{release}
Requires: pkgconfig(x11)

%description devel
YaGL - OpenGLES acceleration module for emulator (devel)

%prep
%setup -q

%build
cp %{SOURCE1001} .
%if %{with wayland}
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr -DPLATFORM_X11=0 -DPLATFORM_GBM=1 -DPLATFORM_WAYLAND=1
%else
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr -DINSTALL_LIB_DIR=lib/yagl -DDUMMY_LIBS=1
%endif
make

%install
make install
%if %{with wayland}
ln -s libEGL.so.1.0 %{buildroot}/usr/lib/libEGL.so.1.0.0
ln -s libGLESv2.so.2.0 %{buildroot}/usr/lib/libGLESv2.so.2.0.0
ln -s libGLESv2.so.2.0.0 %{buildroot}/usr/lib/libGL.so.1.2.0
%else
ln -s libGLESv2.so.2.0 %{buildroot}/usr/lib/yagl/libGLESv2.so.1.0
ln -s libGLESv2.so.1.0 %{buildroot}/usr/lib/yagl/libGLESv2.so.1

ln -s yagl/libEGL.so.1.0 %{buildroot}%{_libdir}/libEGL.so.1
ln -s libEGL.so.1 %{buildroot}%{_libdir}/libEGL.so
ln -s yagl/libGLESv1_CM.so.1.0 %{buildroot}%{_libdir}/libGLESv1_CM.so.1
ln -s libGLESv1_CM.so.1 %{buildroot}%{_libdir}/libGLESv1_CM.so
ln -s yagl/libGLESv2.so.1.0 %{buildroot}%{_libdir}/libGLESv2.so.1
ln -s libGLESv2.so.1 %{buildroot}%{_libdir}/libGLESv2.so

mkdir -p %{buildroot}/etc/emulator
cp packaging/opengl-es-setup-yagl-env.sh %{buildroot}/etc/emulator

mkdir -p %{buildroot}/usr/share/license
cp COPYING %{buildroot}/usr/share/license/%{name}
%endif

mkdir -p %{buildroot}/usr/include
cp -r include/EGL %{buildroot}/usr/include/
cp -r include/GL %{buildroot}/usr/include/
cp -r include/GLES %{buildroot}/usr/include/
cp -r include/GLES2 %{buildroot}/usr/include/
cp -r include/GLES3 %{buildroot}/usr/include/
cp -r include/KHR %{buildroot}/usr/include/

mkdir -p %{buildroot}/usr/lib/pkgconfig
cp pkgconfig/* %{buildroot}/usr/lib/pkgconfig/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%if %{with wayland}
/usr/lib/*.so*
%else
/usr/lib/libEGL*
/usr/lib/libGLES*
/usr/lib/yagl/*
/usr/lib/dummy-gl/*
%attr(777,root,root)/etc/emulator/opengl-es-setup-yagl-env.sh
/usr/share/license/%{name}
%endif

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/include/EGL
/usr/include/GL
/usr/include/GLES
/usr/include/GLES2
/usr/include/GLES3
/usr/include/KHR
/usr/lib/pkgconfig
