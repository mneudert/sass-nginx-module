use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * blocks() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: default comments
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        body_filter_by_lua 'ngx.arg[1] = string.sub(ngx.arg[1], 1, -2) .. "\\n"';
    }
--- request
    GET /conf_source-comments.scss
--- response_body
html {
  background-color: black; }

body {
  color: white; }


=== TEST 2: comments "off"
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile          on;
        sass_source_comments  off;

        body_filter_by_lua 'ngx.arg[1] = string.sub(ngx.arg[1], 1, -2) .. "\\n"';
    }
--- request
    GET /conf_source-comments.scss
--- response_body
html {
  background-color: black; }

body {
  color: white; }


=== TEST 3: comments "on"
--- config
    location ~ ^.*\.scss$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile          on;
        sass_source_comments  on;
    }
--- request
    GET /conf_source-comments.scss
--- response_body_like: /* line 1,
