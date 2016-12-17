use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $FixtureDir;

if (defined($ENV{TRAVIS_BUILD_DIR})) {
    $FixtureDir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'fixtures');
} else {
    $FixtureDir = File::Spec->catdir(html_dir(), '..', '..', 'fixtures');
}

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;

my $FixtureConfig = (defined $ENV{DYNAMIC}) ? '_nginx-dynamic.conf' : '_nginx-static.conf';
my $FixtureHttp   = File::Spec->catfile($FixtureDir, $FixtureConfig);

open(my $fh, '<', $FixtureHttp) or die "cannot open < $FixtureHttp: $!";
read($fh, our $HttpConfig, -s $fh);
close $fh;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * blocks() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: default output style
--- main_config eval: $::HttpConfig
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_output-style.scss
--- response_body
.output {
  background-color: white; }
  .output .with-style {
    color: black; }

=== TEST 2: output style "compact"
--- main_config eval: $::HttpConfig
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_output_style  compact;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_output-style.scss
--- response_body
.output { background-color: white; }

.output .with-style { color: black; }

=== TEST 3: output style "compressed"
--- main_config eval: $::HttpConfig
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_output_style  compressed;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_output-style.scss
--- response_body
.output{background-color:#fff}.output .with-style{color:#000}

=== TEST 4: output style "expanded"
--- main_config eval: $::HttpConfig
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_output_style  expanded;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_output-style.scss
--- response_body
.output {
  background-color: white;
}

.output .with-style {
  color: black;
}

=== TEST 5: output style "nested"
--- main_config eval: $::HttpConfig
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_output_style  nested;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_output-style.scss
--- response_body
.output {
  background-color: white; }
  .output .with-style {
    color: black; }
