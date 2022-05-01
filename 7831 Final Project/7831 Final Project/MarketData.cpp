#include <iostream>
#include <fstream>
#include "json/json.h"
#include "curl/curl.h"
#include <sqlite3.h>

#include "Models.hpp"
#include "Util.h"

// download data
int PullMarketData(const string& url_request, string& read_buffer) {
    //global initiliation of curl before calling a function
    curl_global_init(CURL_GLOBAL_ALL);

    //creating session handle
    CURL * handle;

    // Store the result of CURLís webpage retrieval, for simple error checking.
    CURLcode result;

    // notice the lack of major error-checking, for brevity
    handle = curl_easy_init();

    if (!handle) {
        cout << "curl_easy_init failed" << endl;
        return -1;
    }

    // curl the url
    curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());

    //adding a user agent
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);

    // send all data to this function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);

    // we pass our 'chunk' struct to the callback function
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &read_buffer);

    //perform a blocking file transfer
    result = curl_easy_perform(handle);

    // check errors
    if (result != CURLE_OK) {
        return -1;
    }

    // close
    curl_easy_cleanup(handle);
    return 0;
}

// put daily json into objects
int PopulateTrades(const string& read_buffer, Stock& stock) {
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    Json::Value root;
    string errors;
    
    bool parsingSuccessful = reader->parse(read_buffer.c_str(), read_buffer.c_str() + read_buffer.size(), &root, &errors);
    if (!parsingSuccessful)
    {
        std::cout << "Failed to parse JSON " << std::endl << read_buffer << ": " << errors << std::endl;
        return -1;
    }
    
    string date;
    double open;
    double high;
    double low;
    double close;
    double adjclose;
    long volume;
    
    for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
    {
        date = (*itr)["date"].asString();
        open = (*itr)["open"].asFloat();
        high = (*itr)["high"].asFloat();
        low = (*itr)["low"].asFloat();
        close = (*itr)["close"].asFloat();
        adjclose = (*itr)["adjusted_close"].asFloat();
        volume = (*itr)["volume"].asInt64();
        
        TradeData td(date, open, high, low, close, adjclose, volume);
        stock.addTrade(td);
    }
    
    
    return 0;
}

