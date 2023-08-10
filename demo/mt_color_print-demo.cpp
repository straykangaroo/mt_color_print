#include "mt_color_print.hpp"

#include <chrono>
#include <future>
#include <vector>
#include <thread>


int main()
{
    using namespace std::chrono_literals;

    mt_color_print::set_color_enabled(true);

    std::vector<std::future<void>> futures;
    for( int th = 0; th != 5 ; th++ ) {
        futures.emplace_back(std::async(std::launch::async,[]()
        {
            for( int i = 0; i != 4; i++ ) {
                mt_color_print::print("hello from some thread");
                std::this_thread::sleep_for(i * 597ms);
            }
        }));
        std::this_thread::sleep_for(933ms / (th + 3));
    }

    for( auto & f : futures ) {
        f.wait();
    }

    return 0;
}
