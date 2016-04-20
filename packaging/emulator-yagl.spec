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
Requires:   %{name} = %{version}-%{release}

%description devel
YaGL - OpenGLES acceleration module for emulator (devel)

%if %{with wayland}
%package -n libwayland-egl
Summary:    Wayland EGL backend

%description -n libwayland-egl
Wayland EGL backend

%package -n libwayland-egl-devel
Summary:    Development files for use with Wayland protocol

%description -n libwayland-egl-devel
Development files for use with Wayland protocol
%endif

%prep
%setup -q

%build
cp %{SOURCE1001} .
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_X11=0 -DPLATFORM_GBM=1 -DPLATFORM_WAYLAND=1
make

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig

make install

cp pkgconfig/wayland-egl.pc %{buildroot}%{_libdir}/pkgconfig/

%if %{with wayland}
ln -sf driver/libGLESv1_CM.so.1.0 %{buildroot}%{_libdir}/libGLESv1_CM.so.1.0
%endif

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%if %{with wayland}
%post   -n libwayland-egl -p /sbin/ldconfig
%postun -n libwayland-egl -p /sbin/ldconfig
%endif

%files
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%if %{with wayland}
%{_libdir}/libgbm*
%{_libdir}/driver/libEGL*
%{_libdir}/driver/libGL*
%{_libdir}/libGLESv1_CM.so.1.0
%else
%{_libdir}/libEGL*
%{_libdir}/libGLES*
%{_libdir}/yagl/*
%{_libdir}/dummy-gl/*
%attr(777,root,root)/etc/emulator/opengl-es-setup-yagl-env.sh
%endif

%if %{with wayland}
%files -n libwayland-egl
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%{_libdir}/libwayland-egl*

%files -n libwayland-egl-devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/wayland-egl.pc
%endif
