#%bcond_with wayland
#%bcond_with emulator

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
BuildRequires:  libtpl-egl-devel
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


%prep
%setup -q

%build
cp %{SOURCE1001} .
cmake -DCMAKE_INSTALL_PREFIX=%{buildroot} -DINSTALL_LIB_DIR=%{buildroot}%{_libdir} -DPLATFORM_X11=0 -DPLATFORM_GBM=0 -DPLATFORM_WAYLAND=0 -DPLATFORM_TIZEN=1
make

%install
rm -fr %{buildroot}
mkdir -p %{buildroot}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_libdir}/pkgconfig

make install


%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%license COPYING
%defattr(-,root,root,-)
%{_libdir}/driver/libEGL*
%{_libdir}/driver/libGL*
#%attr(777,root,root)/etc/emulator/opengl-es-setup-yagl-env.sh
#%endif

