#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;

use constant DIR => '/data/dmi/dmi_sounds';

my @ids = do {
  opendir my $dh, DIR or die "Can't read ", DIR, "\n";
  sort { $a <=> $b } map { get_id( $_ ) } grep { !/^\./ } readdir $dh;
};

my $rep     = load_report( \*STDIN );
my $diff    = find_pairs( $rep );
my @missing = grep { !exists $diff->{$_} } @ids;
print scalar( @missing ), " missing matche(s)\n";

sub find_pairs {
  my $rep  = shift;
  my %diff = ();
  for my $corr ( @$rep ) {
    my %rec1 = map { get_id( $_ ) => $_ } @{ $corr->{rec1} };
    my %rec2 = map { get_id( $_ ) => $_ } @{ $corr->{rec2} };
    # Keys of %all are union of keys of %rec1 and %rec2
    my %all = %rec1;
    $all{$_} = 1 for keys %rec2;
    for my $id ( keys %all ) {
      if ( exists $rec1{$id} && exists $rec2{$id} ) {
        $diff{$id} = $corr->{correlation};
      }
    }
  }
  return \%diff;
}

sub get_id {
  my $name = shift;
  die "Can't understand $name\n"
   unless $name =~ /(\d+)(?:\.\w+)+$/;
  return $1;
}

sub load_report {
  my $fh = shift;

  my $state = 'INIT';
  my @rep   = ();
  my $rec   = undef;

  my %next = (
    INIT => 'REC1',
    REC1 => 'REC2',
    REC2 => 'INIT'
  );

  my $stash = sub {
    push @rep, $rec if $rec;
    undef $rec;
  };

  LINE: while ( defined( my $line = <$fh> ) ) {
    chomp $line;
    if ( 'INIT' eq $state ) {
      $stash->();
      die "Correlation is non-numeric\n"
       unless $line =~ /^\d+$/;
      $rec = { correlation => $line };
      $state = 'REC1';
    }
    elsif ( $state =~ /^REC/ ) {
      unless ( length $line ) {
        $state = $next{$state}
         or die "Bad state: $state\n";
        next LINE;
      }
      push @{ $rec->{ lc $state } }, $line;
    }
    else {
      die "Bad state: $state\n";
    }
  }
  $stash->();
  return \@rep;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

