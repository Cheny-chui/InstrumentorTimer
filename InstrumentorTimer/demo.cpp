#include <iostream>

#include "instrumentor_timer.h"


void function(int value) {
        
    BENCHMARK_FUNCTION(); 
    for(int i = 0; i < 1000; i++)
        std::cout << "hello world" << (i + value) << std::endl;
}
void function() {
    BENCHMARK_FUNCTION();
    for(int i = 0; i < 1000; i++)
        std::cout << "hello world" << i << std::endl;
}

void run_benchmark() {
    BENCHMARK_FUNCTION();
    std::cout << "Runint Benchmarks....\n";
    function(2);
    function();
}

int main() {
    Benchmark::Instrumentor::get().begin_session();
    run_benchmark();
    Benchmark::Instrumentor::get().end_session();
    return 0;
}
