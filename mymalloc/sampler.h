typedef void (*func_t) (int*, int*, int);

#define SAMPLES_SIZE 64 // tunable
#define K 9 // tunable

void init_samples();
void register_sampling_cb(func_t f);
void add_sample(int x);
