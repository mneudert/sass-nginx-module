use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $FixtureDir;
my $IncludeDir;

if (defined($ENV{TRAVIS_BUILD_DIR})) {
    $FixtureDir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'fixtures');
    $IncludeDir = File::Spec->catdir($ENV{TRAVIS_BUILD_DIR}, 't', 'includes');
} else {
    $FixtureDir = File::Spec->catdir(html_dir(), '..', '..', 'fixtures');
    $IncludeDir = File::Spec->catdir(html_dir(), '..', '..', 'includes');
}

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;
$ENV{TEST_NGINX_INCLUDE_DIR} = $IncludeDir;

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

=== TEST 1: custom include path
--- main_config eval: $::HttpConfig
--- config
    location /conf_include-path.scss {
        root  $TEST_NGINX_FIXTURE_DIR;

        sass_compile       on;
        sass_include_path  $TEST_NGINX_INCLUDE_DIR;
    }
--- request
    GET /conf_include-path.scss
--- response_body_like: background-color: "black"
