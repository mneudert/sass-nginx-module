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

=== TEST 1: default linefeed
--- config
    location /default.css {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        rewrite  ^(.*)\.css$  $1.scss  break;

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /default.css
--- response_body
body {
  background-color: white;
  color: black; }

=== TEST 2: custom linefeed
--- config
    location /default.scss {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile   on;
        sass_linefeed  "\n/* linefeed */\n";

        header_filter_by_lua_block { ngx.header.content_length = nil }
        body_filter_by_lua_block   { ngx.arg[1] = ngx.arg[1] .. "\n" }
    }
--- request
    GET /default.scss
--- response_body
body {
/* linefeed */
  background-color: white;
/* linefeed */
  color: black; }
/* linefeed */
