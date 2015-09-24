# Syntactically Awesome Nginx Module

Providing on-the-fly compiling of [Sass](http://sass-lang.com/) files as an
nginx module, soon hopefully with source maps.

Stop thinking about "sass watch" shell processes or the integration features of
your IDE while still using the power of Sass while developing your websites.

### Note

This module is experimental and only been used in development environments.

You can, and should, expect some weird bugs from serving unparsed files up to
completely deactivating your vhost.

Use with caution!


## Compilation

### Prerequisites

Libsass has been moved into core include libs so shouldn't hopefully need LDFLAGS to run.

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

The configuration is pretty straightforward.

Add a new location-block to your local server block similar to the (pretty
minimal) configuration example you can find in the [etc directory](etc/vhost.conf).
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
    sass_output  nested;     # default
    sass_output  expanded;
    sass_output  compact;
    sass_output  compressed;
}
```

Source Comments:

```nginx
location / {
    sass_comments  off;    # default
    sass_comments  on;
}
```

Precision:

```nginx
location / {

    precision:     5;       #default
    precision:     10;
}
```
