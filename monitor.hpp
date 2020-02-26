#ifndef __monitor_h
#define __monitor_h

#include<pthread.h>

class Monitor {
public:
	Monitor() {
        pthread_mutex_init(&mutex, nullptr);
    }
    ~Monitor() {
        pthread_mutex_destroy(&mutex);
    }

	void enter() {
		pthread_mutex_lock(&mutex);
	}

	void leave() {
		pthread_mutex_unlock(&mutex);
	}

	void wait(pthread_cond_t &cond) {
        pthread_cond_wait(&cond, &mutex);
	}

	void signal(pthread_cond_t &cond) {
		pthread_cond_signal(&cond);
	}

private:
	pthread_mutex_t mutex;
};

#endif