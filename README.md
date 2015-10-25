# Syntactically Awesome NGINX Module

Providing on-the-fly compiling of [Sass](http://sass-lang.com/) files as an
NGINX module.

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
therefore more or less supported, combinations of libsass and NGINX versions
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
- sass_context.h
- sass_functions.h
- sass_values.h
- sass_version.h

### NGINX

Using this module is as easy as recompiling NGINX from source:

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
    # same as regular NGINX error log
    sass_error_log  error;  #default
}
```

Include Path:

```nginx
location / {
    # windows (semicolon as path sep)
    sass_include_path  "/some/dir;/another/dir";

    # everywhere else (colon as path sep)
    sass_include_path  "/some/dir:/another/dir";
}
```

Indentation:

```nginx
location / {
    sass_indent  "  ";   # default
    sass_indent  "    ";
}
```

Linefeed:

```nginx
location / {
    sass_linefeed  "\n";                 # default
    sass_linefeed  "\n/* linefeed */\n";
}
```

Precision of Fractional Numbers:

```nginx
location / {
    sass_precision  5; # default
    sass_precision  3;
}
```

Output Style:

```nginx
location / {
    sass_output_style  compact;
    sass_output_style  compressed;
    sass_output_style  expanded;
    sass_output_style  nested;     # default
}
```

Source Comments:

```nginx
location / {
    sass_source_comments  off; # default
    sass_source_comments  on;
}
```


## Testing

### Prerequisites

The unit tests use [Test::Nginx](http://github.com/agentzh/test-nginx) and Lua.

Please ensure your environment meets the following:

- `prove` (perl) is available
- `libluajit` is installed

To be able to run them using `prove` (perl).

### Testing Script

If you fulfill the prerequisites you can use the script `./compile_and_test.sh`
to download, compile and test in on go:

```shell
VER_LIBSASS=3.2.5 \
    VER_LUA_NGINX=0.9.16 \
    VER_NGX_DEVEL=0.2.19 \
    VER_NGINX=1.9.5 \
    LUAJIT_LIB=/usr/lib/x86_64-linux-gnu/ \
    LUAJIT_INC=/usr/include/luajit-2.0/ \
    ./compile_and_test.sh
```

The four passed variables `VER_LIBSASS`, `VER_LUA_NGINX`, `VER_NGX_DEVEL` and
`VER_NGINX` define the module versions your are using for compilation. If a
variable is not passed to the script it will be automatically taken from your
environment. An error messages will be printed if no value is available.

All dependencies will automatically be downloaded to the `./vendor` subfolder.

To skip the compilation (and download) step you can pass the `--nocompile` flag:

```shell
VER_LIBSASS=3.2.5 \
    VER_LUA_NGINX=0.9.16 \
    VER_NGX_DEVEL=0.2.19 \
    VER_NGINX=1.9.5 \
    LUAJIT_LIB=/usr/lib/x86_64-linux-gnu/ \
    LUAJIT_INC=/usr/include/luajit-2.0/ \
    ./compile_and_test.sh --nocompile
```

Please be aware that (for now) all the variables are still required for the
script to run.


## License

Licensed under the
[BSD 2 Clause License](https://opensource.org/licenses/BSD-2-Clause).
