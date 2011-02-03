#!perl

use strict;
use warnings;

use File::Spec;

use Test::More;
use Test::Differences;

use constant TESTS => ( 1 );

plan tests => TESTS * 1;

for my $t ( TESTS ) {
  test( "test$t" );
}

sub test {
  my $test = shift;
  my ( $db, $ref )
   = map { File::Spec->catfile( 't', 'data', "$test.$_" ) } 'db', 'ref';

  open my $ph, '-|', "./fdmf_dump --db $db | ./fdmf_correlator"
   or die "Can't run pipe: $!\n";
  chomp( my @got = <$ph> );
  close $ph or die "Can't run pipe: $!\n";

  my @want = slurp( $ref );

  eq_or_diff \@got, \@want, "$test: output matches";
}

sub slurp {
  my $file = shift;
  open my $fh, '<', $file or die "Can't open $file: $!\n";
  chomp( my @l = <$fh> );
  return @l;
}

# vim:ts=2:sw=2:et:ft=perl

