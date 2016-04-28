#%bcond_with wayland
#%bcond_with emulator
%define ENABLE_TIZEN_BACKEND 1

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
%if "%{ENABLE_TIZEN_BACKEND}" == "1"
BuildRequires:  libtpl-egl-devel
%endif
BuildRequires:  libwayland-egl-devel
BuildRequires:  pkgconfig(gbm)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-server)
Provides:   opengl-es-drv

#%if %{with emulator}
#ExclusiveArch: %{ix86} x86_64
#%else
#ExclusiveArch:
#%endif

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
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_TIZEN=1
%else
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_X11=0 -DPLATFORM_GBM=0 -DPLATFORM_WAYLAND=1
%endif
make

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig

make install

%if "%{ENABLE_TIZEN_BACKEND}" == "0"
cp pkgconfig/wayland-egl.pc %{buildroot}%{_libdir}/pkgconfig/
%post   -n libwayland-egl -p /sbin/ldconfig
%postun -n libwayland-egl -p /sbin/ldconfig
ln -sf driver/libGLESv1_CM.so.1.0 %{buildroot}%{_libdir}/libGLESv1_CM.so.1.0
%endif

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%{_libdir}/driver/libEGL*
%{_libdir}/driver/libGL*
%if "%{ENABLE_TIZEN_BACKEND}" == "0"
%{_libdir}/libGLESv1_CM.so.1.0
%endif
#%attr(777,root,root)/etc/emulator/opengl-es-setup-yagl-env.sh
#%endif

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
