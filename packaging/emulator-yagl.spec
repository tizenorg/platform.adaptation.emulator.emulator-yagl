%bcond_with wayland
%bcond_with emulator

Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.6
Release:    1
License:    MIT and LGPL-3.0
Group:      SDK/Libraries
Source0:    %{name}-%{version}.tar.gz
Source1001:     emulator-yagl.manifest
BuildRequires:  cmake
BuildRequires:  flex
BuildRequires:  bison
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(gbm)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-server)
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

%description devel
YaGL - OpenGLES acceleration module for emulator (devel)
This package contains header files for EGL, GLESv1_CM, GLESv2, GLESv3, KHR.

%prep
%setup -q

%build
cp %{SOURCE1001} .
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr -DPLATFORM_X11=0 -DPLATFORM_GBM=1 -DPLATFORM_WAYLAND=1
make

%install
make install

mkdir -p %{buildroot}/usr/share/license
cp COPYING %{buildroot}/usr/share/license/%{name}

mkdir -p %{buildroot}/usr/lib/pkgconfig
cp pkgconfig/wayland-egl.pc %{buildroot}/usr/lib/pkgconfig/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%if %{with wayland}
/usr/lib/libgbm*
/usr/lib/libwayland-egl*
/usr/lib/driver/libEGL*
/usr/lib/driver/libGL*
%else
/usr/lib/libEGL*
/usr/lib/libGLES*
/usr/lib/yagl/*
/usr/lib/dummy-gl/*
%attr(777,root,root)/etc/emulator/opengl-es-setup-yagl-env.sh
%endif
/usr/share/license/%{name}

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/lib/pkgconfig
