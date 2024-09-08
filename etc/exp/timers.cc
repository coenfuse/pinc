#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

std::string getCurrentTimeFormatted()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % std::chrono::seconds(1);
    // auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration) % std::chrono::seconds(1);

    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count(); // 9 digits for nanoseconds

    return ss.str();
}

class TimeEvent
{
public:
    TimeEvent(const std::chrono::milliseconds &age, const std::string &action, const std::string &name)
        : m_name(name),
          m_action(action),
          m_is_expired(false),
          m_expiry(std::chrono::steady_clock::now() + age)
    {
    }

    void set_is_expired() { m_is_expired = true; }
    bool get_is_expired() const { return m_is_expired; }
    std::chrono::steady_clock::time_point get_expiry() const { return m_expiry; }
    const std::string &get_name() const { return m_name; }
    void trigger_action() const
    {
        std::cout << m_action << " @ " << getCurrentTimeFormatted() << std::endl;
    }

private:
    std::string m_action;
    std::string m_name;
    std::chrono::steady_clock::time_point m_expiry;
    bool m_is_expired;
};

bool has_expired(const std::chrono::steady_clock::time_point &expiry)
{
    return std::chrono::steady_clock::now() >= expiry;
}

int main()
{
    TimeEvent ev1(std::chrono::milliseconds(20000), "T1 triggered", "T1");
    TimeEvent ev2(std::chrono::milliseconds(3), "T2 triggered", "T2");
    TimeEvent ev3(std::chrono::milliseconds(1), "T3 triggered", "T3");
    TimeEvent ev4(std::chrono::milliseconds(5), "T4 triggered", "T4");
    TimeEvent ev5(std::chrono::milliseconds(15), "T5 triggered", "T5");

    std::vector<TimeEvent> TimersSet = {ev1, ev2, ev3, ev4, ev5};

    bool all_events_complete = false;

    std::cout << "T0 started   @ " << getCurrentTimeFormatted() << std::endl;

    while (true)
    {
        for (auto &event : TimersSet)
        {
            if (event.get_is_expired())
            {
                all_events_complete = true;
            }
            else
            {
                all_events_complete = false;
                if (has_expired(event.get_expiry()))
                {
                    event.trigger_action();
                    event.set_is_expired();
                }
            }
        }
    }
}
