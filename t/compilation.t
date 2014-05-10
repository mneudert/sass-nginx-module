use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;

# proceed with testing
repeat_each(2);
plan tests => repeat_each() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: runtime compilation
--- config
    location /default.css {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile  on;

        rewrite  ^(.*)\.css$  $1.scss  break;
    }
--- request
    GET /default.css
--- response_body_like eval
["background-color: white", "color: black"]
