#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

#define API_KEY "f535a8f97f56f8f2d8e3c9ca231c6c0db2d53d85dad98053b7ae7b98cab6faa3"

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t real_size = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + real_size + 1);
  if(ptr == NULL) {
    printf("Not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, real_size);
  mem->size += real_size;
  mem->memory[mem->size] = 0;

  return real_size;
}

void checkVirusTotal(const char *domain) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "https://www.virustotal.com/vtapi/v2/domain/report?apikey=%s&domain=%s", API_KEY, domain);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("VirusTotal Response: %s\n", chunk.memory);
        }

        curl_easy_cleanup(curl);
    }

    if(chunk.memory) {
        free(chunk.memory);
    }

    curl_global_cleanup();
}

void extractLinksAndTexts(const char *htmlContent, FILE *linksFile, FILE *textsFile) {
    htmlDocPtr doc = htmlReadMemory(htmlContent, strlen(htmlContent), NULL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL) {
        fprintf(stderr, "Error parsing document\n");
        return;
    }

    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (context == NULL) {
        fprintf(stderr, "Error creating XPath context\n");
        xmlFreeDoc(doc);
        return;
    }

    xmlXPathObjectPtr result = xmlXPathEvalExpression((const xmlChar *)"//a[@href]", context);
    if (result == NULL) {
        fprintf(stderr, "Error evaluating XPath expression\n");
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return;
    }

    if (result->nodesetval) {
        for (int i = 0; i < result->nodesetval->nodeNr; i++) {
            xmlNodePtr node = result->nodesetval->nodeTab[i];
            xmlChar *href = xmlGetProp(node, (const xmlChar *)"href");
            if (href) {
                fprintf(linksFile, "%s\n", href);
                xmlFree(href);
            }
        }
    }

    xmlXPathFreeObject(result);

    result = xmlXPathEvalExpression((const xmlChar *)"//body", context);
    if (result == NULL) {
        fprintf(stderr, "Error evaluating XPath expression\n");
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return;
    }

    if (result->nodesetval) {
        for (int i = 0; i < result->nodesetval->nodeNr; i++) {
            xmlNodePtr node = result->nodesetval->nodeTab[i];
            xmlChar *text = xmlNodeGetContent(node);
            if (text) {
                fprintf(textsFile, "%s\n", text);
                xmlFree(text);
            }
        }
    }

    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
}

void saveToCSV(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s for writing.\n", filename);
        return;
    }

    fprintf(file, "%s", data);
    fclose(file);
    printf("Saved %s\n", filename);
}

void crawlWeb(const char *domain) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "http://%s", domain);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Crawled data from %s\n", domain);

            FILE *linksFile = fopen("links.csv", "w");
            FILE *textsFile = fopen("texts.csv", "w");
            if (linksFile == NULL || textsFile == NULL) {
                fprintf(stderr, "Could not open file for writing.\n");
                if (linksFile) fclose(linksFile);
                if (textsFile) fclose(textsFile);
                return;
            }

            extractLinksAndTexts(chunk.memory, linksFile, textsFile);

            fclose(linksFile);
            fclose(textsFile);
        }

        curl_easy_cleanup(curl);
    }

    if(chunk.memory) {
        free(chunk.memory);
    }

    curl_global_cleanup();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Sai số lượng đối số truyền vào. Nhập lại theo cú pháp ./resolver <domain hoặc IP>\n");
        return 1;
    }

    const char *xaunhapvao = argv[1];
    struct hostent *he;
    struct in_addr addr;
    struct in_addr **addr_list;
    int i;

    if (inet_addr(xaunhapvao) != INADDR_NONE) {
        inet_aton(xaunhapvao, &addr);

        if ((he = gethostbyaddr(&addr, sizeof(addr), AF_INET)) == NULL) {
            printf("Not found information\n");
            return 1;
        }

        printf("Official name: %s\n", he->h_name);
        printf("Alias name:\n");
        addr_list = (struct in_addr **)he->h_addr_list;
        for (i = 0; addr_list[i] != NULL; i++) {
            printf("%s\n", inet_ntoa(*addr_list[i]));
        }
    } else {
        if ((he = gethostbyname(xaunhapvao)) == NULL) {
            printf("Not found information\n");
            return 1;
        }

        addr_list = (struct in_addr **)he->h_addr_list;

        printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
        printf("Alias IP:\n");
        for (i = 1; addr_list[i] != NULL; i++) {
            printf("%s\n", inet_ntoa(*addr_list[i]));
        }

        checkVirusTotal(xaunhapvao);
        crawlWeb(xaunhapvao);
    }

    return 0;
}





