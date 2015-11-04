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

Libsass: which has been included in this build. Remember to update it using

    $ git submodule init
    $ git submodule update

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
    sass_include_path  "/some/dir';
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

Detect SASS files (not SCSS):

```nginx
location / {
    sass_source_type  off;    # default
    sass_source_type  on;
}
```

Precision:

```nginx
location / {
    sass_precision:     5;       #default
    sass_precision:     10;
}
```
To do really simple source maps, just add:

Source map file:

```nginx
location / {
    sass_map_file:    "sass_output.map";
}
```
If it's not included in Nginx, it won't render out a source map for you.

Fine grained control of source map functionality is also covered by Nginx.

Source map embedding:

```nginx
location / {
    sass_map_embed  off;    # default
    sass_map_embed  on;
}
```
Omit source map urls:

```nginx
location / {
    sass_map_url  off;    # default
    sass_map_url  on;
}
```
Source map root:

```nginx
location / {
    sass_map_root:    "/some/directory";
}
```

