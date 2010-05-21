# -*- cperl -*-

# Copyright (c) 2006 Carnegie Mellon University.  All rights
# reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#
# This work was supported in part by funding from the Defense Advanced
# Research Projects Agency and the National Science Foundation of the
# United States of America, and the CMU Sphinx Speech Consortium.
#
# THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
# ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
# NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

use strict;
package Text::CMU::LMTraining;
use vars qw($VERSION);
$VERSION='0.99.0';

require Text::CMU::InputFilter::CMU;
require Text::CMU::InputFilter::ICSI;
require Text::CMU::InputFilter::ISL;
require Text::CMU::InputFilter::NIST;
require Text::CMU::InputFilter::LETSGO;
require Text::CMU::InputFilter::SWB;
require Text::CMU::InputFilter::HUB5;
require Text::CMU::InputFilter::MS98;

require Text::CMU::Vocabulary;
require Text::CMU::NGramModel;
require Text::CMU::NGramFactory;
require Text::CMU::Smoothing;

1;
__END__

=head1 NAME

Text::CMU::LMTraining - Module for language model training

=head1 SYNOPSIS

 use Text::CMU::LMTraining;

=head1 DESCRIPTION

The main purpose of this module is to "pull in" all of the other
language training modules.  In particular, all C<InputFilter> modules,
C<Text::CMU::NGramModel>, C<Text::CMU::NGramFactory>,
C<Text::CMU::Smoothing>, and C<Text::CMU::Vocabulary>.  See their
individual manual pages for more information.

=head1 SEE ALSO

L<Text::CMU::NGramFactory>, L<Text::CMU::NGramModel>,
L<Text::CMU::Vocabulary>, L<Text::CMU::Smoothing>,
L<Text::CMU::InputFilter>

=head1 AUTHORS

David Huggins-Daines E<lt>dhuggins@cs.cmu.eduE<gt>

=cut
