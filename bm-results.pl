#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;
use File::Spec;
use List::Util qw( max );

chdir 'bm' or die "Can't cd bm\n";
my @bm_dir = map { $_->[0] }
 sort { $a->[1] <=> $b->[1] || $a->[0] cmp $b->[0] }
 map { [ $_, /baseline/ ? 0 : 1 ] } grep { -d } glob '*';

my $bm   = read_all( @bm_dir );
my %keep = ();
for my $d ( @bm_dir ) {
  $keep{$_}++ for keys %{ $bm->{$d} };
}
my @width
 = ( max( map { length } keys %keep ), map { length } @bm_dir );
printf join( ' | ', map { "%-${_}s" } @width ) . "\n", 'keep', @bm_dir;
for my $k ( sort { $a <=> $b } keys %keep ) {
  printf join( ' | ', map { "%${_}s" } @width ) . "\n", $k,
   map { seconds( $bm->{$_}{$k}{elapsed} ) || 'n/a' } @bm_dir;
}

sub read_all {
  my @dir = @_;
  my %bm  = ();
  for my $d ( @dir ) {
    $bm{$d} = read_bm( $d );
  }
  return \%bm;
}

sub read_bm {
  my $dir = shift;
  my %bm  = ();
  for my $f ( grep -s, glob "$dir/t*.bm" ) {
    next unless $f =~ /(\d+)\.bm$/;
    $bm{$1} = read_time( $f );
  }
  return \%bm;
}

sub read_time {
  my $file = shift;
  open my $fh, '<', $file or die "Can't read $file: $!\n";
  chomp( my $times = <$fh> );
  my %t = split /=/, $times;
  return \%t;
}

sub seconds {
  my $tm = shift or return;
  my @tp = split /:/, $tm;
  my @mu = ( 60, 60, 100 );
  my $se = 0;
  my $mu = 1;
  while ( @tp ) {
    $se += $mu * pop @tp;
    $mu *= shift @mu;
  }
  return sprintf '%.2f', $se;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

