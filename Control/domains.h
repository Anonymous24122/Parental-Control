#ifndef DOMAINS_H
#define DOMAINS_H

#include "headers.h"

inline
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
        {"googleapis.com", 1},  // API wewnętrzne YouTube
        {
            "instalogik.pl", 1
        }
    };

#endif // DOMAINS_H
