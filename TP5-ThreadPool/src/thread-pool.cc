/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : 
    wts(numThreads), 
    done(false), 
    available_workers_semaphore(numThreads), 
    task_count(0)
{
    dt = thread([this]() {dispatcher();});                                                                      // lanzar el hilo dispatcher
    for (int worker_id = 0; worker_id < (int) numThreads; worker_id++) {
        wts[worker_id].ts = thread([this, worker_id]() {worker(worker_id);});                                   // lanzar los hilos worker
        wts[worker_id].is_available = true;
        wts[worker_id].thunk = nullptr;
        available_workers_queue.push(worker_id);
    }
}


void ThreadPool::schedule(const function<void(void)>& thunk) {                                                  // notifico a dispatcher cada vez que se encola una función
    lock_guard<mutex> lg(queueLock);
    if (done || !thunk) throw "Error";
    thunk_queue.push(thunk);                        
    new_thunk_enqueued_cv.notify_all();                                                                         // el notify se tiene que hacer igual para despertar al dispatcher si está dormido (si no lo hago tengo que esperar a un spurious wakeup del dispatcher)
    task_count++;
}

void ThreadPool::worker(int id) {
    while (true) {
        function<void(void)> thunk_to_exec;
        wts[id].workerLock.lock();
        while (!done && wts[id].thunk == nullptr) wts[id].availability_cv.wait(wts[id].workerLock);             // mientras no se haya llamado al destructor o mi thunk sea null espero
        if (done) {
            wts[id].workerLock.unlock();
            return;
        }
        thunk_to_exec = wts[id].thunk;                                                                          // obtengo el thunk a ejecutar
        wts[id].thunk = nullptr;
        wts[id].workerLock.unlock();
        
        thunk_to_exec();                                                                                        // ejecuto el thunk
        
        available_workers_mutex.lock();
        available_workers_queue.push(id);                                                                       // me pusheo como worker disponible
        available_workers_semaphore.signal();                                                                   // aviso para que se despierte dispatcher
        available_workers_mutex.unlock();
        
        queueLock.lock();
        task_count--;                                                                                           // menos tareas ejecutandose
        if (task_count == 0) {
            wait_cv.notify_all();                                                                               // aviso a wait que no quedan tareas
        }
        queueLock.unlock();
    }
}

void ThreadPool::wait() {
    queueLock.lock();
    while (task_count != 0) wait_cv.wait(queueLock);
    queueLock.unlock();
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> curr_thunk;
        
        queueLock.lock();
        while (thunk_queue.empty() && !done) new_thunk_enqueued_cv.wait(queueLock);                             // espero mientras no llamen al destructor o la cola esté vacía
        if (done && thunk_queue.empty()) {                                                                      // tengo que salir
            queueLock.unlock();
            return;
        }

        curr_thunk = thunk_queue.front();                                                                       // tarea a ejecutar
        thunk_queue.pop();
        queueLock.unlock();
        
        available_workers_semaphore.wait();                                                                     // espero a que haya un worker disponible
        
        int available_worker_id;
        available_workers_mutex.lock();
        available_worker_id = available_workers_queue.front();                                                  // veo el primer worker disponible
        available_workers_queue.pop();
        available_workers_mutex.unlock();
        
        wts[available_worker_id].workerLock.lock();
        wts[available_worker_id].thunk = curr_thunk;                                                            // paso al tarea al worker seleccionado
        wts[available_worker_id].availability_cv.notify_one();                                                  // le aviso
        wts[available_worker_id].workerLock.unlock();
    }
}

ThreadPool::~ThreadPool() {
    wait();

    done = true;
    
    new_thunk_enqueued_cv.notify_all();                                                                         // notifico al dispatcher de que se va a destruir el pool para que salga del loop.
    
    for (size_t i = 0; i < wts.size(); i++) {
        lock_guard<mutex> lg(wts[i].workerLock);
        wts[i].availability_cv.notify_all();                                                                    // notifico a todos los workers
    }

    dt.join();                                                                                                  // libero los recursos del dispatcher 
    for (size_t i = 0; i < wts.size(); i++) wts[i].ts.join();                                                   // libero los recursos de los workers
}