use File::Basename;
use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $test_dir    = File::Spec->rel2abs(dirname(__FILE__));
my $fixture_dir = File::Spec->catdir($test_dir, 'fixtures');

my $fixture_config = (defined $ENV{DYNAMIC}) ? '_nginx-dynamic.conf' : '_nginx-static.conf';
my $fixture_http   = File::Spec->catfile($fixture_dir, $fixture_config);

$ENV{TEST_NGINX_FIXTURE_DIR} = $fixture_dir;

open(my $fh, '<', $fixture_http) or die "cannot open < $fixture_http: $!";
read($fh, our $http_config, -s $fh);
close $fh;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * blocks() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: default syntax (== not indented)
--- main_config eval: $::http_config
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /default.scss
--- response_body
body {
  background-color: white;
  color: black; }

=== TEST 2: indented syntax "off"
--- main_config eval: $::http_config
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile             on;
        sass_is_indented_syntax  off;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /default.scss
--- response_body
body {
  background-color: white;
  color: black; }

=== TEST 3: indented syntax "on"
--- main_config eval: $::http_config
--- config
    location ~ ^.*\.sass$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile             on;
        sass_is_indented_syntax  on;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_is-indented-syntax.sass
--- response_body
body {
  background-color: white;
  color: black; }
