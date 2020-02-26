#include<iostream>
#include<random>
#include<string>
#include<vector>
#include<pthread.h>
#include<unistd.h>

#include"monitor.hpp"
#include"common_functions.hpp"

class Buffer : Monitor {
public:
    Buffer() {
        m = new Monitor();
        pthread_cond_init(&enough_space, nullptr);
        pthread_cond_init(&enough_products, nullptr);
        buffer_fullness = 0;
    }
    ~Buffer() {
        delete m;
        pthread_cond_destroy(&enough_space);
        pthread_cond_destroy(&enough_products);
    }
    void insert(int &to_insert, int &k) {
        this->enter();
        buffer_fullness = get_buffer_fullness();
        // std::cerr << "insertion: " << buffer_fullness + to_insert << "/" << k << std::endl;
        while (buffer_fullness + to_insert > k) {
            // std::cerr << "producer waiting" << std::endl;
            wait(enough_space);
        }
        buffer_fullness += to_insert;
        set_buffer_fullness(buffer_fullness);
        if (buffer_fullness > k/2) signal(enough_products);
        else if (buffer_fullness <= k/2) signal(enough_space);
        this->leave();
    }
    void remove(int &to_remove, int &k) {
        this->enter();
        buffer_fullness = get_buffer_fullness();
        // std::cerr << "removal: " << buffer_fullness - to_remove << "/" << k << std::endl;
        while (buffer_fullness - to_remove < 0) {
            // std::cerr << "consumer waiting" << std::endl;
            wait(enough_products);
        }
        buffer_fullness -= to_remove;
        set_buffer_fullness(buffer_fullness);
        if (buffer_fullness > k/2) signal(enough_products);
        else if (buffer_fullness <= k/2) signal(enough_space);
        this->leave();
    }

private:
    Monitor *m;
    pthread_cond_t enough_space;
    pthread_cond_t enough_products;
    int buffer_fullness;
} buff;

class Arguments {
public:
    int id;
    int a, b;
    int k;

    Arguments(int _id, int _a, int _b, int _k) : id(_id), a(_a), b(_b), k(_k) {}
};

void *producer(void *args) {
    Arguments *producer_args;
    producer_args = (Arguments *) args;
    std::random_device generator;
    std::uniform_int_distribution<int> producer_distribution(producer_args->a, producer_args->b);
    int to_insert;
    while(1) {
        sleep(1);
        to_insert = producer_distribution(generator);
        log_to_file("Produced " + std::to_string(to_insert) + " elements", Type::PRODUCER, producer_args->id);
        buff.insert(to_insert, producer_args->k);
        log_to_file("Successfuly inserted " + std::to_string(to_insert) + " elements", Type::PRODUCER, producer_args->id);
    }
}

void *consumer(void *args) {
    Arguments *consumer_args;
    consumer_args = (Arguments *) args;
    std::random_device generator;
    std::uniform_int_distribution<int> consumer_distribution(consumer_args->a, consumer_args->b);
    int to_remove;
    while(1) {
        sleep(1);
        to_remove = consumer_distribution(generator);
        log_to_file("Prepared to consume " + std::to_string(to_remove) + " elements", Type::CONSUMER, consumer_args->id);
        buff.remove(to_remove, consumer_args->k);
        log_to_file("Successfuly consumed " + std::to_string(to_remove) + " elements", Type::CONSUMER, consumer_args->id);
    }

}

/*
    k, m, n, a, b, c, d
    k - max capacity
    m - no of producers
    n - no of consumers
    a, b - parameters of the distribution of the no of elements prouduced
    c, d - parameters of the distribution of the no of elements consumed
*/

int main(int argc, char *argv[]) { 

    if (argc != 8) {
        std::cerr << "invalid number of arguments\n";
        return EXIT_FAILURE;
    }

    int k = std::stoi(argv[1]);
    int m = std::stoi(argv[2]), n = std::stoi(argv[3]);
    int a = std::stoi(argv[4]), b = std::stoi(argv[5]);
    int c = std::stoi(argv[6]), d = std::stoi(argv[7]);

    if (k <= 0 || m < 0 || n < 0 || a < 0 || b < 0 || c < 0 || d < 0) {
        std::cerr << "arguments cannot be negative\n";
        return EXIT_FAILURE;
    }
    if (m > 30000 || n > 30000) {
        std::cerr << "numeber of prcesses cannot exceed 30000; are you mad?\n";
        return EXIT_FAILURE;
    }
    
    //run producer threads
    pthread_t producer_thread[m];
    std::vector<Arguments> producer_args(m, Arguments(0, a, b, k));
    for (int i = 0; i < m; ++i) {
        producer_args[i].id = i;
        pthread_create(&producer_thread[i], nullptr, producer, (void*) &producer_args[i]);
    }

    //run consumer threads
    pthread_t consumer_thread[n];
    std::vector<Arguments> consumer_args(n, Arguments(0, c, d, k));
    for (int i = 0; i < n; ++i) {
        consumer_args[i].id = i;
        pthread_create(&consumer_thread[i], nullptr, consumer, (void*) &consumer_args[i]);
    }

    //wait for user to terminate 
    std::string exit;
    std::cin >> exit;
}