#!/usr/bin/env bash

cd "${0%/*}"

moduledir=$(pwd)
nocompile=0

if [ "${1}" == "--nocompile" ]; then
  nocompile=1

  shift
fi


echo "==> Checking parameters"

[ -z "${VER_LIBSASS}" ]   && echo 'parameter VER_LIBSASS missing'   && exit 1
[ -z "${VER_LUA_NGINX}" ] && echo 'parameter VER_LUA_NGINX missing' && exit 1
[ -z "${VER_NGINX}" ]     && echo 'parameter VER_NGINX missing'     && exit 1

[ -z "${LUAJIT_INC}" ] && echo 'parameter LUAJIT_INC missing' && exit 1
[ -z "${LUAJIT_LIB}" ] && echo 'parameter LUAJIT_LIB missing' && exit 1


if [ 0 -eq ${nocompile} ]; then
  echo "==> Downloading sources"

  [ -z $(which wget) ] && echo 'can not find "wget" to download libraries' && exit 2

  mkdir -p "${moduledir}/vendor"

  cd "${moduledir}/vendor"

  if [ ! -d "nginx-${VER_NGINX}" ]; then
    wget -q "http://nginx.org/download/nginx-${VER_NGINX}.tar.gz" -O nginx.tar.gz \
      && tar -xf nginx.tar.gz
  fi

  if [ ! -d "lua-nginx-module-${VER_LUA_NGINX}" ]; then
    VER_LUA_NGINX_DL="${VER_LUA_NGINX}"

    if [ 40 -ne ${#VER_LUA_NGINX} ]; then
      VER_LUA_NGINX_DL="v${VER_LUA_NGINX_DL}"
    fi

    wget -q "https://github.com/openresty/lua-nginx-module/archive/${VER_LUA_NGINX_DL}.tar.gz" -O lua-nginx-module.tar.gz \
      && tar -xf lua-nginx-module.tar.gz
  fi

  if [ ! -d "libsass-${VER_LIBSASS}" ]; then
    wget -q "https://github.com/sass/libsass/archive/${VER_LIBSASS}.tar.gz" -O libsass.tar.gz \
      && tar -xf libsass.tar.gz
  fi


  echo "==> Compiling LibSass"

  cd "${moduledir}/vendor/libsass-${VER_LIBSASS}"
  make shared


  echo "==> Building NGINX"

  cd "${moduledir}/vendor/nginx-${VER_NGINX}"

  if [ ! -z "${DYNAMIC}" ]; then
    ./configure \
        --add-module="${moduledir}/vendor/lua-nginx-module-${VER_LUA_NGINX}" \
        --add-dynamic-module="${moduledir}" \
        --with-cc-opt="-I ${moduledir}/vendor/libsass-${VER_LIBSASS}/include" \
        --with-ld-opt="-L ${moduledir}/vendor/libsass-${VER_LIBSASS}/lib"
  else
    ./configure \
        --add-module="${moduledir}/vendor/lua-nginx-module-${VER_LUA_NGINX}" \
        --add-module="${moduledir}" \
        --with-cc-opt="-I ${moduledir}/vendor/libsass-${VER_LIBSASS}/include" \
        --with-ld-opt="-L ${moduledir}/vendor/libsass-${VER_LIBSASS}/lib"
  fi

  make || exit $?

  if [ ! -z "${DYNAMIC}" ]; then
    cp "./objs/ngx_http_sass_module.so" "${moduledir}/vendor/"
  fi
fi


export PATH="$PATH:${moduledir}/vendor/nginx-${VER_NGINX}/objs"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${moduledir}/vendor/libsass-${VER_LIBSASS}/lib"


echo "==> Testing!"

cd "${moduledir}" && prove $@
