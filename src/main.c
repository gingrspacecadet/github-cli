#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

struct Memory {
    char *data;
    size_t size;
};

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *tmp = realloc(mem->data, mem->size + realsize + 1);
    if (!tmp) {
        fprintf(stderr, "Out of memory!");
        exit(1);
    }
    mem->data = tmp;
    memcpy(&(mem->data[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0'; /* null terminate */
    return realsize;
}

int main(void) {
    CURL *curl = NULL;
    CURLcode res;
    struct Memory resp = { .data = NULL, .size = 0 };

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init failed\n");
        return 1;
    }

    char username[64];
    printf("Enter your username:  ");
    scanf("%s", username);
    char url[128];
    sprintf(url, "https://api.github.com/users/%s/repos", username);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "github-cli");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    if (resp.data == NULL) {
        fprintf(stderr, "no response data\n");
        goto cleanup;
    }

    cJSON *root = cJSON_Parse(resp.data);
    if (!root) {
        fprintf(stderr, "Parse error: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    if (!cJSON_IsArray(root)) {
        fprintf(stderr, "Expected an array\n");
        cJSON_Delete(root);
        return 1;
    }

    /* Iterate over array elements */
    int repo_count = cJSON_GetArraySize(root);
    if (repo_count == 0) {
        printf("No repositories.\n");
        goto cleanup;
    }
    for (int i = 0; i < repo_count; i++) {
        cJSON *repo = cJSON_GetArrayItem(root, i);

        /* Access fields inside each repo object */
        cJSON *name = cJSON_GetObjectItemCaseSensitive(repo, "name");
        cJSON *url  = cJSON_GetObjectItemCaseSensitive(repo, "description");

        if (cJSON_IsString(name) && name->valuestring != NULL) {
            printf("Repo %d: %s\n", i, name->valuestring);
        }
        if (cJSON_IsString(url) && url->valuestring != NULL) {
            printf("%s\n", url->valuestring);
        }
        printf("\n");
    }

    cJSON_Delete(root);

cleanup:
    free(resp.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return (res == CURLE_OK) ? 0 : 1;
}
