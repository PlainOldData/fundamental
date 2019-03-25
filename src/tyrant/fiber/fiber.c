#include <fiber/fiber.h>
#include <boost_context/fcontext.h>
#include <lib/assert.h>
#include <lib/alloc.h>
#include <stdlib.h>


/* ------------------------------------------------------- [ Fiber Types ] -- */


struct roa_fiber
{
        void *stack;
        size_t sys_page_size;
        size_t stack_size;
        fcontext_t context;
        void *arg;

        unsigned job_id;
};


/* ---------------------------------------------------- [ Fiber Lifetime ] -- */


void
roa_fiber_create(struct roa_fiber **fiber, roa_fiber_func func, void *arg)
{
        const int fiber_bytes = sizeof(struct roa_fiber);
        struct roa_fiber *new_fiber = (struct roa_fiber*)calloc(1, fiber_bytes);

        if (func != 0) {
                int stack_size = 524288;

                int bytes = stack_size + stack_size + stack_size;

                new_fiber->stack = calloc(1, bytes);
                char *start = ((char*)new_fiber->stack) + stack_size;
                new_fiber->context = make_fcontext(start, stack_size, func);

                new_fiber->arg = arg;
        }
        else {
                fcontext_t local = NULL;
                new_fiber->context = local;
        }

        *fiber = new_fiber;
}


void
roa_fiber_destroy(struct roa_fiber *fiber)
{
        if (fiber->stack) {
                free(fiber->stack);
                fiber->stack = NULL;
        }

        if (fiber) {
                free(fiber);
        }

        fiber->context = NULL;
}


/* -------------------------------------------------- [ Fiber Attributes ] -- */


void
roa_fiber_set_user_id(struct roa_fiber *fiber, unsigned user_id)
{
        /* param check */
        LIB_ASSERT(fiber);

        fiber->job_id = user_id;
}


unsigned
roa_fiber_get_user_id(const struct roa_fiber *fiber)
{
        /* param check */
        LIB_ASSERT(fiber);

        return fiber->job_id;
}


/* ----------------------------------------------------- [ Fiber Actions ] -- */


void
roa_fiber_switch(struct roa_fiber *this_fiber, struct roa_fiber *new_fiber)
{
        jump_fcontext(&this_fiber->context, new_fiber->context, new_fiber->arg);
}
