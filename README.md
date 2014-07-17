# Syntactically Awesome Nginx Module

Providing on-the-fly compiling of [Sass](http://sass-lang.com/) files as an
nginx module.

Stop thinking about "sass watch" shell processes or the integration features of
your IDE while still using the power of Sass while developing your websites.

### Note

This module is highly experimental and has not been tested in any real
environment yet.

You can, and should, expect some weird bugs from serving unparsed files up to
completely deactivating your vhost.

Use with caution!


## Compilation

### Prerequisites

To be able to compile this module you need [libsass](https://github.com/sass/libsass)
available. The version used in the travis environment is `v2.0`, the use of a
different version can result in unexpected behaviour.

Also it has to be recognized by ldconfig:

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
- sass_interface.h
- sass2scss/sass2scss.h

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

The configuration is pretty straightforward.

Add a new location-block to your local server block similar to the (pretty
minimal) configuration example you can find in the [etc directory](etc/vhost.conf).
Using a rewrite ensures all the magic happens under the hood and you do not
have to change your application to load different files.

### Parameters

Error Log Level:

    location / {
        # same as regular nginx error log
        sass_error_log  error;  #default
    }

Image Path:

    location / {
        sass_image_path  "/path/to/something";
    }

Include Paths:

    location / {
        # windows (semicolon as path sep)
        sass_include_paths  "/some/dir;/another/dir";

        # everywhere else (colon as path sep)
        sass_include_paths  "/some/dir:/another/dir";
    }

Output Style:

    location / {
        sass_output  nested;     # default
        sass_output  expanded;
        sass_output  compact;
        sass_output  compressed;
    }

Source Comments:

    location / {
        sass_comments  none;    # default
        sass_comments  default;
        sass_comments  map;
    }
