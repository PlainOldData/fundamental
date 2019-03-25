#include <tyrant/tyrant.h>

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>


void
some_dummy_work(tyrant_ctx_t, void *) { /* do nothing */ }


void
some_other_dummy_work(tyrant_ctx_t, void *) { /* do nothing */ }


void
double_submit_crash(tyrant_ctx_t ctx, void *)
{
  tyrant_job_desc desc{};
  desc.func = some_dummy_work;

  tyrant_job_submit(ctx, &desc, 1);

  tyrant_job_desc other_desc{};
  other_desc.func = some_other_dummy_work;

  tyrant_job_submit(ctx, &other_desc, 1);
}


void
shutdown_failure(tyrant_ctx_t ctx, void *)
{
  tyrant_job_desc desc{};
  desc.func = some_dummy_work;

  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
  tyrant_job_submit(ctx, &desc, 1);
}


TEST_CASE("Regression Tests")
{
  tyrant_ctx_t ctx{};
  tyrant_ctx_create(&ctx, 0);

  SECTION("double submit crash")
  {
    /*
      Bug: crash (note: if submit same function twice it doesn't crash)
    */

    tyrant_job_desc desc{};
    desc.func = double_submit_crash;

    tyrant_job_submit(ctx, &desc, 1);
    tyrant_ctx_run(ctx);

    REQUIRE(true);
  }

  SECTION("shutdown failure")
  {
    /*
      Bug: dispatcher shutdown doesn't happen.
    */

    tyrant_job_desc desc{};
    desc.func = shutdown_failure;

    tyrant_job_submit(ctx, &desc, 1);
    tyrant_ctx_run(ctx);

    REQUIRE(true);
  }

  tyrant_ctx_destroy(&ctx);
}
