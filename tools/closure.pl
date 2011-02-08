#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;

my $spec = parse_spec_file( 'tools/foo.cl' );
my %tpl = ( c => 'tools/closure.c', h => 'tools/closure.h', );

while ( my ( $ext, $tpl ) = each %tpl ) {
  my $name = $spec->{name} . '.' . $ext;
  open my $fh, '>', $name or die "Can't write $name: $!\n";
  print $fh fix_file( $spec, $tpl );
  print "Wrote $name\n";
}

sub fix_file {
  my ( $spec, $file ) = @_;
  my @body = ();
  my $skip = 0;
  open my $fh, '<', $file
   or die "Can't read $file: $!\n";
  my $ctx = make_vars( $spec );
  while ( defined( my $line = <$fh> ) ) {
    chomp $line;
    if ( $line =~ m{^/\*\s*<skip>\s*\*/} ) {
      $skip++;
    }
    elsif ( $line =~ m{^/\*\s*</skip>\s*\*/} ) {
      die "</skip> without matching <skip>\n" unless $skip;
      $skip--;
    }
    elsif ( $line =~ m{^/\*\s*<include\s+block="(\S+)"\s*/>\s*\*/} ) {
      my $block = $1;
      die "Unknown block: $block\n" unless exists $ctx->{$block};
      push @body, ( delete $ctx->{$block} )->();
    }
    else {
      push @body, $line unless $skip;
    }
  }
  my $body = join "\n", @body;
  my $patn = '('
   . join( '|', sort { length $b <=> length $a } keys %$ctx ) . ')';
  $body =~ s/$patn/$ctx->{$1}()/eg;
  return $body;
}

sub make_vars {
  my $spec      = shift;
  my $call_args = sub {
    my $slot = shift;
    join ', ',
     map { $_->{is_context} ? "data[$slot].$_->{name}" : $_->{name} }
     @{ $spec->{args} };
  };
  return {
    NAME      => sub { $spec->{name} },
    BASENAME  => sub { $spec->{name} },
    ALL_PROTO => sub {
      join ', ', map { $_->{proto} } @{ $spec->{args} };
    },
    CTX_ARGS => sub {
      join ', ', map { $_->{name} }
       grep { $_->{is_context} } @{ $spec->{args} };
    },
    CLEANUP_ARGS => sub {
      join ', ', map { "data[i].$_" } map { $_->{name} }
       grep { $_->{is_context} } @{ $spec->{args} };
    },
    CTX_COPY_STMT => sub {
      join ";\n", map { "data[s].$_ = $_" }
       map  { $_->{name} }
       grep { $_->{is_context} } @{ $spec->{args} };
    },
    CTX_PROTO => sub {
      join ', ', map { $_->{proto} }
       grep { $_->{is_context} } @{ $spec->{args} };
    },
    CTX_PROTO_STMT => sub {
      join ";\n", map { $_->{proto} }
       grep { $_->{is_context} } @{ $spec->{args} };
    },
    PASS_PROTO => sub {
      join ', ', map { $_->{proto} }
       grep { !$_->{is_context} } @{ $spec->{args} };
    },
    RETURN             => sub { $spec->{return} },
    NSLOTS             => sub { $spec->{slots} },
    INCLUDE_HEADER     => sub { '#include "BASENAME.h"' },
    CLOSURE_PROTOTYPES => sub {
      map { "static RETURN closure_$_( PASS_PROTO );" }
       0 .. $spec->{slots} - 1;
    },
    CLOSURE_TABLE => sub {
      join ",\n",
       map { "  {" . ( $_ + 1 ) . ", closure_$_, NULL, NULL}" }
       0 .. $spec->{slots} - 1;
    },
    CLOSURE_DEFINITIONS => sub {
      map {
        "static RETURN closure_$_( PASS_PROTO ) { return slot[$_].code( "
         . $call_args->( $_ ) . " ); }"
      } 0 .. $spec->{slots} - 1;
    },
  };
}

sub parse_args {
  my @args = split_args( @_ );
  my @spec = ();
  for my $arg ( @args ) {
    my $is_context = !!( $arg =~ s/^context\s+// );
    push @spec,
     {
      proto      => $arg,
      is_context => $is_context,
      name       => get_name( $arg ),
     };
  }
  return @spec;
}

sub get_name {
  my $proto = shift;
  # These need a lot more work to be general...
  $proto =~ s/\([^\)]*\)$//;
  $proto =~ s/\(([^\)]*)\)/$1/g;
  die "Can't extract name from $proto\n" unless $proto =~ /(\w+)$/;
  return $1;
}

sub split_args {
  my $args = shift;
  my @hide = ();
  $args =~ s/
  ( (?: \( [^\)]* \) ) |
    (?: \[ [^\]]* \] ) )
  / do { push @hide, $1; "[$#hide]" } /xeg;
  my @arg = split /,/, $args;
  for ( @arg ) {
    s/\[(\d+)\]/$hide[$1]/eg;
    s/^\s+//;
    s/\s+$//;
    s/\s+/ /g;
  }
  return @arg;
}

sub parse_spec_file {
  my $name = shift;
  open my $fh, '<', $name or die "Can't read $name: $!\n";
  return parse_spec( $fh );
}

sub parse_spec {
  my $fh   = shift;
  my $spec = {};
  while ( defined( my $line = <$fh> ) ) {
    chomp $line;
    next if $line =~ /^\s*$/;
    next if $line =~ /^\s*#/;
    if ( $line =~ /^(\w+)\s*[:=]\s*(.*)/ ) {
      my ( $k, $v ) = ( $1, $2 );
      $v =~ s/\s+$//;
      $spec->{$k} = $v;
    }
    elsif ( $line =~ /^(.*?)\s+(\w+)\s*\((.*)\)\s*;\s*$/ ) {
      my ( $ret, $name, $args ) = ( $1, $2, $3 );
      $spec->{name}   = $name;
      $spec->{return} = $ret;
      $spec->{args}   = [ parse_args( $args ) ];
    }
    else {
      die "Can't parse $line\n";
    }
  }
  return $spec;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

