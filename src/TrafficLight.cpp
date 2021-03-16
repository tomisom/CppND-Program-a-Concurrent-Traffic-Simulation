#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    // lock the mutex
    std::unique_lock<std::mutex> ulock(_mutex);

    // wait here for something in the queue
    _cond.wait(ulock, [this]{ return !_queue.empty(); });

    // get the msg object from the back of the queue using move semantics
    T obj = std::move(_queue.back());

    // clear the queue now that we have the reference
    _queue.clear();

    // return the object (will use move semantics)
    return obj;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // lock the mutex
    std::lock_guard<std::mutex> glock(_mutex);

    // add the msg to the front of the queue
    _queue.push_front(msg);

    // notify that we got a msg
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::PHASE_RED) { }

TrafficLight::~TrafficLight() {}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    // initialize a TrafficLightPhase variable to red
    TrafficLightPhase curPhase = PHASE_RED;

    // loop until we get a green light
    while(curPhase != PHASE_GREEN) {
        // get the current phase from the message queue
        /// the msgQueue will manage locking its mutex, so we don't need one here
        /// plus a mutex lock here would break introduce a race condition with cycleThroughPhases
        /// since it needs the lock to send a msg to the msgQueue
        curPhase = _msqQueue.receive();
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::unique_lock<std::mutex>(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

    // set the traffic light cycle time
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::chrono::_V2::system_clock::time_point prevTime = std::chrono::system_clock::now();
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // compute the random delay between 4 and 6 milliseconds for this traffic light
    int cycleTimeMs = ((rand() % 3) + 4) * 1000;

    while (true) {
        // get the current time
        std::chrono::_V2::system_clock::time_point curTime = std::chrono::system_clock::now();

        // compute the time delta between loops
        auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - prevTime).count();

        // if the delta exceeds the random cycle time, change the phase
        if(deltaMs > cycleTimeMs) {
            // lock the mutex
            std::unique_lock<std::mutex> ulock(_mutex);

            // update the phase and send it to the queue
            _currentPhase = (_currentPhase == PHASE_RED) ? PHASE_GREEN : PHASE_RED;
            _msqQueue.send(std::move(_currentPhase));

            // update previous time
            prevTime = curTime;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
