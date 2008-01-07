# Test to make sure the module loads

BEGIN { $| = 1; print "1..18\n"; }
END {print "not ok 1\n" unless $loaded;}
use Audio::SPX;
$loaded = 1;
print "ok 1\n";

my $ad = Audio::SPX->open_sps(16000); # 8000 "not supported" (!)
unless (defined($ad)) {
    print "Audio not supported, skipping tests\n";
    exit 0;
}
print "ok 2\n";

$ad->start_rec
    or die "not ok 3\n";
print "ok 3\n";

my $rbuf;
my $samples = $ad->read($rbuf, 256);
sleep 1;
$samples = $ad->read($rbuf, 256)
    or die "not ok 4\n";
print "read $samples samples OK\n";
print "ok 4\n";

my $cad = Audio::SPX::Continuous->init($ad)
    or die "not ok 5\n";
print "ok 5\n";

$cad->calib_loop($rbuf)
    or die "not ok 6\n";
print "ok 6\n";

$cad->calib
    or die "not ok 7\n";
print "ok 7\n";

$samples = $cad->read($rbuf, 256)
    or die "not ok 8\n";
print "read $samples samples OK\n";
print "ok 8\n";

my @params = $cad->get_params
    or die "not ok 9\n";
print "params are @params OK\n";
print "ok 9\n";

$cad->set_params(@params)
    or die "not ok 10\n";
print "ok 10\n";

$cad->detach
    or die "not ok 11\n";
print "ok 11\n";

$cad->attach($ad)
    or die "not ok 12\n";
print "ok 12\n";

my $ts = $cad->read_ts;
die "not ok 13\n" unless defined $ts;
print "read_ts is $ts OK\n";
print "ok 13\n";

print "Please speak into the microphone.\n";
eval {
    local $SIG{ALRM} = sub { die "timeout" };
    alarm 5;
    while (($samples = $cad->read($rbuf, 256)) == 0) {
	select undef, undef, undef, 0.02;
    }
};
if ($@ =~ /timeout/) {
    print "Read timed out, skipping test\n";
    print "ok 14\n";
} else {
    die "not ok 14\n" unless defined $samples;
    print "read $samples samples OK\n";
    print "ok 14\n";
}

my $ts2 = $cad->read_ts;
die "not ok 15\n" unless defined $ts2;
print "read_ts is $ts2 OK\n";
print "ok 15\n";

die "not ok 16\n" unless $ts2 != $ts;
print "ok 16\n";

$cad->reset
    or die "not ok 17\n";
print "ok 17\n";

$ad->stop_rec
    or die "not ok 18\n";
print "ok 18\n";

