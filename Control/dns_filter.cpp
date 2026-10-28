#include "dns_filter.h"
#include "utils.h"

namespace Modules
{

Dns_Filter::Dns_Filter()
{
    this->
    promise.p_catch([](DWORD cat)
                    {
                        std::string str = std::error_code(cat, std::system_category()).message();

                        std::wprintf(L"[Error]: %ls\n", ToWstring(str).c_str());
                    })->StartAwait();
}

Dns_Filter::~Dns_Filter()
{
    this->STOP.store(true);

    // 1. NATYCHMIAST zamykamy sockety, żeby recvfrom w wątkach natychmiast zwróciło błąd (SOCKET_ERROR)
    if (this->sock_ipv4 != INVALID_SOCKET) closesocket(this->sock_ipv4);
    if (this->sock_ipv6 != INVALID_SOCKET) closesocket(this->sock_ipv6);
    if (this->out_sock != INVALID_SOCKET) closesocket(this->out_sock);
    if (this->out_sock_ipv6 != INVALID_SOCKET) closesocket(this->out_sock_ipv6);

    // 2. Teraz wątki błyskawicznie wyjdą z pętli recvfrom i zakończą się bezpiecznie
    if (this->IpV4_Filter.t.joinable()) this->IpV4_Filter.t.join();
    if (this->IpV6_Filter.t.joinable()) this->IpV6_Filter.t.join();

    WSACleanup();
}

} // namespace Modules
