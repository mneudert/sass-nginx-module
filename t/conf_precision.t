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

=== TEST 1: default precision
--- config
    location ~ ^.*\.css$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        rewrite  ^(.*)\.css$  $1.scss  break;

        body_filter_by_lua 'ngx.arg[1] = string.sub(ngx.arg[1], 1, -2) .. "\\n"';
    }
--- request
    GET /conf_precision.css
--- response_body
.precision-element {
  width: 1.95312px; }


=== TEST 2: custom precision
--- config
    location ~ ^.*\.css$ {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile    on;
        sass_precision  3;

        rewrite  ^(.*)\.css$  $1.scss  break;

        body_filter_by_lua 'ngx.arg[1] = string.sub(ngx.arg[1], 1, -2) .. "\\n"';
    }
--- request
    GET /conf_precision.css
--- response_body_like
.precision-element {
  width: 1.953px; }
