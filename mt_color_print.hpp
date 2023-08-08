#ifndef MT_COLOR_PRINT_HPP
#define MT_COLOR_PRINT_HPP

#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>


namespace mt_color_print {

    // PRIVATE IMPLEMENTATION DETAILS:
    namespace details {

        std::mutex mut;
        bool color_enabled = true;
        const char * reset_color = "\033[0m";

        // return the "next color" (wrapping around) in the 4-bit ANSI color sequence
        // @see https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
        // not thread-safe
        const char * next_color()
        {
            static const char * colors[]{"31","32","33","34","35","36","37","90","91","92","93","94","95","96","97"};

            static auto it = std::cbegin(colors);

            if( it == std::cend(colors) ) {
                it = std::cbegin(colors);
            }
            return *it++;

        }

        // return the color for a thread
        // not thread-safe
        const char * thread_color(const std::thread::id & thread_id)
        {
            static std::unordered_map<std::thread::id,const char *> thread_color_map;

            // If thread_id has already been assigned a color, return that color. Otherwise, assign a color and return it
            auto it = thread_color_map.find(thread_id);
            if( it == thread_color_map.cend() ) {
                return (thread_color_map[thread_id] = next_color());
            } else {
                return it->second;
            }

        }

        // return a (perhaps colorized) string representation of thread_id, between brackets
        // not thread-safe
        std::string thread_id_str(const std::thread::id & thread_id)
        {
            // we use a stream (vs just appending stuff to a plain string) because std::thread::id is only serializable via operator<<(std::thread::id)
            // @see https://en.cppreference.com/w/cpp/thread/thread/id/operator_ltlt
            
            std::ostringstream ss;

            if( color_enabled ) {
                ss << "\033[" << thread_color(thread_id) << 'm';
            }
            ss << '[' << thread_id << ']';

            return ss.str();
        }

    }   // namespace mt_color_print::details
    // END OF PRIVATE IMPLEMENTATION DETAILS


    // PUBLIC INTERFACE:

    /**
    *   Enable/disable coloring of output
    *
    *   @param      enable  enable coloring of output
    *   @note       thread-safe
    *   @warning    enable/disable for every thread
    */
    void set_color_enabled(bool enable)
    {
        using namespace details;

        std::lock_guard<std::mutex> lock{mut};
        color_enabled = enable;
    }

    /**
    *   Tell if coloring of output is enabled
    *
    *   @return     whether coloring of output is enabled
    *   @note       thread-safe
    *   @warning    this is for every thread
    */
    bool is_color_enabled()
    {
        using namespace details;

        std::lock_guard<std::mutex> lock{mut};
        return color_enabled;
    }

    /**
    *   Print items in the iterator range [from,to) to `std::cout`, without being interrupted by other threads, and preceded by a (perhaps colorized) string representation of the thread's id, between brackets. Each item on its own line
    *
    *   @tparam Iter    input iterator type
    *   @param  from    iterator pointing to the first element
    *   @param  to      iterator pointing to one past the last element
    *   @note   thread-safe
    */
    template<typename Iter>
    void print_range(Iter from, Iter to)
    {
        using namespace details;

        std::scoped_lock<std::mutex> lock{mut};

        std::cout << thread_id_str(std::this_thread::get_id()) << '\n';

        while( from != to ) {
            std::cout << *from++ << '\n';
        }

        std::cout << reset_color << std::endl;        // reset color (no harm even if we had not set it before), then newline & flush
    }

    /**
    *   Print `args` to `std::cout`, without being interrupted by other threads, and preceded by a (perhaps colorized) string representation of the thread's id, between brackets. Print a space between each adjacent arg
    *
    *   @tparam Ts          printable types (i.e. having an extraction to \c std::ostream operator overload)
    *   @param  ... args    items to be printed
    *   @note               thread-safe
    */  
    template<typename ... Ts>
    void print(Ts && ... args)
    {
        using namespace details;

        std::lock_guard<std::mutex> lock{mut};

        auto nArgs = sizeof...(Ts);
        decltype(nArgs) i = 1;

        auto print_one = [& nArgs, & i](auto && x){
            std::cout << x;
            if( i++ <  nArgs ) {
                std::cout << ' ';
            }
        };

        std::cout << thread_id_str(std::this_thread::get_id()) << ' ';

        ( print_one(std::forward<Ts>(args)), ... );

        std::cout << reset_color << std::endl;        // reset color (no harm even if we had not set it before), then newline & flush
    }

}   // namespace mt_color_print


#endif  // MT_COLOR_PRINT_HPP
