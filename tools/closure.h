/* BASENAME.h */

#ifndef __BASENAME_H
#define __BASENAME_H

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

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c 
 */
