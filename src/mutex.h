#ifndef _MUTEX_H
#define _MUTEX_H

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>
#include <list>
namespace server_cc{

    class Semaphore{
        public:
            Semaphore(uint32_t count=0);
            ~Semaphore();

            void wait();
            void notify();

        private:
            sem_t m_semaphore;

    };
template<class T>
class ScopedLock{

    public:
        ScopedLock(T& mutex) : m_mutex(mutex){
            m_mutex.lock();
            m_locked = true;
        }
        ~ScopedLock() {
            unlock();
        }   
        void lock(){
            if(!m_locked){
                m_mutex.lock();
                m_locked = true;

            }
        }

        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked =false;
            }
        }
    
    private:
        T& m_mutex;
        bool m_locked;

};


class Mutex{
    public:
        typedef ScopedLock<Mutex> Lock;
        Mutex(){
            pthread_mutex_init(&m_mutex, NULL);

        }

        ~Mutex(){
            pthread_mutex_destroy(&m_mutex);
        }

        void lock(){
            pthread_mutex_lock(&m_mutex);
        }

        void unlock(){
            pthread_mutex_unlock(&m_mutex);

        }

    private:
        pthread_mutex_t m_mutex;
};
}
#endif // !_MUTEH#d