#!/usr/bin/env bash

cd "${0%/*}"

moduledir=`pwd`


echo "==> Checking parameters"

[ -z "${VER_LIBSASS}" ]   && echo 'parameter VER_LIBSASS missing'   && exit 1
[ -z "${VER_LUA_NGINX}" ] && echo 'parameter VER_LUA_NGINX missing' && exit 1
[ -z "${VER_NGX_DEVEL}" ] && echo 'parameter VER_NGX_DEVEL missing' && exit 1
[ -z "${VER_NGINX}" ]     && echo 'parameter VER_NGINX missing'     && exit 1

[ -z "${LUAJIT_INC}" ] && echo 'parameter LUAJIT_INC missing' && exit 1
[ -z "${LUAJIT_LIB}" ] && echo 'parameter LUAJIT_LIB missing' && exit 1


if [ "${1}" != "--nocompile" ]; then
  echo "==> Downloading sources"

  [ -z `which wget` ] && echo 'can not find "wget" to download libraries' && exit 2

  mkdir -p "${moduledir}/vendor"

  cd "${moduledir}/vendor"

  if [ ! -d "nginx-${VER_NGINX}" ]; then
    wget "http://nginx.org/download/nginx-${VER_NGINX}.tar.gz" -O nginx.tar.gz \
      && tar -xf nginx.tar.gz
  fi

  if [ ! -d "lua-nginx-module-${VER_LUA_NGINX}" ]; then
    wget "https://github.com/openresty/lua-nginx-module/archive/v${VER_LUA_NGINX}.tar.gz" -O lua-nginx-module.tar.gz \
      && tar -xf lua-nginx-module.tar.gz
  fi

  if [ ! -d "libsass-${VER_LIBSASS}" ]; then
    wget "https://github.com/sass/libsass/archive/${VER_LIBSASS}.tar.gz" -O libsass.tar.gz \
      && tar -xf libsass.tar.gz
  fi

  if [ ! -d "ngx_devel_kit-${VER_NGX_DEVEL}" ]; then
    wget "https://github.com/simpl/ngx_devel_kit/archive/v${VER_NGX_DEVEL}.tar.gz" -O ngx_devel_kit.tar.gz \
      && tar -xf ngx_devel_kit.tar.gz
  fi


  echo "==> Compiling LibSass"

  cd "${moduledir}/vendor/libsass-${VER_LIBSASS}"
  make shared


  echo "==> Building NGINX"

  cd "${moduledir}/vendor/nginx-${VER_NGINX}"

  if [ -d "${moduledir}/vendor/libsass-${VER_LIBSASS}/include" ]; then
    sass_include="${moduledir}/vendor/libsass-${VER_LIBSASS}/include"
  else
    # required for libsass < 3.3.0
    sass_include="${moduledir}/vendor/libsass-${VER_LIBSASS}"
  fi

  ./configure \
      --add-module="${moduledir}/vendor/ngx_devel_kit-${VER_NGX_DEVEL}" \
      --add-module="${moduledir}/vendor/lua-nginx-module-${VER_LUA_NGINX}" \
      --add-module="${moduledir}" \
      --with-cc-opt="-I ${sass_include}" \
      --with-ld-opt="-L ${moduledir}/vendor/libsass-${VER_LIBSASS}/lib"
  make || exit $?
fi


export PATH="$PATH:${moduledir}/vendor/nginx-${VER_NGINX}/objs"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${moduledir}/vendor/libsass-${VER_LIBSASS}/lib"


echo "==> Testing!"

cd "${moduledir}" && prove
