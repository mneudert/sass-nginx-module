use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $fixture_dir;
my $include_dir;

if (defined($ENV{TRAVIS_BUILD_DIR})) {
    $fixture_dir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'fixtures');
    $include_dir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'includes');
} else {
    $fixture_dir = File::Spec->catdir(html_dir(), '..', '..', 'fixtures');
    $include_dir = File::Spec->catdir(html_dir(), '..', '..', 'includes');
}

$ENV{TEST_NGINX_FIXTURE_DIR} = $fixture_dir;
$ENV{TEST_NGINX_INCLUDE_DIR} = $include_dir;

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

=== TEST 1: custom include path
--- main_config eval: $::http_config
--- config
    location /conf_include-path.scss {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_include_path  $TEST_NGINX_INCLUDE_DIR;
    }
--- request
    GET /conf_include-path.scss
--- response_body_like: background-color: "black"
