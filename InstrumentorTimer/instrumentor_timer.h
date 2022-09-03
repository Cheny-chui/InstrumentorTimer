//
// Basic instrumentation profiler by Cherno and Gavin
// ------------------------
// Modified by Chui
// 1. re-write whole project in my code style for learning cpp
//  a. make 'WriteProfile' thread-safe by add 'std::mutex' and 'std::lock_guard'
//  b. change the write_foot and write_head fuction to in-line code
//  c. add displayTimeUnit to json file
//  d. make instrumentor singleton
//  e. remove InstrumentationSession
// ------------------------

// Usage: include this header file somewhere in your code (e.g. precompiled header), and then use like:
//
// Benchmark::Instrumentor::get().begin_session();          // Begin session
// {   
//     // Place add code like this in scopes you'd like to include in profiling or you can use the macros in line 34
//     Benchmark::InstrumentationTimer timer("Profiled Scope Name");   
//     // Code
// }
// Benchmark::Instrumentor::get().end_session();            // End Session
//
//
#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

#define BENCHMARK_MACRO 1
#if BENCHMARK_MACRO
#define BENCHMARK_SCOPE(name) Benchmark::InstrumentationTimer timer__LINE__(name)
#ifdef _MSC_VER
#define BENCHMARK_FUNCTION() BENCHMARK_SCOPE(__FUNCSIG__)
#else
#define BENCHMARK_FUNCTION() BENCHMARK_SCOPE(__PRETTY_FUNCTION__)
#endif
#else
#define BENCHMARK_SCOPE(name)
#define BENCHMARK_FUNCTION()
#endif

namespace Benchmark
{
    struct EventAttr {
        std::string_view name;
        long long start;
        long long dur;
        size_t thread_id;
    };

    class Instrumentor {
    private:
        friend class InstrumentationTimer;

        std::ofstream m_ofstream;
        std::mutex m_lock;
        size_t m_event_count{};
        Instrumentor() {}

        Instrumentor(const Instrumentor&) = delete;

        void write_profile(const EventAttr& event) {
            std::lock_guard<std::mutex> lockGuard(m_lock);

            if(m_event_count++ > 0)
                m_ofstream << ",";

            std::string name = event.name.data();
            std::replace(name.begin(), name.end(), '"', '\'');

            m_ofstream << "{";
            m_ofstream << R"("name": ")" << name << R"(",)"
                << R"("cat": "function",)"
                << R"("ph": "X",)"
                << R"("ts": )" << event.start << ","
                << R"("dur": )" << event.dur << ","
                << R"("pid": 0,)"
                << R"("tid": )" << event.thread_id;
            m_ofstream << "}";

            m_ofstream.flush();
        }

    public:
        static Instrumentor& get() {
            static Instrumentor instance;
            return instance;
        }
        void begin_session(std::string_view filepath = "result.json") {
            m_ofstream.open(filepath.data());
            m_ofstream << R"({ "displayTimeUnit": "ms", "traceEvents": [ )";
            m_ofstream.flush();
        }

        void end_session() {
            m_ofstream << R"( ]})";
            m_ofstream.flush();
        }

    };

    class InstrumentationTimer {
    private:
        std::string_view m_name;
        std::chrono::time_point<std::chrono::steady_clock> m_start_time;

        void stop() {
            auto end_time = std::chrono::steady_clock::now();
            auto dur = end_time - m_start_time;
            long long dur_count = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
            EventAttr event{m_name, std::chrono::duration_cast<std::chrono::microseconds>(m_start_time.time_since_epoch()).count(), dur_count, std::hash<std::thread::id>{}(std::this_thread::get_id())};
            Instrumentor::get().write_profile(event);
        }
    public:
        ~InstrumentationTimer() {
            stop();
        }
        InstrumentationTimer(const char* name):
            m_name(name), m_start_time(std::chrono::steady_clock::now()) {}
    };
}