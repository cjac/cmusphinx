# Test to make sure the module loads

BEGIN { $| = 1; print "1..5\n"; }
END {print "not ok 1\n" unless $loaded;}
use Audio::MFCC;
$loaded = 1;
print "ok 1\n";

my $fe = Audio::MFCC->init({
			    sampling_rate => 8000,
			    frame_rate => 80,
			    pre_emphasis_alpha => 0.95,
			   })
    or die "not ok 2\n";
print "ok 2\n";

$fe->start_utt or die "not ok 3\n";
print "ok 3\n";

my $rawdata = pack '@8000';
my @ceps = $fe->process_utt($rawdata, 4000);
if (@ceps) {
    print "processed ", scalar(@ceps), " frames OK\n";
    print "ok 4\n";
} else {
    print "not ok 4\n";
}

my $leftover = $fe->end_utt;
print "leftover ", ($leftover ? 1 : 0), " frames OK\n";
print "ok 5\n";
