/* BASENAME.c */

/* <skip> */
#include "closure.h"
/* </skip> */
/* <include block="INCLUDE_HEADER" /> */

#include <stdlib.h>
#include <stdio.h>

struct closure_slot {
  unsigned next;
  NAME cl;
   RETURN( *code ) ( ALL_PROTO );
  void ( *cleanup ) ( CTX_PROTO );
   CTX_PROTO_STMT;
};

/* <skip> */
static RETURN closure_0( PASS_PROTO );
static RETURN closure_1( PASS_PROTO );
/* </skip> */
/* <include block="CLOSURE_PROTOTYPES" /> */

static struct closure_slot slot[NAME_SLOTS] = {
/* <skip> */
  {1, closure_0, NULL, NULL, NULL},
  {2, closure_1, NULL, NULL, NULL},
/* </skip> */
/* <include block="CLOSURE_TABLE" /> */
};

static unsigned free_slot = 0;
static unsigned order_known = 0;

#if defined( THREADED_CLOSURES ) || defined( THREADED_NAME )
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t try_again = PTHREAD_COND_INITIALIZER;
#define new_closure_cleanup new_NAME_cleanup_nts
#define new_closure         new_NAME_nts
#define free_closure        free_NAME_nts
#else
#define new_closure_cleanup new_NAME_cleanup
#define new_closure         new_NAME
#define free_closure        free_NAME
#endif

/* <skip> */
static RETURN
closure_0( PASS_PROTO ) {
  return slot[0].code( CALL_ARGS( 0 ) );
}

static RETURN
closure_1( PASS_PROTO ) {
  return slot[1].code( CALL_ARGS( 1 ) );
}

/* </skip> */
/* <include block="CLOSURE_DEFINITIONS" /> */

NAME
new_closure_cleanup( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
                     void ( *cleanup ) ( CTX_PROTO ) ) {
  unsigned s;
  if ( free_slot == NAME_SLOTS )
    return NULL;
  s = free_slot;
  free_slot = slot[s].next;
  slot[s].next = NAME_SLOTS + 1;
  slot[s].code = code;
  slot[s].cleanup = cleanup;
  slot[s].h = h;

  return slot[s].cl;
}

NAME
new_closure( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO ) {
  return new_NAME_cleanup( code, CTX_ARGS, NULL );
}

static unsigned
are_ordered( void ) {
  unsigned long long last_cl = 0;
  unsigned i;
  for ( i = 0; i < NAME_SLOTS; i++ ) {
    unsigned long long cl = ( unsigned long long ) slot[i].cl;
    if ( cl < last_cl )
      return 2;
    last_cl = cl;
  }
  return 1;
}

static void
bad_free( void ) {
  fprintf( stderr, "Attempt to free unallocated closure" );
  exit( 1 );
}

static int
by_addr( const void *a, const void *b ) {
  const struct closure_slot *csa = ( const struct closure_slot * ) a;
  const struct closure_slot *csb = ( const struct closure_slot * ) b;
  unsigned long long cla = ( unsigned long long ) csa->cl;
  unsigned long long clb = ( unsigned long long ) csb->cl;
  return cla < clb ? -1 : cla > clb ? 1 : 0;
}

void
free_closure( NAME cl ) {
  unsigned i;

  if ( !order_known )
    order_known = are_ordered(  );

  if ( order_known == 1 ) {
    struct closure_slot key = {
      0, cl, NULL, NULL, NULL
    };
    struct closure_slot *sl =
        bsearch( &key, slot, NAME_SLOTS, sizeof( struct closure_slot ),
                 by_addr );
    if ( sl ) {
      i = sl - slot;
      goto free_it;
    }
  }
  else if ( order_known == 2 ) {
    for ( i = 0; i < NAME_SLOTS; i++ ) {
      if ( slot[i].cl == cl ) {
        goto free_it;
      }
    }
  }

  bad_free(  );

free_it:
  if ( slot[i].next != NAME_SLOTS + 1 )
    bad_free(  );
  if ( slot[i].cleanup ) {
    slot[i].cleanup( slot[i].h );
    slot[i].cleanup = NULL;
  }
  slot[i].next = free_slot;
  free_slot = i;
}

#if defined( THREADED_CLOSURES ) || defined( THREADED_NAME )
NAME
new_NAME_cleanup( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
                  void ( *cleanup ) ( CTX_PROTO ) ) {
  return new_NAME_nb( code, cleanup, 0 );
}

NAME
new_NAME( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO ) {
  return new_NAME_nb( code, NULL, 0 );
}

void
free_NAME( NAME cl ) {
  pthread_mutex_lock( lock );
  free_closure( cl );
  pthread_mutex_unlock( lock );
  pthread_cond_signal( try_again );
}

NAME
new_NAME_nb( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
             void ( *cleanup ) ( CTX_PROTO ), unsigned timeout ) {
  NAME cl;
  pthread_mutex_lock( lock );
  if ( free_slot == SLOTS ) {
    /* TODO do something other than wait forever if the timeout != 0 */
    if ( timeout == 0 ) {
      pthread_mutex_unlock( lock );
      return NULL;
    }
    pthread_cond_wait( try_again, lock );
  }
  cl = new_closure_cleanup( code, cleanup );
  pthread_mutex_unlock( lock );
  return cl;
}

#endif

/* <skip> */

static RETURN
printer( ALL_PROTO ) {
  printf( ( char * ) h, x );
  printf( "\n" );
  return x * 2;
}

static void
cleanup( CTX_PROTO ) {
  printf( "cleaning up %s\n", ( char * ) h );
}

int
main( void ) {
  NAME cl1 = new_NAME_cleanup( printer, ( void * ) "hello %d", cleanup );
  NAME cl2 = new_NAME( printer, ( void * ) "world %d" );
  cl1( 1 );
  cl1( 2 );
  cl2( 3 );
  cl2( 4 );
  free_NAME( cl1 );
  cl1 = new_NAME( printer, ( void * ) "whoop %d" );
  cl1( 1 );
  cl1( 2 );
  cl2( 3 );
  cl2( 4 );
  return 0;
}

/* </skip> */

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
