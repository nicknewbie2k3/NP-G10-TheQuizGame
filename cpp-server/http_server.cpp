#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static int interrupted = 0;

// MIME types
static const char *get_mimetype(const char *file) {
    int n = strlen(file);
    
    if (n > 5 && !strcmp(&file[n-5], ".html"))
        return "text/html";
    if (n > 4 && !strcmp(&file[n-4], ".css"))
        return "text/css";
    if (n > 3 && !strcmp(&file[n-3], ".js"))
        return "application/javascript";
    if (n > 4 && !strcmp(&file[n-4], ".png"))
        return "image/png";
    if (n > 4 && !strcmp(&file[n-4], ".jpg"))
        return "image/jpeg";
    if (n > 5 && !strcmp(&file[n-5], ".jpeg"))
        return "image/jpeg";
    if (n > 4 && !strcmp(&file[n-4], ".gif"))
        return "image/gif";
    if (n > 4 && !strcmp(&file[n-4], ".svg"))
        return "image/svg+xml";
    if (n > 5 && !strcmp(&file[n-5], ".json"))
        return "application/json";
    
    return "text/plain";
}

// Signal handler
void sigint_handler(int sig) {
    interrupted = 1;
}

// HTTP callback - serve files from public/ directory
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    
    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            char *requested_uri = (char *)in;
            const char *file_path;
            char file_buffer[256];
            
            // Default to index.html
            if (strcmp(requested_uri, "/") == 0) {
                strcpy(file_buffer, "public/index.html");
            } else {
                // Serve requested file from public/
                snprintf(file_buffer, sizeof(file_buffer), "public%s", requested_uri);
            }
            
            // Get MIME type
            const char *mimetype = get_mimetype(file_buffer);
            
            // Serve the file
            if (lws_serve_http_file(wsi, file_buffer, mimetype, NULL, 0) < 0) {
                return -1; // Failed to serve file
            }
            
            break;
        }
        
        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
            // Close connection after serving file
            return -1;
            
        default:
            break;
    }
    
    return 0;
}

// Protocols for HTTP server
static struct lws_protocols protocols_http[] = {
    {
        "http",
        callback_http,
        0,
        0,
    },
    { NULL, NULL, 0, 0 }
};

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    int port = 3001;
    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
    
    signal(SIGINT, sigint_handler);
    
    lws_set_log_level(logs, NULL);
    
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols_http;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, " Failed to create HTTP server context\n");
        return 1;
    }
    
    printf(" HTTP Server started on http://localhost:%d\n", port);
    printf(" Serving files from ./public/\n");
    printf("Press Ctrl+C to stop\n");
    
    while (!interrupted) {
        lws_service(context, 50);
    }
    
    lws_context_destroy(context);
    printf("\n HTTP Server stopped\n");
    
    return 0;
}

