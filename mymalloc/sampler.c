#define SAMPLES_SIZE 4096

typedef void (*func_t) (int*, int);

int samples[SAMPLES_SIZE];
int samples_n;
int sampling;
func_t sampling_cb;

void init_samples() {
  samples_n = 0;
  sampling = 1;
}

void register_sampling_cb(func_t f) {
  sampling_cb = f;
}

int cmpfunc(const void * a, const void * b) {
  return ( *(int*)a - *(int*)b );
}


void bin_sample() {
  qsort(samples, samples_n, sizeof(int), cmpfunc);
  int distinct = 1;
  for (int i = 1; i < samples_n; i++) {
    if (samples[i] != samples[i - 1])
      distinct++;
  }

  int *bins = malloc(distinct * sizeof(int));
  int *sizes = malloc(distinct * sizeof(int));

  bins[0] = samples[0];
  sizes[0] = 1;

  distinct = 1;
  for (int i = 1; i < samples_n; i++) {
    if (samples[i] != samples[i - 1]) {
      bins[distinct++] = samples[i];
      sizes[distinct] = 1;
    } else {
      sizes[distinct]++;
    }
  }

  if (sampling_cb)
    sampling_cb(bins, distinct);
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
