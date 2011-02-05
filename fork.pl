#!/usr/bin/env perl

use strict;
use warnings;

use File::Find;
use IO::Handle;
use IO::Select;
use Storable qw( store_fd fd_retrieve dclone );

forkit( 50, @ARGV );

sub forkit {
  my ( $par, @dir ) = @_;

  my %active = ();
  my $select = IO::Select->new;

  for ( 1 .. $par ) {
    my $my_wtr    = IO::Handle->new;
    my $child_rdr = IO::Handle->new;

    pipe $child_rdr, $my_wtr
     or die "Can't open write pipe ($!)\n";

    if ( my $pid = fork ) {
      # Parent
      close $child_rdr;
      $active{$pid}++;
      $select->add( $my_wtr );
    }
    else {
      # Child
      close $my_wtr;

      # Don't execute any END blocks
      use POSIX '_exit';
      eval q{END { _exit 0 }};

      FILE: while () {
        my $msg = fd_retrieve( $child_rdr )->[0];
        last FILE unless defined $msg;
        print "$$:$msg\n";
        sleep 1;
      }

      close $child_rdr;
      # We use CORE::exit for MP compatibility
      CORE::exit;
    }
  }

  my @wtr = $select->can_write;
  find {
    wanted => sub {
      @wtr = $select->can_write unless @wtr;
      store_fd [$_], shift @wtr;
    },
    no_chdir => 1
  }, @dir;

  print ">>>DONE WRITING\n" for 1 .. 5;

  while ( $select->count ) {
    my @wtr = $select->can_write;
    for my $wh ( @wtr ) {
      store_fd [undef], $wh;
      $select->remove( $wh );
      close $wh;
    }
  }

  while ( keys %active ) {
    my $pid = waitpid( -1, 0 );
    delete $active{$pid};
  }
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

