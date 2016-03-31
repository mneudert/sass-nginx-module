use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');
my $IncludeDir = File::Spec->catfile($HtmlDir, '..', '..', 'includes');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;
$ENV{TEST_NGINX_INCLUDE_DIR} = $IncludeDir;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * blocks() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: custom include path
--- config
    location /conf_include-path.scss {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_include_path  $TEST_NGINX_INCLUDE_DIR;
    }
--- request
    GET /conf_include-path.scss
--- response_body_like: background-color: "black"
