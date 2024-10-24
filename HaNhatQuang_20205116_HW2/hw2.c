#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>

// API
#define API_KEY "f535a8f97f56f8f2d8e3c9ca231c6c0db2d53d85dad98053b7ae7b98cab6faa3"

size_t write_callback(void *ptr, size_t size, size_t nmemb, char *data) {
    strcat(data, (char *)ptr);
    return size * nmemb;
}

// check
void check_virustotal(char *domain) {
    CURL *curl;
    CURLcode res;
    char url[256];
    char response_data[10000] = {0}; 
    sprintf(url, "https://www.virustotal.com/vtapi/v2/domain/report?apikey=%s&domain=%s", API_KEY, domain);
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("VirusTotal Response: %s\n", response_data);
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}


int main(int argc, char *argv[]){
	int i;
	struct hostent *he;
	struct in_addr **addr_list;
	struct in_addr ipv4addr;

    //check so luong dau vao
	if(argc !=2){
		printf("Sai so luong doi so truyen vao. Nhap lai theo cu phap ./resolver B\n");
	}

	else{
        //nhap data
		char xaunhapvao[100];
		strcpy(xaunhapvao,argv[1]);
        
		if(inet_addr(xaunhapvao)==-1){
			he = gethostbyname(xaunhapvao);
			if(he!=NULL){
				addr_list = (struct in_addr **)he->h_addr_list;
				printf("Official IP: %s\n",inet_ntoa(*addr_list[0]));
    			printf("Alias IP:\n");
    			for(i = 1; addr_list[i] != NULL; i++) {
        			printf("%s \n", inet_ntoa(*addr_list[i]));     //chuyen cau truc in_addr sang cau truc a.b.c.d
    			}
                check_virustotal(xaunhapvao);
			} 
			else printf("Not found information\n");


		}
		else{
			inet_pton(AF_INET, xaunhapvao, &ipv4addr);
			he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
			if(he!=NULL){
			printf("Official name: %s\n", he->h_name);
			printf("Alias name:\n");
			while(*he->h_aliases){
					printf("%s\n", *he->h_aliases);
					he->h_aliases++;
				
				}
			}
			else printf("Not found information\n");

		}


	}

	return 0;
}