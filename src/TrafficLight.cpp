#include <iostream>
#include <random>
#include <thread>
#include <future>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    std::cout << "Watiing for green..." << std::endl;
    while (true)
    {
        TrafficLightPhase message = _trafficLightQueue->receive();
        if (message == TrafficLightPhase::green) {
            std::cout << "Traffic light is green" << std::endl;
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread t(&TrafficLight::cycleThroughPhases, this);
    t.join();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    _trafficLightQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
    std::vector<std::future<void>> futures;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(4, 6);
    while (true)
    {
        auto startTime = std::chrono::steady_clock::now();

        std::this_thread::sleep_for(std::chrono::seconds(distribution(generator)));
        
        if (_currentPhase == TrafficLightPhase::red) {
            _currentPhase = TrafficLightPhase::green;
        } else {
            _currentPhase = TrafficLightPhase::red;
        }
        _trafficLightQueue->send(std::move(_currentPhase));
        auto endTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "cycle elapased time: " << elapsedTime.count() << " milliscends" << std::endl;

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}