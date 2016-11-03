#include <stdlib.h>
#include "./util.h"
#include "./sampler.h"

#define SAMPLES_SIZE 1024 // tunable
#define K 7 // tunable
#define OUTLIER_CUTOFF 5 // pretty realistic

#define INF (1<<30)

static int samples[SAMPLES_SIZE];
static int samples_n;
static int sampling;
func_t sampling_cb;

void init_samples() {
  samples_n = 0;
  sampling = 1;
}

void register_sampling_cb(func_t f) {
  sampling_cb = f;
}

static
int cmpfunc(const void * a, const void * b) {
  return ( *(int*)a - *(int*)b );
}


static int bins[SAMPLES_SIZE];
static int sizes[SAMPLES_SIZE];
static int dp[SAMPLES_SIZE][K + 1];
static int cnt[SAMPLES_SIZE][K + 1];
static int best_bins[K + 1];
static int bins_sizes[K + 1];

static
void bin_sample() {
  qsort(samples, samples_n, sizeof(int), cmpfunc);
  int distinct = 1;
  for (int i = 1; i < samples_n; i++) {
    if (samples[i] != samples[i - 1])
      distinct++;
  }

  bins[0] = samples[0];
  sizes[0] = 1;

  distinct = 1;
  for (int i = 1; i < samples_n; i++) {
    if (samples[i] != samples[i - 1]) {
      bins[distinct] = samples[i];
      sizes[distinct++] = 1;
    } else {
      sizes[distinct - 1]++;
    }
  }

  // remove all sizes that were represented by a small number of samples
  int n = 0;
  for (int i = 0; i < distinct; i++) {
    if (sizes[i] > OUTLIER_CUTOFF) {
      bins[n] = bins[i];
      sizes[n] = sizes[i];
      n++;
    }
  }

  // run a dp to determine the best way to pick K bins to fit all samples
  // with the minimum internal fragmentation O(nK)
  int m = min(K, n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= m; j++) {
      dp[i][j] = INF;
      cnt[i][j] = 0;
    }
  }
  dp[0][1] = 0;
  cnt[0][1] = sizes[0];

  D(
  for (int i = 0; i < n; i++) {
    printf("%d ", bins[i]);
  }
  printf(":bins\n");
  for (int i = 0; i < n; i++) {
    printf("%d ", sizes[i]);
  }
  printf(":sizes\n");
  )

  for (int i = 1; i < n; i++) {
    for (int j = 1; j <= m; j++) {
      // use the same bin as the previous
      int coal_cost = dp[i-1][j] + cnt[i-1][j] * (bins[i] - bins[i-1]);
      if (dp[i][j] > coal_cost) {
        dp[i][j] = coal_cost;
        cnt[i][j] = cnt[i-1][j] + sizes[i];
      }
      // use a new bin
      if (dp[i][j] > dp[i-1][j-1]) {
        dp[i][j] = dp[i-1][j-1];
        cnt[i][j] = sizes[i];
      }
    }
  }

  D(
  for (int i = 0; i < n; i++) {
    for (int j = 1; j <= m; j++) {
      printf("%d(%d)%c", dp[i][j], cnt[i][j], " \n"[j == m]);
    }
  })

  int t = m;
  best_bins[t - 1] = bins[n - 1];
  for (int i = n - 1; i >= 0; i--) {
    if (i && dp[i][t] == dp[i-1][t-1]) {
      t--;
      best_bins[t - 1] = bins[i-1];
      bins_sizes[t - 1] = sizes[i-1];
    }
  }

  if (sampling_cb)
    sampling_cb(best_bins, bins_sizes, m);
}

void add_sample(int x) {
  if (!sampling)return;
  if (samples_n == SAMPLES_SIZE) {
    sampling = 0;
    bin_sample();
  } else {
    samples[samples_n++] = x;
  }
}
