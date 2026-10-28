#include "dns_filter.h"

// TODO: Implement when you compile and register it as a service
// template<typename T>
// using _SVCStream = _OStream<std::ostream, T, [](T val, std::ostream& str){/*Empty herer*/}>;

// std::unordered_map<std::string, bool> domains =
//     {
//         // --- Rdzeń systemu operacyjnego i sieci (Inaczej Windows uzna, że nie ma internetu) ---
//         {"msftconnecttest.com", 1}, // Sprawdzanie stanu sieci w Windows
//         {"://gstatic.com", 1}, // Sprawdzanie stanu sieci w Chrome/Android

//         // --- YouTube i Google ---
//         {"youtube.com", 1},
//         {"google.com", 1},
//         {"googlevideo.com", 1}, // Wideo i audio
//         {"ytimg.com", 1},       // Miniatury i style CSS
//         {"ggpht.com", 1},       // Awatary i zdjęcia profilowe
//         {"gstatic.com", 1},     // Czcionki i biblioteki JS od Google
//         {"googleapis.com", 1},  // API wewnętrzne YouTube
//         {
//             "speedtest.pl", 1
//         }
// };


// auto ToWstring = [](std::string str) -> std::wstring
// {
//     int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), NULL, 0);
//     std::wstring wstr(size_needed, L' ');
//     MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
//     return wstr;
// };

// template <typename ResolveType = bool, typename CatchType = bool>
// class Promise
// {
// public:
//     using _res_type = std::function<void(ResolveType)>;
//     using _rej_type = std::function<void(CatchType)>;
// private:
//     std::condition_variable cv;
//     std::mutex cv_m;
//     std::atomic<bool> cv_a{false};
// public:
//     ResolveType res{};
//     CatchType cat{};
//     std::atomic<bool> error{false};
// private:
//     _res_type resolve = [this](ResolveType val) -> void
//     {
//         {
//             std::lock_guard<std::mutex> lk(this->cv_m);
//             this->cv_a.store(true);
//             this->res = val;
//         }
//         this->cv.notify_all();
//     };
//     _rej_type reject = [this](CatchType val) -> void
//     {
//         {
//             std::lock_guard<std::mutex> lk(this->cv_m);
//             this->cv_a.store(true);
//             this->error.store(true);
//             this->cat = val;
//         }

//         this->cv.notify_all();
//     };
// public:
//     std::thread t;
// private:
//     std::function<void(CatchType)> catch_functor{[](CatchType){}};
//     std::function<void(ResolveType)> resolve_functor{[](ResolveType){}};
//     std::function<void()> finally_functor{[](){}};

//     const std::function<void(_res_type, _rej_type)> _resolver;
// public:
//     inline Promise<ResolveType, CatchType>* p_catch(decltype(catch_functor) func)
//     {
//         catch_functor = func;
//         return this;
//     }

//     inline Promise<ResolveType, CatchType>* p_then(decltype(resolve_functor) func)
//     {
//         resolve_functor = func;
//         return this;
//     }

//     inline Promise<ResolveType, CatchType>* p_finally(decltype(finally_functor) func)
//     {
//         finally_functor = func;
//         return this;
//     }

//     Promise<ResolveType, CatchType>* StartAwait()
//     {
//         std::unique_lock<std::mutex> lck(this->cv_m);
//         this->t = std::thread([this]{this->_resolver(resolve, reject);});
//         this->cv.wait(lck, [this]{return this->cv_a.load();});
//         if (this->t.joinable())
//             this->t.join();
//         if (error.load())
//             catch_functor(this->cat);
//         else resolve_functor(this->res);
//         finally_functor();
//         return this;
//     }

//     void Start(bool join = 1)
//     {
//         std::thread th([this, join]()
//                        {
//                            std::unique_lock<std::mutex> lck(this->cv_m);
//                            this->t = std::thread([this]{this->_resolver(resolve, reject);});
//                            this->cv.wait(lck, [this]{return this->cv_a.load();});
//                            if (this->t.joinable() && join)
//                                this->t.join();
//                            if (error.load())
//                                catch_functor(this->cat);
//                            else resolve_functor(this->res);
//                            finally_functor();
//                        });
//         th.detach();
//     }

//     Promise(const std::function<void(_res_type, _rej_type)> resolver): _resolver(resolver)
//     {
//     };

//     Promise(const Promise&) = delete;
//     Promise& operator=(const Promise&) = delete;

// };

// class SERVER
// {
// private:
//     WSADATA _data;
//     SOCKET sock_ipv4{0};
//     SOCKET sock_ipv6{0};
//     SOCKET out_sock{0};
//     SOCKET out_sock_ipv6{0};
//     std::atomic<bool> STOP{0};
//     std::mutex out_sock_m{};
// public:
//     std::function<bool(int, Promise<bool, DWORD>::_rej_type&, DWORD)> error_handle = [&](int result, Promise<bool, DWORD>::_rej_type& func, DWORD ignore = WSAECONNRESET) -> bool
//     {
//         if (result == SOCKET_ERROR)
//         {
//             DWORD err = WSAGetLastError();
//             if (err != ignore && err != 0)
//             {
//                 func(err);
//                 return true;
//             }
//         }
//         return false;
//     };

//     std::function<std::string(const std::string&)> GetBaseDomain = [](const std::string& full_address) -> std::string
//     {
//         size_t last_dot = full_address.rfind('.');
//         if (last_dot == std::string::npos) return full_address;

//         size_t second_last_dot = full_address.rfind('.', last_dot - 1);

//         if (second_last_dot == std::string::npos)
//         {
//             return full_address;
//         }

//         return full_address.substr(second_last_dot + 1);
//     };

//     Promise<bool, DWORD> IpV4_Filter
//         {[this](Promise<bool, DWORD>::_res_type resolve,
//                 Promise<bool, DWORD>::_rej_type reject)
//          {
//              struct sockaddr_in addr;
//              int bytes_received = 0,
//                  WordIndex = 0;
//              std::string address = "";
//              int s = 0;
//              ZeroMemory(&addr, s = sizeof(addr));
//              char buffer[1024],
//                  out_buffer[1024];

//              while (true)
//              {
//                  if (this->STOP.load()) break;
//                  if ((bytes_received = recvfrom(this->sock_ipv4, buffer, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &s)) < 1)
//                  {if (this->error_handle(bytes_received, reject, WSAECONNRESET)) return; else continue;};

//                  for (int i = 12; i < bytes_received; ++i)
//                  {
//                      if (WordIndex == 0)
//                      {
//                          WordIndex = buffer[i];
//                          if (WordIndex == 0) break;
//                          if (i > 12) address += '.';
//                          continue;
//                      }
//                      address += buffer[i];
//                      WordIndex--;
//                  }

//                  wprintf(L"\n[DNS ipv4]: Odebrano pakiet o rozmiarze: %d%ls%s", bytes_received, L" bajtów!\n[DNS ipv4]: Adres domeny: \n", address.c_str());

//                  if (domains.find(this->GetBaseDomain(address)) == domains.end())
//                  {
//                      buffer[2] = 0x81;
//                      buffer[3] = 0x83;
//                      if (this->error_handle(sendto(this->sock_ipv4, buffer, bytes_received, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
//                  }
//                  else
//                  {
//                      if (this->error_handle(send(this->out_sock, buffer, bytes_received, 0), reject, WSAECONNRESET)) return;
//                      int out_len = recv(this->out_sock, out_buffer, sizeof(out_buffer), 0);
//                      if (this->error_handle(out_len, reject, WSAECONNRESET)) return;
//                      if (this->error_handle(sendto(this->sock_ipv4, out_buffer, out_len, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
//                  }

//                  address = "";
//                  WordIndex = 0;
//                  s = sizeof(addr);
//              };

//              resolve(0);
//          }};

//     Promise<bool, DWORD> IpV6_Filter
//         {[this](Promise<bool, DWORD>::_res_type resolve,
//                 Promise<bool, DWORD>::_rej_type reject)
//          {
//              struct sockaddr_in6 addr;
//              int bytes_received = 0,
//                  WordIndex = 0;
//              std::string address = "";
//              int s = 0;
//              ZeroMemory(&addr, s = sizeof(addr));
//              char buffer[1024],
//                  out_buffer[1024];

//              while (true)
//              {
//                  if (this->STOP.load()) break;
//                  if ((bytes_received = recvfrom(this->sock_ipv6, buffer, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &s)) < 1)
//                  {if (this->error_handle(bytes_received, reject, WSAECONNRESET)) return; else continue;};

//                  for (int i = 12; i < bytes_received; ++i)
//                  {
//                      if (WordIndex == 0)
//                      {
//                          WordIndex = buffer[i];
//                          if (WordIndex == 0) break;
//                          if (i > 12) address += '.';
//                          continue;
//                      }
//                      address += buffer[i];
//                      WordIndex--;
//                  }

//                  wprintf(L"\n[DNS ipv6]: Odebrano pakiet o rozmiarze: %d%ls%s", bytes_received, L" bajtów!\n[DNS ipv6]: Adres domeny: \n", address.c_str());

//                  if (domains.find(this->GetBaseDomain(address)) == domains.end())
//                  {
//                      buffer[2] = 0x81;
//                      buffer[3] = 0x83;
//                      if (this->error_handle(sendto(this->sock_ipv6, buffer, bytes_received, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
//                  }
//                  else
//                  {
//                      if (this->error_handle(send(this->out_sock_ipv6, buffer, bytes_received, 0), reject, WSAECONNRESET)) return;
//                      int out_len = recv(this->out_sock_ipv6, out_buffer, sizeof(out_buffer), 0);
//                      if (this->error_handle(out_len, reject, WSAECONNRESET)) return;
//                      if (this->error_handle(sendto(this->sock_ipv6, out_buffer, out_len, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
//                  }

//                  address = "";
//                  WordIndex = 0;
//                  s = sizeof(addr);
//              };

//              resolve(0);
//          }};

//     SERVER()
//     {
//         Promise<bool, DWORD> promise
//             ([this](Promise<bool, DWORD>::_res_type res,
//                     Promise<bool, DWORD>::_rej_type rej)
//              {
//                  if (error_handle(WSAStartup(MAKEWORD(2, 2), &this->_data), rej, WSAECONNRESET)) return;

//                  {
//                      if (error_handle(this->sock_ipv4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) return;

//                      struct sockaddr_in sa;
//                      ZeroMemory(&sa, sizeof(sa));
//                      sa.sin_family = AF_INET;
//                      sa.sin_port = htons(53);

//                      int result = InetPtonA(AF_INET, "127.0.0.1", &sa.sin_addr);
//                      if (result != 1) {rej(WSAGetLastError()); return;};

//                      if (error_handle(bind(this->sock_ipv4, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)), rej, WSAECONNRESET)) return;
//                  } // First (IpV4) socket

//                  {
//                      if (error_handle(this->sock_ipv6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) return;

//                      struct sockaddr_in6 sa;
//                      ZeroMemory(&sa, sizeof(sa));
//                      sa.sin6_family = AF_INET6;
//                      sa.sin6_port = htons(53);

//                      int result = InetPtonA(AF_INET6, "0000:0000:0000:0000:0000:0000:0000:0001", &sa.sin6_addr);
//                      if (result != 1) {rej(WSAGetLastError()); return;};

//                      if (error_handle(bind(this->sock_ipv6, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)), rej, WSAECONNRESET)) return;
//                  } // Second (IpV6) socket

//                  {
//                      if (error_handle(this->out_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) {return;};

//                      struct sockaddr_in server_addr;
//                      ZeroMemory(&server_addr, sizeof(server_addr));
//                      server_addr.sin_family = AF_INET;
//                      server_addr.sin_port = htons(53);

//                      int result = InetPtonA(AF_INET, "192.168.0.1", &server_addr.sin_addr);
//                      if (result != 1) {rej(WSAGetLastError()); return;};
//                      if (error_handle(connect(this->out_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)), rej, WSAECONNRESET)) return;
//                  } // Third (IpV4) out socket

//                  {
//                      if (error_handle(this->out_sock_ipv6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) {return;};

//                      struct sockaddr_in6 server_addr;
//                      ZeroMemory(&server_addr, sizeof(server_addr));
//                      server_addr.sin6_family = AF_INET6;
//                      server_addr.sin6_port = htons(53);

//                      int result = InetPtonA(AF_INET6, "fe80::541f:8de7:8b2a:1", &server_addr.sin6_addr);
//                      if (result != 1) {rej(WSAGetLastError()); return;};
//                      if (error_handle(connect(this->out_sock_ipv6, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)), rej, WSAECONNRESET)) return;
//                  } // Fourth (IpV6) out socket


//                  this->IpV4_Filter.p_catch(rej)->Start(0);
//                  this->IpV6_Filter.p_catch(rej)->Start(0);

//                  res(0);
//              });
//         promise.p_catch([](DWORD cat)
//                         {
//                             std::string str = std::error_code(cat, std::system_category()).message();

//                             std::wprintf(L"[Error]: %ls\n", ToWstring(str).c_str());
//                         })->StartAwait();
//     }

//     ~SERVER()
//     {
//         this->STOP.store(true);

//         // 1. NATYCHMIAST zamykamy sockety, żeby recvfrom w wątkach natychmiast zwróciło błąd (SOCKET_ERROR)
//         if (this->sock_ipv4 != INVALID_SOCKET) closesocket(this->sock_ipv4);
//         if (this->sock_ipv6 != INVALID_SOCKET) closesocket(this->sock_ipv6);
//         if (this->out_sock != INVALID_SOCKET) closesocket(this->out_sock);
//         if (this->out_sock_ipv6 != INVALID_SOCKET) closesocket(this->out_sock_ipv6);

//         // 2. Teraz wątki błyskawicznie wyjdą z pętli recvfrom i zakończą się bezpiecznie
//         if (this->IpV4_Filter.t.joinable()) this->IpV4_Filter.t.join();
//         if (this->IpV6_Filter.t.joinable()) this->IpV6_Filter.t.join();

//         WSACleanup();
//     }
// };

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    {
        Modules::Dns_Filter filter;
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }

    wprintf(L"\nEnd!\n");

    return 0;
}