# Syntactically Awesome NGINX Module

Providing on-the-fly compiling of [Sass](http://sass-lang.com/) files as an NGINX module.

Stop thinking about "sass watch" shell processes or the integration features of your IDE while still using the power of Sass while developing your websites.

Supported versions of LibSass:

- `3.4.0` (`3.4.9` used on travis)
- `3.5.0` (`3.5.5` used on travis)
- `3.6.0` (`3.6.3` used on travis)

### Note

This module is experimental and only been used in development environments.

You can, and should, expect some weird bugs from serving unparsed files up to completely deactivating your vhost.

Use with caution!

## Compilation

### Prerequisites

To be able to compile this module you need [LibSass](https://github.com/sass/libsass) available. For a list of tested, and therefore more or less supported, combinations of LibSass and NGINX versions please refer to the travis environment in `.travis.yml`. Using different versions can result in unexpected behaviour or won't work at all.

Also ldconfig has to find the library:

```shell
ldconfig -p | grep "libsass"
```

If it does not show up try to rebuild the index first using *ldconfig* as *root* user and rerun the grep command. Sometimes you need to add the path where LibSass is installed manually to the list of paths ldconfig is looking for libraries in.

This can be done by adding the path to the *$LD\_LIBRARY\_PATH* variable or the file */etc/ld.so.conf*. Either way, please ensure the command above finds your local LibSass installation before proceeding.

During compilation the header file `sass.h` has to be available. The files included inside this file are also required. The exact list depends on your LibSass version.

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

# configure as static module
./configure \
  --add-module=/projects/public/lua-nginx-module \
  --add-module=/projects/private/sass-nginx-module

# configure as dynamic module
./configure \
  --add-module=/projects/public/lua-nginx-module \
  --add-dynamic-module=/projects/private/sass-nginx-module

make install
```

## Configuration

__Note__: If you have compiled a dynamic module you need to also add the `load_module` directive with the appropriate directory.

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

Using a rewrite ensures all the magic happens under the hood and you do not have to change your application to load different files.

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

Indented Syntax (SASS):

```nginx
location / {
    sass_is_indented_syntax  off; # default
    sass_is_indented_syntax  on;
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

Source Map (embedded):

```nginx
location / {
    sass_source_map_embed  off; # default
    sass_source_map_embed  on;
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

If you fulfill the prerequisites you can use the script `./compile_and_test.sh` to download, compile and test in on go:

```shell
VER_LIBSASS=3.6.3 \
    VER_LUA_NGINX=0.10.15 \
    VER_NGINX=1.16.1 \
    LUAJIT_LIB=/usr/lib/x86_64-linux-gnu/ \
    LUAJIT_INC=/usr/include/luajit-2.0/ \
    ./compile_and_test.sh
```

The three passed variables `VER_LIBSASS`, `VER_LUA_NGINX` and `VER_NGINX` define the module versions your are using for compilation. If a variable is not passed to the script it will be automatically taken from your environment. An error message will be printed if no value is available.

Running the compilation and testing using a dynamic module is possible by additionally passing `DYNAMIC=true` to the script.

All dependencies will automatically be downloaded to the `./vendor` subfolder.

To skip the compilation (and download) step you can pass the `--nocompile` flag:

```shell
ALL_THE_CONFIGURATION_VARIABLES \
    ./compile_and_test.sh --nocompile
```

Please be aware that (for now) all the variables are still required for the script to run.

If you want to only run a single test from the testing folder you can pass it as a parameter to the script (and therefore on to `prove`):

```shell
# single test
ALL_THE_CONFIGURATION_VARIABLES \
    ./compile_and_test.sh t/conf_output-style.t

# single test and --nocompile
ALL_THE_CONFIGURATION_VARIABLES \
    ./compile_and_test.sh --nocompile t/conf_output-style.t
```

## License

Licensed under the [BSD 2 Clause License](https://opensource.org/licenses/BSD-2-Clause).
