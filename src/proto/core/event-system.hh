#pragma once
#include "proto/core/containers/DynamicArray.hh"
#include "proto/core/memory/common.hh"

namespace proto {

template<typename> struct Sink;
template<typename> struct Channel;

template<typename EventType>
struct Sink {
    using Callback = void(*)(EventType&);
    Callback _callback = nullptr; // const?
    Channel<EventType> * _channel = nullptr;

    Sink() {};

    // NOTE(kacper): this template is here just to allow captureless
    // lambdas with Callback sugnature to be passed.
    template<typename CallbackType>
    void init(Channel<EventType>& channel, CallbackType callback) {
        _channel = &channel;
        _callback = callback;
        _channel->attach(this);
    }

    ~Sink() {
        if(_channel)
            _channel->detach(this);
    }

    // in case I will start using private in future
    friend struct Channel<EventType>;
};

template<typename EventType>
struct Channel {
    struct SinkRecord {
        Sink<EventType> * sink;
        EventType mask;
    };
    DynamicArray<SinkRecord> sinks;
    memory::Allocator * _allocator;
    
    void init(size_t init_sink_cap, memory::Allocator * allocator) {
        assert(allocator);
        _allocator = allocator;
        sinks.init(init_sink_cap, _allocator);
    }
    
    void attach(Sink<EventType> * sink) {
        assert(sink);
        sinks.push_back(SinkRecord{sink});
    }

    void emit(EventType event){
        for(size_t i=0; i<sinks.size(); i++) {
            if(sinks[i].sink->_callback)
                sinks[i].sink->_callback(event);
        }
    }

    void detach(Sink<EventType> * sink) {
        for(size_t i=0; i<sinks.size(); i++) {
            if(sinks[i].sink == sink) {
                sinks.erase(i);
                return;
            }
        }
        assert(0);
    }
};


}
