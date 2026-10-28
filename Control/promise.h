#ifndef PROMISE_H
#define PROMISE_H

#include "headers.h"

namespace Modules
{

template <typename ResolveType = bool, typename CatchType = bool>
class Promise
{
public:
    using _res_type = std::function<void(ResolveType)>;
    using _rej_type = std::function<void(CatchType)>;
private:
    std::condition_variable cv;
    std::mutex cv_m;
    std::atomic<bool> cv_a{false};
public:
    ResolveType res{};
    CatchType cat{};
    std::atomic<bool> error{false};
private:
    _res_type resolve = [this](ResolveType val) -> void
    {
        {
            std::lock_guard<std::mutex> lk(this->cv_m);
            this->cv_a.store(true);
            this->res = val;
        }
        this->cv.notify_all();
    };
    _rej_type reject = [this](CatchType val) -> void
    {
        {
            std::lock_guard<std::mutex> lk(this->cv_m);
            this->cv_a.store(true);
            this->error.store(true);
            this->cat = val;
        }

        this->cv.notify_all();
    };
public:
    std::thread t;
    std::thread th;
private:
    std::function<void(CatchType)> catch_functor{[](CatchType){}};
    std::function<void(ResolveType)> resolve_functor{[](ResolveType){}};
    std::function<void()> finally_functor{[](){}};

    const std::function<void(_res_type, _rej_type)> _resolver;
public:
    inline Promise<ResolveType, CatchType>* p_catch(std::function<void(CatchType)> func);
    inline Promise<ResolveType, CatchType>* p_then(std::function<void(ResolveType)> func);
    inline Promise<ResolveType, CatchType>* p_finally(std::function<void()> func);

    Promise<ResolveType, CatchType>* StartAwait();

    void Start(bool join = 1);
public:
    Promise(const std::function<void(_res_type, _rej_type)> resolver): _resolver(resolver)
    {
    };

    Promise(const Promise&) = delete;
    Promise& operator=(const Promise&) = delete;

    ~Promise()
    {
        if (this->th.joinable()) this->th.join();
    };
};

template<typename ResolveType, typename CatchType>
Promise<ResolveType, CatchType>* Promise<ResolveType, CatchType>::p_catch(std::function<void(CatchType)> func)
{
    catch_functor = func;
    return this;
}

template<typename ResolveType, typename CatchType>
Promise<ResolveType, CatchType>* Promise<ResolveType, CatchType>::p_then(std::function<void(ResolveType)> func)
{
    resolve_functor = func;
    return this;
}

template<typename ResolveType, typename CatchType>
Promise<ResolveType, CatchType>* Promise<ResolveType, CatchType>::p_finally(std::function<void()> func)
{
    finally_functor = func;
    return this;
}

template<typename ResolveType, typename CatchType>
Promise<ResolveType, CatchType>* Promise<ResolveType, CatchType>::StartAwait()
{
    std::unique_lock<std::mutex> lck(this->cv_m);
    this->t = std::thread([this]{this->_resolver(resolve, reject);});
    this->cv.wait(lck, [this]{return this->cv_a.load();});
    if (this->t.joinable())
        this->t.join();
    if (error.load())
        catch_functor(this->cat);
    else resolve_functor(this->res);
    finally_functor();
    return this;
}

template<typename ResolveType, typename CatchType>
void Promise<ResolveType, CatchType>::Start(bool join)
{
    this->th = std::thread([this, join]()
                           {
                               std::unique_lock<std::mutex> lck(this->cv_m);
                               this->t = std::thread([this]{this->_resolver(resolve, reject);});
                               this->cv.wait(lck, [this]{return this->cv_a.load();});
                               if (this->t.joinable() && join)
                                   this->t.join();
                               if (error.load())
                                   catch_functor(this->cat);
                               else resolve_functor(this->res);
                               finally_functor();
                           });
}

} // namespace Modules

#endif // PROMISE_H
