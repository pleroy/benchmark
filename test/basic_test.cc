
#include "benchmark/benchmark.h"

#define BASIC_BENCHMARK_TEST(x) BENCHMARK(x)->Arg(8)->Arg(512)->Arg(8192)

void BM_empty(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(state.iterations());
  }
}
BENCHMARK(BM_empty);
BENCHMARK(BM_empty)->ThreadPerCpu();

void BM_spin_empty(benchmark::State& state) {
  for (auto _ : state) {
    for (int x = 0; x < state.range(0); ++x) {
      benchmark::DoNotOptimize(x);
    }
  }
}
BASIC_BENCHMARK_TEST(BM_spin_empty);
BASIC_BENCHMARK_TEST(BM_spin_empty)->ThreadPerCpu();

void BM_spin_pause_before(benchmark::State& state) {
  for (int i = 0; i < state.range(0); ++i) {
    benchmark::DoNotOptimize(i);
  }
  for (auto _ : state) {
    for (int i = 0; i < state.range(0); ++i) {
      benchmark::DoNotOptimize(i);
    }
  }
}
BASIC_BENCHMARK_TEST(BM_spin_pause_before);
BASIC_BENCHMARK_TEST(BM_spin_pause_before)->ThreadPerCpu();

void BM_spin_pause_during(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    for (int i = 0; i < state.range(0); ++i) {
      benchmark::DoNotOptimize(i);
    }
    state.ResumeTiming();
    for (int i = 0; i < state.range(0); ++i) {
      benchmark::DoNotOptimize(i);
    }
  }
}
BASIC_BENCHMARK_TEST(BM_spin_pause_during);
BASIC_BENCHMARK_TEST(BM_spin_pause_during)->ThreadPerCpu();

void BM_pause_during(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_pause_during);
BENCHMARK(BM_pause_during)->ThreadPerCpu();
BENCHMARK(BM_pause_during)->UseRealTime();
BENCHMARK(BM_pause_during)->UseRealTime()->ThreadPerCpu();

void BM_spin_pause_after(benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i < state.range(0); ++i) {
      benchmark::DoNotOptimize(i);
    }
  }
  for (int i = 0; i < state.range(0); ++i) {
    benchmark::DoNotOptimize(i);
  }
}
BASIC_BENCHMARK_TEST(BM_spin_pause_after);
BASIC_BENCHMARK_TEST(BM_spin_pause_after)->ThreadPerCpu();

void BM_spin_pause_before_and_after(benchmark::State& state) {
  for (int i = 0; i < state.range(0); ++i) {
    benchmark::DoNotOptimize(i);
  }
  for (auto _ : state) {
    for (int i = 0; i < state.range(0); ++i) {
      benchmark::DoNotOptimize(i);
    }
  }
  for (int i = 0; i < state.range(0); ++i) {
    benchmark::DoNotOptimize(i);
  }
}
BASIC_BENCHMARK_TEST(BM_spin_pause_before_and_after);
BASIC_BENCHMARK_TEST(BM_spin_pause_before_and_after)->ThreadPerCpu();

void BM_empty_stop_start(benchmark::State& state) {
  for (auto _ : state) {
  }
}
BENCHMARK(BM_empty_stop_start);
BENCHMARK(BM_empty_stop_start)->ThreadPerCpu();


void BM_KeepRunning(benchmark::State& state) {
  benchmark::IterationCount iter_count = 0;
  assert(iter_count == state.iterations());
  while (state.KeepRunning()) {
    ++iter_count;
  }
  assert(iter_count == state.iterations());
}
BENCHMARK(BM_KeepRunning);

void BM_KeepRunningBatch(benchmark::State& state) {
  // Choose a batch size >1000 to skip the typical runs with iteration
  // targets of 10, 100 and 1000.  If these are not actually skipped the
  // bug would be detectable as consecutive runs with the same iteration
  // count.  Below we assert that this does not happen.
  const benchmark::IterationCount batch_size = 1009;

  static benchmark::IterationCount prior_iter_count = 0;
  benchmark::IterationCount iter_count = 0;
  while (state.KeepRunningBatch(batch_size)) {
    iter_count += batch_size;
  }
  assert(state.iterations() == iter_count);

  // Verify that the iteration count always increases across runs (see
  // comment above).
  assert(iter_count == batch_size            // max_iterations == 1
         || iter_count > prior_iter_count);  // max_iterations > batch_size
  prior_iter_count = iter_count;
}
// Register with a fixed repetition count to establish the invariant that
// the iteration count should always change across runs.  This overrides
// the --benchmark_repetitions command line flag, which would otherwise
// cause this test to fail if set > 1.
BENCHMARK(BM_KeepRunningBatch)->Repetitions(1);

void BM_RangedFor(benchmark::State& state) {
  benchmark::IterationCount iter_count = 0;
  for (auto _ : state) {
    ++iter_count;
  }
  assert(iter_count == state.max_iterations);
}
BENCHMARK(BM_RangedFor);

// Ensure that StateIterator provides all the necessary typedefs required to
// instantiate std::iterator_traits.
static_assert(std::is_same<
  typename std::iterator_traits<benchmark::State::StateIterator>::value_type,
  typename benchmark::State::StateIterator::value_type>::value, "");
