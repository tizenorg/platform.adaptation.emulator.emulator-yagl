%bcond_with wayland
%bcond_with emulator

%define with_emulator 1

%define ENABLE_TIZEN_BACKEND 1

Name:       emulator-yagl
Summary:    YaGL - OpenGLES acceleration module for emulator
Version:    1.6
Release:    1
License:    MIT and LGPL-3.0+
Group:      SDK/Libraries
Source0:    %{name}-%{version}.tar.gz
Source1001:     emulator-yagl.manifest
BuildRequires:  cmake
BuildRequires:  flex
BuildRequires:  bison
BuildRequires:  pkgconfig(libdrm)
%if "%{ENABLE_TIZEN_BACKEND}" == "1"
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(tpl-egl)
BuildRequires:  pkgconfig(wayland-egl)
%endif
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

%if "%{ENABLE_TIZEN_BACKEND}" == "0"
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
%if "%{ENABLE_TIZEN_BACKEND}" == "1"
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_TIZEN=1 -DDUMMY_LIBS=1
%else
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_X11=0 -DPLATFORM_GBM=0 -DPLATFORM_WAYLAND=1
%endif
make

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig
mkdir -p %{buildroot}/etc/profile.d

make install

cp packaging/opengl-es-setup-yagl-env.sh %{buildroot}/etc/profile.d/

%if "%{ENABLE_TIZEN_BACKEND}" == "0"
cp pkgconfig/wayland-egl.pc %{buildroot}%{_libdir}/pkgconfig/
%post   -n libwayland-egl -p /sbin/ldconfig
%postun -n libwayland-egl -p /sbin/ldconfig
%endif

ln -sf driver/libGLESv1_CM.so.1.0 %{buildroot}%{_libdir}/libGLESv1_CM.so.1.1

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%{_libdir}/driver/libEGL*
%{_libdir}/driver/libGL*
%{_libdir}/libGLESv1_CM.so.1.1
%attr(770,root,root)/etc/profile.d/opengl-es-setup-yagl-env.sh

%if "%{ENABLE_TIZEN_BACKEND}" == "0"
%files -n libwayland-egl
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%{_libdir}/libwayland-egl*

%files -n libwayland-egl-devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/wayland-egl.pc
%endif
