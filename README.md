# Syntactically Awesome Nginx Module

Providing on-the-fly compiling of [Sass](http://sass-lang.com/) files as an
nginx module.

Stop thinking about "sass watch" shell processes or the integration features of
your IDE while still using the power of Sass while developing your websites.

### Note

This module is experimental and only been used in development environments.

You can, and should, expect some weird bugs from serving unparsed files up to
completely deactivating your vhost.

Use with caution!


## Compilation

### Prerequisites

To be able to compile this module you need
[libsass](https://github.com/sass/libsass) available. For a list of tested, and
therefore more or less supported, combinations of libsass and nginx versions
please refer to the travis environment in `.travis.yml`. Using different
versions can result in unexpected behaviour or won't work at all.

Also ldconfig has to find the library:

```shell
ldconfig -p | grep "libsass"
```

If it does not show up try to rebuild the index first using *ldconfig* as
*root* user and rerun the grep command. Sometimes you need to add the path
where libsass is installed manually to the list of paths ldconfig is looking
for libraries in.

This can be done by adding the path to the *$LD\_LIBRARY\_PATH* variable or the
file */etc/ld.so.conf*. Either way, please ensure the command above finds your
local libsass installation before proceeding.

During compilation the following header files have to be available:

- sass.h
- sass_functions.h
- sass_interface.h
- sass_values.h
- sass_version.h

### Unit Test Requirements

The unit tests use [Test::Nginx](http://github.com/agentzh/test-nginx) and Lua.

To be able to run them using `prove` you need to compile nginx with the
[lua module](https://github.com/openresty/lua-nginx-module) and
[devel kit module](https://github.com/simpl/ngx_devel_kit).

### Nginx

Using this module is as easy as recompiling nginx from source:

```shell
cd /path/to/nginx/src
./configure --add-module=/path/to/sass-nginx-module
make install
```

Or if you want to have debug logs available:

```shell
cd /path/to/nginx/src
./configure --add-module=/path/to/sass-nginx-module --with-debug
make install
```

To be able to run the unit tests you need additional modules configured:

```shell
cd /path/to/nginx/src
./configure \
  --add-module=/projects/public/ngx_devel_kit \
  --add-module=/projects/public/lua-nginx-module \
  --add-module=/projects/private/sass-nginx-module
make install
```


## Configuration

The configuration is pretty straightforward:

```nginx
server {
    location ~ ^.*\.css$ {
        sass_compile  on;

        rewrite  ^(.*)\.css$  $1.scss  break;
    }
}
```

Add this location block to your server activates the sass compilation.

Using a rewrite ensures all the magic happens under the hood and you do not
have to change your application to load different files.

### Parameters

Error Log Level:

```nginx
location / {
    # same as regular nginx error log
    sass_error_log  error;  #default
}
```

Include Paths:

```nginx
location / {
    # windows (semicolon as path sep)
    sass_include_paths  "/some/dir;/another/dir";

    # everywhere else (colon as path sep)
    sass_include_paths  "/some/dir:/another/dir";
}
```

Output Style:

```nginx
location / {
    sass_output  compact;
    sass_output  compressed;
    sass_output  expanded;
    sass_output  nested;     # default
}
```

Source Comments:

```nginx
location / {
    sass_comments  off; # default
    sass_comments  on;
}
```
