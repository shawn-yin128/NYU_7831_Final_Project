//
//  Models.cpp
//  7831 Final Project
//
//  Created by Xiangyang Yin on 4/30/22.
//

#include "Models.hpp"
#include <iomanip>

using namespace std;

ostream & operator<<(ostream &out, const TradeData &tradedata)
{
    out << "sDate = " << tradedata.GetDate() << endl;
    out << "dOpen = " << tradedata.GetOpen() << endl;
    out << "dHigh = " << tradedata.GetHigh() << endl;
    out << "dLow = " << tradedata.GetLow() << endl;
    out << "dClose = " << tradedata.GetClose() << endl;
    out << "dAdjClose = " << tradedata.GetAdjClose() << endl;
    return out;
}

ostream & operator<<(ostream &out, const Stock &stock)
{
    out << "Symbol: " << stock.GetSymbol() << endl;
    for (const TradeData &tradedata : stock.GetTrades()) {
        out << tradedata;
    }
    return out;
}

ostream & operator<<(std::ostream &out, const PairPrice &pp)
{
    out << "open1=" << pp.dOpen1 << ", close1=" << pp.dClose1
        << ", open2=" << pp.dOpen2 << ", close2=" << pp.dClose2
        << ", P&L=" << pp.dProfitLoss;
    return out;
}

ostream & operator<<(ostream &out, const StockPairPrices &spp)
{
    out << "(" << spp.GetStockPair().first << "," << spp.GetStockPair().second;
    out << "), vol=" << spp.GetVolatility();
    out << ", K=" << spp.GetK() << endl;
    
    for (const pair<const string, PairPrice> &dp : spp.GetDailyPrices())
    {
        out << "Date=" << dp.first << ", " << dp.second << endl;
    }
    
    return out;
}
