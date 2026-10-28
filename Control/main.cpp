#include "dns_filter.h"

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    {
        Modules::Dns_Filter filter;
        std::this_thread::sleep_for(std::chrono::seconds(555));
    }

    wprintf(L"\nEnd!\n");

    return 0;
}
