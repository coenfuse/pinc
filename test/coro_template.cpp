#include <coroutine>
#include <iostream>
#include <memory>

void print(const std::string_view msg) {
    std::cout << msg << std::endl;
}


template <typename t_TYPE>
class async;

template <typename t_TYPE>
class coro_state_manager;


template <typename t_TYPE>
class async
{
    public:
        using promise_type = class coro_state_manager<t_TYPE>;
        using t_coro_handle = std::coroutine_handle<promise_type>;
        friend promise_type;

    public:
        bool is_done() const;       // consider them to be set as private
        void resume();              // and only accessible to internal scheduler
        t_TYPE get_result() const;
        bool await_ready() const;
        void await_suspend(std::coroutine_handle<> caller) const;
        t_TYPE await_resume() const;

    private:
        explicit async(t_coro_handle* handle);

    private:
        t_coro_handle* muptr_handle;
};


template <typename t_TYPE>
class coro_state_manager
{
    public:
        using t_coro_handle = typename async<t_TYPE>::t_coro_handle;
        friend class async<t_TYPE>;

    public:
        async<t_TYPE> get_return_object();
        std::suspend_always initial_suspend();
        std::suspend_always final_suspend() noexcept;
        void unhandled_exception() noexcept;
        void return_value(t_TYPE to_return);
        explicit coro_state_manager();
        ~coro_state_manager();

    private:
        std::unique_ptr<t_coro_handle> muptr_handle;
        t_TYPE m_return_value;
};


template <typename t_TYPE>
bool async<t_TYPE>::is_done() const {
    return muptr_handle->done();
}

template <typename t_TYPE>
void async<t_TYPE>::resume() {
    muptr_handle->resume();   
}

template <typename t_TYPE>
t_TYPE async<t_TYPE>::get_result() const {
    return muptr_handle->promise().m_return_value;
}

template <typename t_TYPE>
bool async<t_TYPE>::await_ready() const {
    return is_done();   // need to evaluate this, sometimme a coroutine could be ready to yield but is not done
}

template <typename t_TYPE>
void async<t_TYPE>::await_suspend(std::coroutine_handle<> caller_handle) const {
    this->muptr_handle->resume();           // finish self
    if (this->is_done()) {
        caller_handle.resume();             // then call the caller notifying that I'm done and resume it (this will be later done by the scheduler where coroutine will notify scheduler instead)
    }
}

template <typename t_TYPE>
t_TYPE async<t_TYPE>::await_resume() const {
    return get_result();
}

template <typename t_TYPE>
async<t_TYPE>::async(async<t_TYPE>::t_coro_handle* handle) {
    muptr_handle = handle;
}

template <typename t_TYPE>
async<t_TYPE> coro_state_manager<t_TYPE>::get_return_object() 
{
    muptr_handle = std::make_unique<t_coro_handle>(t_coro_handle::from_promise(*this));
    return async<t_TYPE>(muptr_handle.get());
}

template <typename t_TYPE>
std::suspend_always coro_state_manager<t_TYPE>::initial_suspend() {
    return {};
}

template <typename t_TYPE>
std::suspend_always coro_state_manager<t_TYPE>::final_suspend() noexcept {
    return {};
}

template <typename t_TYPE>
void coro_state_manager<t_TYPE>::unhandled_exception() noexcept {
}

template <typename t_TYPE>
void coro_state_manager<t_TYPE>::return_value(t_TYPE to_return) {
    m_return_value = to_return;
}

template <typename t_TYPE>
coro_state_manager<t_TYPE>::coro_state_manager() {
}

template <typename t_TYPE>
coro_state_manager<t_TYPE>::~coro_state_manager() {
    muptr_handle.get()->destroy();
}

async<int> leaf() {
    co_return 5;
}

async<int> branch() 
{
    int response = co_await leaf();
    co_return response;
}

int main() 
{
    auto coro = branch();
    coro.resume();
    std::cout << coro.get_result() << std::endl;
}