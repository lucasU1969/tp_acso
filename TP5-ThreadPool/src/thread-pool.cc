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
        // lock_guard<mutex> lg(wts[worker_id].workerLock);
        wts[worker_id].ts = thread([this, worker_id]() {worker(worker_id);});                                   // lanzar los hilos worker
        wts[worker_id].is_available = true;
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

void ThreadPool::dispatcher() {
    while (true) {
        function <void(void)> curr_thunk; 
        {
            lock_guard<mutex> lg(queueLock);
            new_thunk_enqueued_cv.wait(queueLock, [this]() {return thunk_queue.size() > (size_t) 0 || done;});  // espero a que se encole una tarea o que done sea true
            if (done) return;                                                                                   // si el wait funciona bien no puede pasar que haya tareas en la cola y que done sea true
            curr_thunk = thunk_queue.front();                                                                       // obtengo el thunk que voy a asignar a un worker desocupado.
            thunk_queue.pop();
        }
        
        available_workers_semaphore.wait();                                                                     // espero a que algún hilo worker se desocupe.

        int available_worker_id; 
        available_workers_mutex.lock();
        available_worker_id = available_workers_queue.front();                                                  // obtengo un id de worker disponible
        available_workers_queue.pop(); 
        available_workers_mutex.unlock();

        wts[available_worker_id].workerLock.lock();
        wts[available_worker_id].is_available = false;                                                          // marco como ocupado
        wts[available_worker_id].thunk = curr_thunk;                                                            // le paso la tarea que tiene que ejecutar
        wts[available_worker_id].availability_cv.notify_all();                                                  // lo despierto
        wts[available_worker_id].workerLock.unlock();
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        function<void(void)> thunk_to_exec;
        
        wts[id].workerLock.lock();
        wts[id].availability_cv.wait(wts[id].workerLock, [this, id]() {return done || !wts[id].is_available;}); // mientras esté disponible espero
        thunk_to_exec = wts[id].thunk;
        wts[id].workerLock.unlock();
        
        if (done) return;
        
        thunk_to_exec();                                                                                        // ejecuto el thunk
        
        wts[id].workerLock.lock();
        wts[id].is_available = true;                                                                            // vuelvo a estar disponible
        wts[id].workerLock.unlock();
        
        available_workers_mutex.lock();
        available_workers_queue.push(id);                                                                       // me pusheo como worker disponible
        available_workers_semaphore.signal();                                                                   // aviso que un worker terminó
        available_workers_mutex.unlock();

        queueLock.lock();
        task_count--;
        if (task_count == 0) {
            wait_cv.notify_all();                                                                               // aviso a wait que ya no hay más tareas por ejecutar.
        }
        queueLock.unlock();
    }
}

void ThreadPool::wait() {
    queueLock.lock();
    wait_cv.wait(queueLock, [this]() {return task_count == 0;});
    queueLock.unlock();
}

ThreadPool::~ThreadPool() {
    wait(); 

    done = true;
    
    new_thunk_enqueued_cv.notify_all();                                                                         // notifico al dispatcher de que se va a destruir el pool para que salga del loop.
    
    for (size_t i = 0; i < wts.size(); i++) {
        lock_guard<mutex> lg(wts[i].workerLock);
        wts[i].availability_cv.notify_all();                                                                    // notifico a todos los workers
    }

    // esto podría no funcionar
    dt.join();                                                                                                  // libero los recursos del dispatcher 
    for (size_t i = 0; i < wts.size(); i++) wts[i].ts.join();                                                   // libero los recursos de los workers
}