#ifndef FIBER_INCLUDED_4FF59572_11DC_4131_AEF6_7E7686AB5A20
#define FIBER_INCLUDED_4FF59572_11DC_4131_AEF6_7E7686AB5A20


/* ------------------------------------------------------- [ Fiber Types ] -- */
/*
A fiber represents and execution context, these can be freely switched with
minimal performance overhead.
*/


struct roa_fiber;

typedef void(*roa_fiber_func)(void*);


/* ---------------------------------------------------- [ Fiber Lifetime ] -- */
/*
Create a fiber by passing it a function to execute. If you wish a fiber that
represents the local context simply pass 0/NULL in the 'func' paramater in
the create function.
*/


void
roa_fiber_create(struct roa_fiber **fiber, roa_fiber_func func, void *arg);


void
roa_fiber_destroy(struct roa_fiber *fiber);


/* -------------------------------------------------- [ Fiber Attributes ] -- */


void
roa_fiber_set_user_id(struct roa_fiber *fiber, unsigned user_id);


unsigned
roa_fiber_get_user_id(const struct roa_fiber *fiber);


/* ----------------------------------------------------- [ Fiber Actions ] -- */
/*
Switch from one execution context to another context. Both contexts must be
valid.
*/


void
roa_fiber_switch(struct roa_fiber *this_fiber, struct roa_fiber *new_fiber);


#endif /* inc guard */
