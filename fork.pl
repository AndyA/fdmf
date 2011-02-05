#!/usr/bin/env perl

use strict;
use warnings;

use File::Find;
use IO::Handle;

forkit( 5, $_ ) for @ARGV;

sub forkit {
  my ( $par, @dir ) = @_;

  my $my_wtr    = IO::Handle->new;
  my $child_rdr = IO::Handle->new;

  my %active = ();

  for ( 1 .. $par ) {

  }

}

# vim:ts=2:sw=2:sts=2:et:ft=perl

