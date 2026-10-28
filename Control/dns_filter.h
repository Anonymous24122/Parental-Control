#ifndef DNS_FILTER_H
#define DNS_FILTER_H

#include "headers.h"
#include "promise.h"
#include "domains.h"

namespace Modules
{

class Dns_Filter
{
private:
    WSADATA _data;
    SOCKET sock_ipv4{0};
    SOCKET sock_ipv6{0};
    SOCKET out_sock{0};
    SOCKET out_sock_ipv6{0};
    std::atomic<bool> STOP{0};
    std::mutex out_sock_m{};
public:
    std::function<bool(int, Promise<bool, DWORD>::_rej_type&, DWORD)> error_handle = [&](int result, Promise<bool, DWORD>::_rej_type& func, DWORD ignore = WSAECONNRESET) -> bool
    {
        if (result == SOCKET_ERROR)
        {
            DWORD err = WSAGetLastError();
            if (err != ignore && err != 0)
            {
                func(err);
                return true;
            }
        }
        return false;
    };

    std::function<std::string(const std::string&)> GetBaseDomain = [](const std::string& full_address) -> std::string
    {
        size_t last_dot = full_address.rfind('.');
        if (last_dot == std::string::npos) return full_address;

        size_t second_last_dot = full_address.rfind('.', last_dot - 1);

        if (second_last_dot == std::string::npos)
        {
            return full_address;
        }

        return full_address.substr(second_last_dot + 1);
    };

    Promise<bool, DWORD> IpV4_Filter
        {[this](Promise<bool, DWORD>::_res_type resolve,
                Promise<bool, DWORD>::_rej_type reject)
         {
             struct sockaddr_in addr;
             int bytes_received = 0,
                 WordIndex = 0;
             std::string address = "";
             int s = 0;
             ZeroMemory(&addr, s = sizeof(addr));
             char buffer[1024],
                 out_buffer[1024];

             while (true)
             {
                 if (this->STOP.load()) break;
                 if ((bytes_received = recvfrom(this->sock_ipv4, buffer, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &s)) < 1)
                 {if (this->error_handle(bytes_received, reject, WSAECONNRESET)) return; else continue;};

                 for (int i = 12; i < bytes_received; ++i)
                 {
                     if (WordIndex == 0)
                     {
                         WordIndex = buffer[i];
                         if (WordIndex == 0) break;
                         if (i > 12) address += '.';
                         continue;
                     }
                     address += buffer[i];
                     WordIndex--;
                 }

                 wprintf(L"\n[DNS ipv4]: Odebrano pakiet o rozmiarze: %d%ls%s", bytes_received, L" bajtów!\n[DNS ipv4]: Adres domeny: \n", address.c_str());

                 if (domains.find(this->GetBaseDomain(address)) == domains.end())
                 {
                     buffer[2] = 0x81;
                     buffer[3] = 0x83;
                     if (this->error_handle(sendto(this->sock_ipv4, buffer, bytes_received, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
                 }
                 else
                 {
                     if (this->error_handle(send(this->out_sock, buffer, bytes_received, 0), reject, WSAECONNRESET)) return;
                     int out_len = recv(this->out_sock, out_buffer, sizeof(out_buffer), 0);
                     if (this->error_handle(out_len, reject, WSAECONNRESET)) return;
                     if (this->error_handle(sendto(this->sock_ipv4, out_buffer, out_len, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
                 }

                 address = "";
                 WordIndex = 0;
                 s = sizeof(addr);
             };

             resolve(0);
         }};

    Promise<bool, DWORD> IpV6_Filter
        {[this](Promise<bool, DWORD>::_res_type resolve,
                Promise<bool, DWORD>::_rej_type reject)
         {
             struct sockaddr_in6 addr;
             int bytes_received = 0,
                 WordIndex = 0;
             std::string address = "";
             int s = 0;
             ZeroMemory(&addr, s = sizeof(addr));
             char buffer[1024],
                 out_buffer[1024];

             while (true)
             {
                 if (this->STOP.load()) break;
                 if ((bytes_received = recvfrom(this->sock_ipv6, buffer, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &s)) < 1)
                 {if (this->error_handle(bytes_received, reject, WSAECONNRESET)) return; else continue;};

                 for (int i = 12; i < bytes_received; ++i)
                 {
                     if (WordIndex == 0)
                     {
                         WordIndex = buffer[i];
                         if (WordIndex == 0) break;
                         if (i > 12) address += '.';
                         continue;
                     }
                     address += buffer[i];
                     WordIndex--;
                 }

                 wprintf(L"\n[DNS ipv6]: Odebrano pakiet o rozmiarze: %d%ls%s", bytes_received, L" bajtów!\n[DNS ipv6]: Adres domeny: \n", address.c_str());

                 if (domains.find(this->GetBaseDomain(address)) == domains.end())
                 {
                     buffer[2] = 0x81;
                     buffer[3] = 0x83;
                     if (this->error_handle(sendto(this->sock_ipv6, buffer, bytes_received, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
                 }
                 else
                 {
                     if (this->error_handle(send(this->out_sock_ipv6, buffer, bytes_received, 0), reject, WSAECONNRESET)) return;
                     int out_len = recv(this->out_sock_ipv6, out_buffer, sizeof(out_buffer), 0);
                     if (this->error_handle(out_len, reject, WSAECONNRESET)) return;
                     if (this->error_handle(sendto(this->sock_ipv6, out_buffer, out_len, 0, reinterpret_cast<sockaddr*>(&addr), s), reject, WSAECONNRESET)) return;
                 }

                 address = "";
                 WordIndex = 0;
                 s = sizeof(addr);
             };

             resolve(0);
         }};

    Promise<bool, DWORD> promise
        {[this](Promise<bool, DWORD>::_res_type res,
                Promise<bool, DWORD>::_rej_type rej)
         {
             if (error_handle(WSAStartup(MAKEWORD(2, 2), &this->_data), rej, WSAECONNRESET)) return;

             {
                 if (error_handle(this->sock_ipv4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) return;

                 struct sockaddr_in sa;
                 ZeroMemory(&sa, sizeof(sa));
                 sa.sin_family = AF_INET;
                 sa.sin_port = htons(53);

                 int result = InetPtonA(AF_INET, "127.0.0.1", &sa.sin_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};

                 if (error_handle(bind(this->sock_ipv4, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)), rej, WSAECONNRESET)) return;
             } // First (IpV4) socket

             {
                 if (error_handle(this->sock_ipv6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) return;

                 struct sockaddr_in6 sa;
                 ZeroMemory(&sa, sizeof(sa));
                 sa.sin6_family = AF_INET6;
                 sa.sin6_port = htons(53);

                 int result = InetPtonA(AF_INET6, "0000:0000:0000:0000:0000:0000:0000:0001", &sa.sin6_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};

                 if (error_handle(bind(this->sock_ipv6, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)), rej, WSAECONNRESET)) return;
             } // Second (IpV6) socket

             {
                 if (error_handle(this->out_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) {return;};

                 struct sockaddr_in server_addr;
                 ZeroMemory(&server_addr, sizeof(server_addr));
                 server_addr.sin_family = AF_INET;
                 server_addr.sin_port = htons(53);

                 int result = InetPtonA(AF_INET, "192.168.0.1", &server_addr.sin_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};
                 if (error_handle(connect(this->out_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)), rej, WSAECONNRESET)) return;
             } // Third (IpV4) out socket

             {
                 if (error_handle(this->out_sock_ipv6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP), rej, WSAECONNRESET)) {return;};

                 struct sockaddr_in6 server_addr;
                 ZeroMemory(&server_addr, sizeof(server_addr));
                 server_addr.sin6_family = AF_INET6;
                 server_addr.sin6_port = htons(53);

                 int result = InetPtonA(AF_INET6, "fe80::541f:8de7:8b2a:1", &server_addr.sin6_addr);
                 if (result != 1) {rej(WSAGetLastError()); return;};
                 if (error_handle(connect(this->out_sock_ipv6, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)), rej, WSAECONNRESET)) return;
             } // Fourth (IpV6) out socket


             this->IpV4_Filter.p_catch(rej)->Start(0);
             this->IpV6_Filter.p_catch(rej)->Start(0);

             res(0);
         }};

    Dns_Filter();
    ~Dns_Filter();
};

} // namespace Modules

#endif // DNS_FILTER_H
