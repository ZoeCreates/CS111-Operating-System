# Hash Hash Hash
This project is making a hash table implementation safe for concurrent use. The two versions v1 and v2 have been implemented, each using different locking strategies to ensure thread safety. Performance comparisons with the base (serial) implementation are included.

## Building
```shell
make
```

## Running
```shell
./hash-table-tester
```

You can customize the number of threads and the number of entries per thread using the -t and -s flags, respectively:

```shell
./hash-table-tester -t 8 -s 50000
```

## First Implementation
In the hash_table_v1_add_entry function, I added a single global mutex (pthread_mutex_t) to the entire hash table. This mutex locks the hash table during any modification (e.g., adding or updating an entry) to prevent data races. The mutex is initialized in the hash_table_v1_create function and destroyed in the hash_table_v1_destroy function.


### Performance
To measure the performance of version 1, run the following command:

```shell
./hash-table-tester -t 8 -s 50000
```

Version 1 is generally slower than the base version. This is because the single mutex introduces a bottleneck, where only one thread can modify the hash table at a time. The overhead of acquiring and releasing the lock also contributes to the slower performance.

## Second Implementation
In the hash_table_v2_add_entry function, I implemented a finer-grained locking strategy. Instead of using a single global mutex, each hash table bucket has its own mutex. This allows multiple threads to modify different buckets concurrently, reducing contention and improving performance.


### Performance
```shell
./hash-table-tester -t 8 -s 50000
```

Version 2 shows improved performance compared to version 1, particularly when multiple threads are used. The fine-grained locking allows greater concurrency, as threads are less likely to block each other. In cases where different keys map to different buckets, the performance can approach that of the base version, or even surpass it in multi-core environments.

To analyze the speedup provided by version 2, compare the results of the base, version 1, and version 2 implementations under different thread counts. The performance improvement in version 2 is more significant with higher numbers of threads, demonstrating the effectiveness of fine-grained locking.



## Cleaning up
```shell
make clean
```
This will remove the compiled hash-table-tester executable and any other build artifacts.


