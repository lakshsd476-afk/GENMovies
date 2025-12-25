#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PORT 8080
#define BUF_SIZE 8192

void send_response(int client, const char *status, const char *content_type, const char *body) {
    char header[1024];
    int len = snprintf(header, sizeof(header), "HTTP/1.1 %s\r\nContent-Type: %s; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                       status, content_type, strlen(body));
    send(client, header, len, 0);
    send(client, body, (int)strlen(body), 0);
}

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

char *strcasestr_local(const char *haystack, const char *needle);

// Movie database (title, genre, language, poster path placeholder, description, rating, trailer id, watch link)
struct Movie { const char *title; const char *genre; const char *lang; const char *poster; const char *desc; const char *rating; const char *trailer; const char *watch; };
static struct Movie movies[] = {
    {"Interstellar","Sci-Fi","Hollywood", NULL, "A team of explorers travel through a wormhole in space in an attempt to ensure humanity's survival.", "8.6/10", "zSWdZVtXT7E", "https://www.paramountplus.com/"},
    {"Parasite","Drama","Hollywood", NULL, "A poor family schemes to become employed by a wealthy family, with unexpected consequences.", "8.6/10", "SEUXfv87Wpk", "https://www.hulu.com/"},
    {"The Grand Budapest Hotel","Comedy","Hollywood", NULL, "A whimsical tale of a hotel's concierge and his trusted lobby boy during the interwar period.", "8.1/10", "1Fg5iWmQjwk", "https://www.fandango.com/"},
    {"Mad Max: Fury Road","Action","Hollywood", NULL, "In a post-apocalyptic wasteland, Max helps a rebel and a group flee from a tyrant.", "8.1/10", "hEJnMQG9ev8", "https://www.hbo.com/"},
    {"3 Idiots","Comedy","Bollywood", NULL, "Three engineering students' friendship and their struggles with India's educational system.", "8.4/10", "K0eDlFX9GMc", "https://www.netflix.com/"},
    {"Dangal","Drama","Bollywood", NULL, "A former wrestler trains his daughters to become world-class athletes.", "8.4/10", "XpXg7sZx3S0", "https://www.amazon.com/"},
    {"Baahubali: The Beginning","Action","Telugu", NULL, "A mythical tale of brotherhood, power, and betrayal in an ancient kingdom.", "8.0/10", "sOEg_YZQsTI", "https://www.primevideo.com/"},
    {"KGF: Chapter 1","Action","Kannada", NULL, "A young man's rise from poverty to a powerful gangster in the gold-mines of Kolar.", "8.2/10", "tY4k6FYa6B4", "https://www.primevideo.com/"},
    {"Jai Bhim","Drama","Tamil", NULL, "A courtroom drama focused on a woman's fight for justice for a wrongfully accused man.", "8.6/10", "tQ6mYhX8Gzg", "https://www.netflix.com/"},
    {"Kantara","Drama","Kannada", NULL, "A folk tale about a village, faith, and the balance between man and nature.", "8.2/10", "1xQ6jVQbJ2w", "https://www.netflix.com/"},
    {"Spirited Away","Animation","Hollywood", NULL, "A young girl enters a world of spirits and must find a way to save her parents.", "8.6/10", "ByXuk9QqQkk", "https://www.netflix.com/"},
    {"The Matrix","Sci-Fi","Hollywood", NULL, "A hacker discovers the reality he knows is a simulated reality called the Matrix.", "8.7/10", "vKQi3bBA1y8", "https://www.hbo.com/"},
    {"The Shawshank Redemption","Drama","Hollywood", NULL, "Two imprisoned men bond over a number of years, finding solace and eventual redemption.", "9.3/10", "6hB3S9bIaco", "https://www.netflix.com/"},
    {"Klaus","Animation","Hollywood", NULL, "A postman befriends a reclusive toymaker and together they bring joy to a frozen town.", "8.2/10", "5gV-6WlQg0I", "https://www.netflix.com/"},
    {"Drishyam","Thriller","Bollywood", NULL, "A man uses his intelligence to protect his family from a criminal investigation.", "8.2/10", "XHc1t3f9jXw", "https://www.primevideo.com/"},
    {"Baahubali 2: The Conclusion","Action","Telugu", NULL, "The epic conclusion to the Baahubali saga, revealing truth and justice.", "8.2/10", "sOEg_YZQsTI", "https://www.primevideo.com/"},
    {"Tumbbad","Horror","Bollywood", NULL, "A dark fantasy horror about greed, gods, and a cursed treasure.", "7.3/10", "m2tQ9r3nW8Y", "https://www.netflix.com/"},
    {"KGF: Chapter 2","Action","Kannada", NULL, "The continuation of the KGF saga as power struggles intensify.", "8.2/10", "xGbM3oH2f_8", "https://www.primevideo.com/"},
    {"Oh My God","Comedy","Bollywood", NULL, "A shopkeeper sues God after his shop is destroyed in an earthquake.", "8.0/10", "sC1k5d0Jz0E", "https://www.amazon.com/"},
    {"Black Panther","Action","Hollywood", NULL, "A superhero returns to his African nation to take his throne and face a challenger.", "7.3/10", "xjDjIWPwcPU", "https://www.disneyplus.com/"},
    {"Piku","Comedy","Bollywood", NULL, "A quirky story about the relationship between a daughter and her aging father.", "7.6/10", "T7tx0mpg4LM", "https://www.amazon.com/"},
    {"Bangalore Days","Drama","Malayalam", NULL, "Three cousins navigate life, love, and career in the bustling city of Bangalore.", "8.3/10", "3xTBN3A9JkY", "https://www.netflix.com/"},
    {"Kahaani","Thriller","Bollywood", NULL, "A pregnant woman searches for her missing husband in Kolkata.", "8.1/10", "0K_7h2xKktI", "https://www.primevideo.com/"},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}
};

void handle_client(int client) {
    char buf[BUF_SIZE];

    int received = recv(client, buf, sizeof(buf)-1, 0);
    if (received <= 0) return;
    buf[received] = '\0';

    // Parse GET line
    char method[16], path[1024];
    if (sscanf(buf, "%15s %1023s", method, path) != 2) return;

    if (strcmp(path, "/") == 0) {
        char *body = read_file("index.html");
        if (!body) {
            send_response(client, "404 Not Found", "text/plain", "index.html not found");
        } else {
            send_response(client, "200 OK", "text/html", body);
            free(body);
        }
    } else if (strcmp(path, "/styles.css") == 0) {
        char *body = read_file("styles.css");
        if (!body) send_response(client, "404 Not Found", "text/plain", "styles.css not found");
        else { send_response(client, "200 OK", "text/css", body); free(body); }
    } else if (strncmp(path, "/suggest", 8) == 0) {
        // parse ?genre=...
        char *q = strchr(path, '?');
        char genre[128] = "Any";
        if (q) {
            char *g = strstr(q+1, "genre=");
            if (g) {
                g += 6;
                int i = 0;
                while (*g && *g != '&' && i < (int)sizeof(genre)-1) {
                    genre[i++] = *g++;
                }
                genre[i] = '\0';
            }
        }

        // parse language param
        char lang[128] = "Any";
        char *lg = strstr(path, "lang=");
        if (lg) {
            lg += 5;
            int i = 0;
            while (*lg && *lg != '&' && i < (int)sizeof(lang)-1) lang[i++] = *lg++;
            lang[i] = '\0';
        }

        // If the request is for an individual movie, handle /movie?id=N
        if (strncmp(path, "/movie", 6) == 0) {
            char *q = strchr(path, '?');
            int id = -1;
            if (q) {
                char *p = strstr(q+1, "id=");
                if (p) id = atoi(p+3);
            }
            if (id < 0) { send_response(client, "400 Bad Request", "text/plain", "Missing id"); return; }
            if (movies[id].title == NULL) { send_response(client, "404 Not Found", "text/plain", "Movie not found"); return; }

            char body[16384];
            const char *poster_path = movies[id].poster ? movies[id].poster : "/posters/poster";
            // build poster path as /posters/posterN.svg
            char fullposter[256];
            snprintf(fullposter, sizeof(fullposter), "/posters/poster%d.svg", id);

            int n = snprintf(body, sizeof(body), "<html><head><meta charset=\\"utf-8\\"><title>%s</title><link rel=\\"stylesheet\\" href=\\"/styles.css\\"></head><body><div class=\\"container\\"><a class=\\"back\\" href=\\"/\\">← Home</a><div class=\\"movie-detail\\"><img src=\\"%s\\" alt=\\"%s poster\\"><div class=\\"detail-body\\"><h1>%s</h1><p class=\\"meta\\">%s • %s</p><p class=\\"desc\\">%s</p><p class=\\"rating\\">Rating: %s</p>",
                             fullposter, movies[id].title, movies[id].title, movies[id].title, movies[id].genre, movies[id].lang, movies[id].desc, movies[id].rating);

            if (movies[id].trailer && movies[id].trailer[0]) {
                n += snprintf(body + n, sizeof(body) - n, "<h2>Trailer</h2><div class=\\"video\\"><iframe width=\\"560\\" height=\\"315\\" src=\\"https://www.youtube.com/embed/%s\\" frameborder=\\"0\\" allow=\\"accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture\\" allowfullscreen></iframe></div>", movies[id].trailer);
            }
            if (movies[id].watch && movies[id].watch[0]) {
                n += snprintf(body + n, sizeof(body) - n, "<h2>Where to watch</h2><p><a class=\\"watch-btn\\" href=\\"%s\\" target=\\"_blank\\">Watch on platform</a></p>", movies[id].watch);
            }

            n += snprintf(body + n, sizeof(body) - n, "</div></div><p class=\\"note\\">Powered by a tiny C HTTP server — fast and simple.</p></div></body></html>");
            send_response(client, "200 OK", "text/html", body);
            return;
        }

        char body[65536];
        int n = snprintf(body, sizeof(body), "<html><head><meta charset=\"utf-8\"><title>Suggestions</title><link rel=\"stylesheet\" href=\"/styles.css\"></head><body><div class=\"container\"><a class=\"back\" href=\"/\">← Back</a><h1>Suggestions for %s - %s</h1><div class=\"cards\">", genre, lang);

        for (int i = 0; movies[i].title != NULL && n < (int)sizeof(body)-1024; ++i) {
            int match_genre = (strcmp(genre, "Any") == 0) || (strcasestr_local(movies[i].genre, genre) != NULL) || (strcasestr_local(genre, movies[i].genre) != NULL);
            int match_lang = (strcmp(lang, "Any") == 0) || (strcasestr_local(movies[i].lang, lang) != NULL) || (strcasestr_local(lang, movies[i].lang) != NULL);
            if (match_genre && match_lang) {
                char poster_local[256];
                snprintf(poster_local, sizeof(poster_local), "/posters/poster%d.svg", i);
                n += snprintf(body + n, sizeof(body) - n,
                              "<a class=\"card-link\" href=\"/movie?id=%d\"><div class=\"card\"><img src=\"%s\" alt=\"%s poster\"><div class=\"card-body\"><h3>%s</h3><p class=\"meta\">%s • %s</p></div></div></a>",
                              i, poster_local, movies[i].title, movies[i].title, movies[i].genre, movies[i].lang);
            }
        }

        n += snprintf(body + n, sizeof(body) - n, "</div><p class=\"note\">Powered by a tiny C HTTP server — fast and simple.</p></div></body></html>");
        send_response(client, "200 OK", "text/html", body);
    } else {
        send_response(client, "404 Not Found", "text/plain", "Not found");
    }
}

// case-insensitive substring search (portable)
char *strcasestr_local(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; ++haystack) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) { h++; n++; }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif
    int server_fd;
    struct sockaddr_in addr;

#ifdef _WIN32
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    printf("Server running on http://localhost:%d/\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client < 0) { perror("accept"); continue; }
        handle_client(client);
#ifdef _WIN32
        closesocket(client);
#else
        close(client);
#endif
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return 0;
}
