void process_work(long niter) {
    for (long i = 0; i < niter; i++) {
        sqrt(rand());
    }
}

int main(int argc, char *argv[]) {
    process_work(1e9);
    return 0;
}