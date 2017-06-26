use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $fixture_dir;

if (defined($ENV{TRAVIS_BUILD_DIR})) {
    $fixture_dir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'fixtures');
} else {
    $fixture_dir = File::Spec->catdir(html_dir(), '..', '..', 'fixtures');
}

$ENV{TEST_NGINX_FIXTURE_DIR} = $fixture_dir;

my $fixture_config = (defined $ENV{DYNAMIC}) ? '_nginx-dynamic.conf' : '_nginx-static.conf';
my $fixture_http   = File::Spec->catfile($fixture_dir, $fixture_config);

open(my $fh, '<', $fixture_http) or die "cannot open < $fixture_http: $!";
read($fh, our $http_config, -s $fh);
close $fh;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * blocks() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: default precision
--- main_config eval: $::http_config
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_precision.scss
--- response_body
.precision-element {
  width: 1.95312px; }

=== TEST 2: custom precision
--- main_config eval: $::http_config
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile    on;
        sass_precision  3;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /conf_precision.scss
--- response_body
.precision-element {
  width: 1.953px; }
