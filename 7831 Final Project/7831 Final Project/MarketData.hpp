#pragma once
#include <string>
class Stock;    // forward declaration

// download data
int PullMarketData(const std::string& url_request, std::string& read_buffer);

// put json into objects
int PopulateTrades(const std::string& read_buffer, Stock& stock);
