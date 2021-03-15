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
    std::unique_lock<std::mutex> ulock(_mutex);
    _cond.wait(ulock, [this]{ return !_queue.empty(); });

    T obj = std::move(_queue.back());
    _queue.pop_back();
    return obj;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> glock(_mutex);
    _queue.push_back(msg);
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::PHASE_RED;
}

TrafficLight::~TrafficLight() {}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase curPhase = PHASE_RED;
    while(true) {
        curPhase = _queue.receive();
        if(curPhase == PHASE_GREEN) break;
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
    int cycleTimeMs = ((rand() % 3) + 4) * 1000;
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this, cycleTimeMs);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases(int cycleTimeMs)
{
    std::chrono::_V2::system_clock::time_point prevTime = std::chrono::system_clock::now();
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    while (true) {
        std::chrono::_V2::system_clock::time_point curTime = std::chrono::system_clock::now();
        auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - prevTime).count();
        //int cycleTimeMs = ((rand() % 3) + 4) * 1000;

        // if the delta exceeds the cycle time, change the phase
        if(deltaMs > cycleTimeMs) {
            // grab the mutex
            std::unique_lock<std::mutex>(_mutex);

            // update the phase and send it to the queue
            _currentPhase = (_currentPhase == PHASE_RED) ? PHASE_GREEN : PHASE_RED;
            _queue.send(std::move(_currentPhase));

            // update previous time
            prevTime = curTime;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
