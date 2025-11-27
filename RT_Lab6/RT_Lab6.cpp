#include <coroutine>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>

struct task;

struct promise_type
{
    int current_value = 0;

    task get_return_object();

    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int value)
    {
        current_value = value;
        return {};
    }
};

struct task
{
    std::coroutine_handle<promise_type> handle;

    task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~task() { if (handle) handle.destroy(); }

    void resume() { handle.resume(); }
    bool done() const { return handle.done(); }
    int get_value() const { return handle.promise().current_value; }
};

task promise_type::get_return_object()
{
    return task{ std::coroutine_handle<promise_type>::from_promise(*this) };
}

namespace std
{
    template<>
    struct coroutine_traits<task, int>
    {
        using promise_type = ::promise_type;
    };
}

task long_computation(int steps)
{
    for (int i = 1; i <= steps; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        co_yield i;
    }
    co_return;
}

void print_progress(int current, int total)
{
    float percent = static_cast<float>(current) / total * 100.f;
    int bar_width = 50;
    std::string bar;

    size_t pos = static_cast<size_t>(percent / 100.f * bar_width);

    std::string my_name = "Грайр";

    for (size_t i = 0; i < bar_width; ++i) {
        if (i < pos) {
            bar += my_name[i % my_name.length()];
        }
        else {
            bar += ' ';
        }
    }

    std::cout << "\r[" << bar << "] "
        << std::fixed << std::setprecision(1) << percent << "%";
    std::cout.flush();
}

int main()
{
    setlocale(LC_ALL, "RU");
    const int TOTAL_STEPS = 100;

    auto coro = long_computation(TOTAL_STEPS);

    while (!coro.done())
    {
        coro.resume();
        if (coro.done()) break;

        int progress = coro.get_value();
        print_progress(progress, TOTAL_STEPS);
    }

    std::cout << "\nDONE" << std::endl;
    return 0;
}