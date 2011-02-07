#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;

my @args = parse_args( "int x, context void *h" );

my %spec = (
  name   => 'foo',
  return => 'int',
  args   => \@args,
);

print Dumper( \%spec );
print make_typedef( \%spec );

sub make_typedef {
  my $spec = shift;

  return
   "typedef $spec->{return} (*$spec->{name})("
   . pass_proto( $spec ) . ");";
}

sub all_proto {
  my $spec = shift;
  return join( ', ', map { $_->{proto} } @{ $spec->{args} } );
}

sub ctx_proto {
  my $spec = shift;
  return join( ', ',
    map { $_->{proto} } grep { $_->{is_context} } @{ $spec->{args} } );

}

sub pass_proto {
  my $spec = shift;
  return join( ', ',
    map { $_->{proto} } grep { !$_->{is_context} } @{ $spec->{args} } );

}

=for ref

typedef int (*closure)(int x, int (*foo)(int dc[], void *h));

closure new_closure( int (*coderef)(int x, void *h, int (*foo)(int dc[], void *h), void *h) {
}

void free_closure( closure c ) {
}

static int closure_0(int x, int (*foo)(int dc[], void *h)) {
  return slot[0].code( x, slot[0].h, foo );
}

=cut

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

# vim:ts=2:sw=2:sts=2:et:ft=perl

