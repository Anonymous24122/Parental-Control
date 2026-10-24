#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <functional>
#include <coroutine>
#include <condition_variable>
#include <mutex>
#include <system_error>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <string>
#include <unordered_map>

// TODO: Implement when you compile and register it as a service
// template<typename T>
// using _SVCStream = _OStream<std::ostream, T, [](T val, std::ostream& str){/*Empty herer*/}>;

std::unordered_map<std::string, bool> domains =
{
    // --- Rdzeń systemu operacyjnego i sieci (Inaczej Windows uzna, że nie ma internetu) ---
    {"msftconnecttest.com", 1}, // Sprawdzanie stanu sieci w Windows
    {"://gstatic.com", 1}, // Sprawdzanie stanu sieci w Chrome/Android

    // --- YouTube i Google ---
    {"youtube.com", 1},
    {"google.com", 1},
    {"googlevideo.com", 1}, // Wideo i audio
    {"ytimg.com", 1},       // Miniatury i style CSS
    {"ggpht.com", 1},       // Awatary i zdjęcia profilowe
    {"gstatic.com", 1},     // Czcionki i biblioteki JS od Google
    {"googleapis.com", 1}   // API wewnętrzne YouTube
};


auto ToWstring = [](std::string str) -> std::wstring
{
    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstr(size_needed, L' ');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
    return wstr;
};

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
    std::thread t;

    std::function<void(CatchType)> catch_functor{[](CatchType){}};
    std::function<void(ResolveType)> resolve_functor{[](ResolveType){}};
    std::function<void()> finally_functor{[](){}};

    const std::function<void(_res_type, _rej_type)> _resolver;
public:
    inline Promise<ResolveType, CatchType>* p_catch(decltype(catch_functor) func)
    {
        catch_functor = func;
        return this;
    }

    inline Promise<ResolveType, CatchType>* p_then(decltype(resolve_functor) func)
    {
        resolve_functor = func;
        return this;
    }

    inline Promise<ResolveType, CatchType>* p_finally(decltype(finally_functor) func)
    {
        finally_functor = func;
        return this;
    }

    Promise<ResolveType, CatchType>* StartAwait()
    {
        std::unique_lock<std::mutex> lck(this->cv_m);
        t = std::thread([this]{this->_resolver(resolve, reject);});
        this->cv.wait(lck, [this]{return this->cv_a.load();});
        if (t.joinable())
            t.join();
        if (error.load())
            catch_functor(this->cat);
        else resolve_functor(this->res);
        finally_functor();
        return this;
    }

    Promise<ResolveType, CatchType>* Start()
    {
        std::thread th([this]()
                      {
                            std::unique_lock<std::mutex> lck(this->cv_m);
                            this->t = std::thread([this]{this->_resolver(resolve, reject);});
                            this->cv.wait(lck, [this]{return this->cv_a.load();});
                            if (t.joinable())
                                t.join();
                            if (error.load())
                                catch_functor(this->cat);
                            else resolve_functor(this->res);
                            finally_functor();
                      });
        th.detach();
        return this;
    }

    Promise(const std::function<void(_res_type, _rej_type)> resolver): _resolver(resolver)
    {
    };

    Promise(const Promise&) = delete;
    Promise& operator=(const Promise&) = delete;

};

class DNS_SERVER
{
private:
    WSADATA _data;
    SOCKET sock{0};
    SOCKET out_sock{0};
public:
    DNS_SERVER()
    {
        Promise<bool, DWORD> promise
        ([this](Promise<bool, DWORD>::_res_type res,
            Promise<bool, DWORD>::_rej_type rej)
         {
             auto error_handle = [&](int result, DWORD ignore = WSAECONNRESET)
                {
                    if (result == SOCKET_ERROR)
                    {
                        DWORD err = WSAGetLastError();
                        if (err != ignore && err != 0)
                        {
                            rej(err);
                            return true;
                        }
                    }

                    return false;
                };

             {
                 // Initialization (1st socket)..
                 if (error_handle(WSAStartup(MAKEWORD(2, 2), &this->_data))) return;
                 if (error_handle(this->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) return;

                 struct sockaddr_in sa;
                 sa.sin_family = AF_INET;
                 sa.sin_port = htons(53);

                 int result = InetPtonA(AF_INET, "127.0.0.1", &sa.sin_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};

                 if (error_handle(bind(this->sock, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)))) return;
             }

             {
                 // Initialization (2nd socket)..
                 if (error_handle(this->out_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))) {rej(WSAGetLastError()); return;};

                 struct sockaddr_in server_addr;
                 server_addr.sin_family = AF_INET;
                 server_addr.sin_port = htons(53);

                 int result = InetPtonA(AF_INET, "192.168.0.1", &server_addr.sin_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};
                 if (error_handle(connect(this->out_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)))) return;
             }

             char buffer[1024];
             char out_buffer[1024];

             struct sockaddr_in addr;

             int s = sizeof(addr),
                 bytes_received = 0;

            auto GetBaseDomain = [](const std::string& full_address) -> std::string
            {
                size_t last_dot = full_address.rfind('.');
                if (last_dot == std::string::npos) return full_address;

                size_t second_last_dot = full_address.rfind('.', last_dot - 1);

                if (second_last_dot == std::string::npos) {
                    return full_address;
                }

                return full_address.substr(second_last_dot + 1);
            };


             while(true)
             {
                 s = sizeof(addr);
                 if ((bytes_received =
                 recvfrom(this->sock,
                          buffer,
                          1024,
                          0,
                          reinterpret_cast<sockaddr*>(&addr),
                          &s)) < 1) {if (error_handle(bytes_received)) return; else continue;};

                wprintf(L"\n[DNS]: Odebrano pakiet o rozmiarze: %d%ls", bytes_received, L" bajtów!\n[DNS]: Adres domeny: ");
                size_t WordIndex = 0;
                std::string address = "";
                for (int i = 12; i < bytes_received; ++i)
                {
                    if (WordIndex == 0)
                    {
                        WordIndex = buffer[i];
                        if (WordIndex == 0) break;
                        if (i != 12)
                            address += '.';
                        continue;
                    }
                    address += buffer[i];
                    WordIndex--;
                }

                if (domains.find(GetBaseDomain(address)) == domains.end())
                {
                    buffer[2] = 0x81;
                    buffer[3] = 0x83;
                    if (error_handle(sendto(this->sock, buffer, bytes_received, 0, reinterpret_cast<sockaddr*>(&addr), s))) return;
                }
                else
                {
                    if (error_handle(send(this->out_sock, buffer, bytes_received, 0))) return;
                    int out_len = recv(out_sock, out_buffer, sizeof(out_buffer), 0);
                    if (error_handle(out_len)) return;
                    if (error_handle(sendto(this->sock, out_buffer, out_len, 0, reinterpret_cast<sockaddr*>(&addr), s))) return;
                }

                wprintf(L"%ls\n", ToWstring(address).c_str());
             };

            res(0);
         });
        promise.p_catch([](DWORD cat)
                        {
                            std::string str = std::error_code(cat, std::system_category()).message();

                            std::wprintf(L"[Error]: %ls\n", ToWstring(str).c_str());
                        })->StartAwait();
    }

    ~DNS_SERVER()
    {
        WSACleanup();
        closesocket(this->sock);
        closesocket(this->out_sock);
    }
};

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    DNS_SERVER server;



    return 0;
}
