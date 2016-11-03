typedef void (*func_t) (int*, int*, int);


void init_samples();
void register_sampling_cb(func_t f);
void add_sample(int x);
