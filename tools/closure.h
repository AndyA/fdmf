/* BASENAME.h */

#ifndef __BASENAME_H
#define __BASENAME_H

#if defined( THREADED_CLOSURES ) || defined( THREADED_NAME )
#include <pthread.h>
#endif

/* <skip> */
#define ALL_PROTO       int x, void *h
#define CTX_PROTO       void *h
#define CTX_PROTO_STMT  void *h
#define PASS_PROTO      int x
#define CALL_ARGS(n)    x, slot[n].h
#define RETURN          int
#define SLOTS           2
/* </skip> */

#define NAME_SLOTS      SLOTS

typedef RETURN( *NAME ) ( PASS_PROTO );

NAME new_NAME_cleanup( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
                       void ( *cleanup ) ( CTX_PROTO ) );
NAME new_NAME( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO );
void free_NAME( NAME cl );

#if defined( THREADED_CLOSURES ) || defined( THREADED_NAME )

NAME new_NAME_cleanup_nts( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
                           void ( *cleanup ) ( CTX_PROTO ) );
NAME new_NAME_nts( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO );
void free_NAME_nts( NAME cl );

NAME new_NAME_nb( RETURN( *code ) ( ALL_PROTO ), CTX_PROTO,
                  void ( *cleanup ) ( CTX_PROTO ), unsigned timeout );

#endif

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
