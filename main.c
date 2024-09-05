#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include "cjson/cJSON.h"

CURLcode fetchData(char **buffer);
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
struct Node {
    int id;
    double price;
    char *time;
    double movingAvg;
    struct Node *next;
};

void printList(struct Node **head);

int main(void)
{
    struct Node *head = malloc(sizeof(struct Node));
    while(1) {
        char *readBuffer = NULL;
        CURLcode res = fetchData(&readBuffer);

        if (res != CURLE_OK) {
            printf("Error!\n");
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse the JSON response
            cJSON *json = cJSON_Parse(readBuffer);
            printf("JSON parsed.\n");
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    fprintf(stderr, "Error before: %s\n", error_ptr);
                }
            } else {
                // Process JSON data here
                // For example, print the JSON
                // char *json_string = cJSON_Print(json);
                printf("Fetching bitcoin array item...\n");
                cJSON *bitcoin = cJSON_GetArrayItem(json, 0);
                printf("Bitcoin data fetched.\n");
                if (bitcoin != NULL) {
                    printf("Fetching price from Bitcoin data...\n");
                    cJSON *price = cJSON_GetObjectItem(bitcoin, "current_price");
                    cJSON *time = cJSON_GetObjectItem(bitcoin, "last_updated");
                    if (cJSON_IsNumber(price)) {
                        printf("Current Price: %f\n", price->valuedouble);
                        struct Node *lastNode = head;
                        while (lastNode->next != NULL) {
                            lastNode = lastNode->next;
                        }

                        if (head->price == 0 && head->next == NULL) {
                            // This is the first node
                            head->id = 0;
                            head->price = price->valuedouble;
                            head->time = strdup(time->valuestring);
                            head->next = NULL;
                        } else {
                            // Add a new node
                            struct Node *newNode = malloc(sizeof(struct Node));
                            newNode->id = lastNode->id + 1;
                            newNode->price = price->valuedouble;
                            newNode->time = strdup(time->valuestring);
                            newNode->next = NULL;
                            lastNode->next = newNode;

                            // Alert user to significant price change
                            double priceDiff = newNode->price - lastNode->price;
                            double percentChange = (priceDiff / lastNode->price) * 100.00;
                            if (percentChange > 1 || percentChange < -1) {
                                printf("ALERT: Significant price movement: %f%% change\n", percentChange);
                            }
                        }

                        int numNodes = 0;
                        double cumPrice = 0;
                        struct Node *currentNode = head;
                        while (currentNode != NULL) {
                            numNodes++;
                            cumPrice += currentNode->price;
                            lastNode = currentNode;
                            currentNode = currentNode->next;
                        }

                        double movingAvg = cumPrice / numNodes;

                        lastNode->movingAvg = movingAvg;

                        printList(&head);
                        printf("Num nodes: %d\n", numNodes);
                    } else {
                        printf("Error retrieving 'current_price' key\n");
                    }
                } else {
                    printf("Error accessing first element in array");
                }

                // Clean up
                cJSON_Delete(json);
            }
        }

        free(readBuffer);
        sleep(30);
    }
    return 0;
}

CURLcode fetchData(char **buffer) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    if(curl) {
        printf("CURL Initialized. Fetching data...\n");
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.coingecko.com/api/v3/coins/markets?vs_currency=usd");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) buffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    } else {
        printf("Problem occurred with CURL init.");
    }
    return res;
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **buffer = (char **)userp;

    size_t current_len = *buffer ? strlen(*buffer) : 0;
    *buffer = realloc(*buffer, current_len + realsize + 1);
    if (*buffer == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    printf("Callback appending data...\n");
    // Use memcpy to append the data
    memcpy(*buffer + current_len, contents, realsize);
    (*buffer)[current_len + realsize] = '\0'; // Ensure null termination
    printf("Data appended.\n");
    return realsize;
}

void printList(struct Node **head) {
    struct Node *currentNode = *head;
    printf("Printing list...\n");
    while (currentNode != NULL) {
        printf("ID: %d\n", currentNode->id);
        printf("Price: %f\n", currentNode->price);
        printf("movingAvg: %f\n", currentNode->movingAvg);
        printf("Next: %p\n\n", currentNode->next);
        currentNode = currentNode->next;
    }
}
