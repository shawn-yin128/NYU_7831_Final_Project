//
//  Models.hpp
//  7831 Final Project
//
//  Created by Xiangyang Yin on 4/30/22.
//

#ifndef Models_hpp
#define Models_hpp

#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;

class TradeData {
private:
    string sDate;
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    double dAdjClose;
    long lVolume;
    
public:
    TradeData(): sDate(""), dOpen(0), dHigh(0), dLow(0), dClose(0), dAdjClose(0), lVolume(0) {}
    TradeData(string sDate_, double dOpen_, double dHigh_, double dLow_, double dClose_, double dAdjClose_, long lVolume_): sDate(sDate_), dOpen(dOpen_), dHigh(dHigh_), dLow(dLow_), dClose(dClose_), dAdjClose(dAdjClose_), lVolume(lVolume_){}
    TradeData(const TradeData &tradeData): sDate(tradeData.sDate), dOpen(tradeData.dOpen), dHigh(tradeData.dHigh), dLow(tradeData.dLow), dClose(tradeData.dClose), dAdjClose(tradeData.dAdjClose), lVolume(tradeData.lVolume) {}
    // Overload assignment operator
    TradeData operator=(const TradeData &tradeData) {
        sDate = tradeData.sDate;
        dOpen = tradeData.dOpen;
        dHigh = tradeData.dHigh;
        dLow = tradeData.dLow;
        dClose = tradeData.dClose;
        dAdjClose = tradeData.dAdjClose;
        lVolume = tradeData.lVolume;
        return *this;
    }
    
    // Getter
    string GetDate() const { return sDate; }
    double GetOpen() const { return dOpen; }
    double GetHigh() const { return dHigh; }
    double GetLow() const { return dLow; }
    double GetClose() const { return dClose; }
    double GetAdjClose() const { return dAdjClose; }
    long GetVolume() const { return lVolume; }
    
    friend std::ostream & operator<<(std::ostream &out, const TradeData &tradedata);
};

class Stock {
private:
    string sSymbol;
    vector<TradeData> trades;
    
public:
    Stock(): sSymbol("") {}
    Stock(string sSymbol_, const vector<TradeData> trades_): sSymbol(sSymbol_), trades(trades_) {}
    Stock(const Stock &stock):sSymbol(stock.sSymbol), trades(stock.trades) {}
    
    Stock operator=(const Stock &stock) {
        sSymbol = stock.sSymbol;
        trades = stock.trades;
        return *this;
    }
    
    // Getter
    string GetSymbol() const { return sSymbol; }
    const vector<TradeData> & GetTrades() const { return trades; }
    
    void addTrade(const TradeData &tradedata) {
        trades.push_back(tradedata);
    }
    friend std::ostream & operator<<(std::ostream &out, const Stock &stock);
};

struct PairPrice {
    double dOpen1;
    double dClose1;
    double dOpen2;
    double dClose2;
    double dProfitLoss;

    PairPrice(): dOpen1(0), dClose1(0), dOpen2(0), dClose2(0), dProfitLoss(0) {}
    PairPrice(double dOpen1_, double dClose1_, double dOpen2_, double dClose2_): dOpen1(dOpen1_), dClose1(dClose1_), dOpen2(dOpen2_), dClose2(dClose2_), dProfitLoss(0) {}
    
    friend std::ostream & operator<<(std::ostream &out, const PairPrice &pp);
};

class StockPairPrices {
private:
    pair<string,string> stockPair;
    double volatility;
    double k;
    map<string,PairPrice> dailyPairPrices;
    
public:
    StockPairPrices(): volatility(0), k(0) {}
    StockPairPrices(pair<string,string> stockPair_): stockPair(stockPair_), volatility(0), k(0) {}
    
    // Getter
    const pair<string, string> & GetStockPair() const { return stockPair; }
    const map<string, PairPrice> & GetDailyPrices() const { return dailyPairPrices; }
    map<string, PairPrice> & GetDailyPrices() { return dailyPairPrices; }
    double GetVolatility() const { return volatility; }
    double GetK() const { return k; }
    
    // Setter
    void SetVolatility(double volatility_) { volatility = volatility_; }
    void SetK(double k_) { k = k_; }
    void SetDailyPairPrice(string sDate_, PairPrice pairPrice_) {
        dailyPairPrices.insert(pair<string, PairPrice>(sDate_, pairPrice_));
    }
    
    void UpdateProfitLoss(std::string sDate_, double dProfitLoss_) {
        dailyPairPrices[sDate_].dProfitLoss = dProfitLoss_;
    }
    
    friend std::ostream & operator<<(std::ostream &out, const StockPairPrices &spp);
};


#endif /* Models_hpp */
