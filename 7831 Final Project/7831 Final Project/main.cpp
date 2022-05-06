#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm> // add if runs in Visual Studio
#include "Database.h"
#include "MarketData.hpp"
#include "Models.hpp"
#include "Util.h"

using namespace std;

// set EOD parameters
const string START_DATE = "2012-01-01";
const string END_DATE = "2021-12-31";
const string BT_START_DATE = "2022-01-01";
const string API_TOKEN = "623f3c21d0bd93.06437138";
const string URL_COMMON = "https://eodhistoricaldata.com/api/eod/";

int main(void) {
    // menu
    char selection;
    string menu = string("Menu \n")
        + "============================================================ \n"
        + "A: Create and Populate Pair Table \n"
        + "B: Retrieve and Populate Historical Data for Each Stock \n"
        + "C: Create PairPrices Table \n"
        + "D: Calculate Volatility \n"
        + "E: Back Testing \n"
        + "F: Calculate Profit and Loss for Each Pair \n"
        + "G: Manual Testing \n"
        + "H: Drop All tables \n"
        + "X: Exit \n"
        + "============================================================ \n"
        + "Enter Selection: ";

    sqlite3* db = NULL;
    string dbName = "PairTrading.db";
    string pair_file = "PairTrading.txt";

    int flag = 1;

    vector<pair<string, string>> PairVector;
    unordered_set<string> PairOneStocks;
    unordered_set<string> PairTwoStocks;
    // Models.hpp
    vector<StockPairPrices> BTStockPairPricesVector;
    vector<StockPairPrices> PTStockPairPricesVector;

    // get current time
    time_t t = time(0);
    tm* now = localtime(&t);
    const string BT_END_DATE = to_string(now->tm_year + 1900) + "-" + to_string(now->tm_mon + 1) + "-" + to_string(now->tm_mday);

    PairVector.clear();
    PairOneStocks.clear();
    PairTwoStocks.clear();
    BTStockPairPricesVector.clear();
    PTStockPairPricesVector.clear();
    
    while (flag) {
        // clear all containers
        cout << menu;
        cin >> selection;
        switch (selection) {
            // A: Create and Populate Pair Table
            case 'A':
            case 'a': {
                string StockPairs_sql_create_table = string("CREATE TABLE IF NOT EXISTS StockPairs ")
                    + "(id INT NOT NULL,"
                    + "symbol1 CHAR(20) NOT NULL,"
                    + "symbol2 CHAR(20) NOT NULL,"
                    + "volatility FLOAT NOT NULL,"
                    + "profit_loss FLOAT NOT NULL,"
                    + "PRIMARY KEY(symbol1, symbol2));";

                string PairOnePrices_sql_create_table = string("CREATE TABLE IF NOT EXISTS PairOnePrices ")
                    + "(symbol CHAR(20) NOT NULL,"
                    + "date CHAR(20) NOT NULL,"
                    + "open REAL NOT NULL,"
                    + "high REAL NOT NULL,"
                    + "low REAL NOT NULL,"
                    + "close REAL NOT NULL,"
                    + "adjusted_close REAL NOT NULL,"
                    + "volume INT NOT NULL,"
                    + "PRIMARY KEY(symbol, date));";

                string PairTwoPrices_sql_create_table = string("CREATE TABLE IF NOT EXISTS PairTwoPrices ")
                    + "(symbol CHAR(20) NOT NULL,"
                    + "date CHAR(20) NOT NULL,"
                    + "open REAL NOT NULL,"
                    + "high REAL NOT NULL,"
                    + "low REAL NOT NULL,"
                    + "close REAL NOT NULL,"
                    + "adjusted_close REAL NOT NULL,"
                    + "volume INT NOT NULL,"
                    + "PRIMARY KEY(symbol, date));";

                string PairPrices_sql_create_table = string("CREATE TABLE IF NOT EXISTS PairPrices ")
                    + "(symbol1 CHAR(20) NOT NULL,"
                    + "symbol2 CHAR(20) NOT NULL,"
                    + "date CHAR(20) NOT NULL,"
                    + "open1 REAL NOT NULL,"
                    + "close1 REAL NOT NULL,"
                    + "adjusted_close1 REAL NOT NULL,"
                    + "open2 REAL NOT NULL,"
                    + "close2 REAL NOT NULL,"
                    + "adjusted_close2 REAL NOT NULL,"
                    + "profit_loss REAL NOT NULL,"
                    + "PRIMARY KEY(symbol1, symbol2, date),"
                    + "FOREIGN KEY(symbol1, date) REFERENCES PairOnePrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
                    + "FOREIGN KEY(symbol2, date) REFERENCES PairTwoPrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
                    + "FOREIGN KEY(symbol1, symbol2) REFERENCES StockPairs(symbol1, symbol2) ON DELETE CASCADE ON UPDATE CASCADE);";

                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }

                if (ExecuteSQL(db, StockPairs_sql_create_table.c_str()) != 0) {
                    return -1;
                }
                if (ExecuteSQL(db, PairOnePrices_sql_create_table.c_str()) != 0) {
                    return -1;
                }
                if (ExecuteSQL(db, PairTwoPrices_sql_create_table.c_str()) != 0) {
                    return -1;
                }
                if (ExecuteSQL(db, PairPrices_sql_create_table.c_str()) != 0) {
                    return -1;
                }

                CloseDatabase(db);

                cout << "Tables Created." << endl << endl;
                break;
            }

                    // B: Retrieve and Populate Historical Data for Each Stock
            case 'B':
            case 'b': {
                // read pairs
                ifstream fin(pair_file.c_str());

                if (!fin.is_open()) {
                    cerr << "ERROR: Unable to open " << pair_file << std::endl;
                    return -1;
                }

                string line;
                while (getline(fin, line)) {
                    istringstream iss(line);
                    string symbol1;
                    string symbol2;
                    getline(iss, symbol1, ',');
                    getline(iss, symbol2, ',');

                    if (!symbol2.empty() && symbol2[symbol2.length() - 1] == '\r') {
                        symbol2.erase(symbol2.size() - 1);
                    }

                    PairVector.push_back(pair<string, string> { symbol1, symbol2 });
                }

                int id = 0;

                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }

                for (const pair<string, string>& p : PairVector) {
                    const string& symbol1 = p.first;
                    const string& symbol2 = p.second;

                    id++;

                    string sql = string("INSERT INTO StockPairs(id, symbol1, symbol2, volatility, profit_loss) VALUES(")
                        + to_string(id) + ",'"
                        + symbol1 + "','"
                        + symbol2 + "',0.0,0.0);";

                    if (ExecuteSQL(db, sql.c_str()) != 0) {
                        return -1;
                    }
                    else {
                        cout << "Inserted (" << symbol1 << "," << symbol2 << ") into table StockPairs." << endl;
                    }
                }

                for (const pair<string, string>& p : PairVector) {
                    string symbol1 = p.first;
                    string symbol2 = p.second;

                    // pull symbol1
                    if (PairOneStocks.find(symbol1) == PairOneStocks.end()) { // make sure no second pull
                        string url_request = URL_COMMON + symbol1 + ".US?from="
                            + START_DATE + "&to=" + BT_END_DATE + "&api_token=" + API_TOKEN + "&period=d&fmt=json";

                        string read_buffer;
                        vector<TradeData> trades;
                        Stock stock(symbol1, trades);

                        if (PullMarketData(url_request, read_buffer) != 0) {
                            cerr << "ERROR: Failed to pull market data: " << symbol1 << endl;
                            return -1;
                        }

                        if (PopulateTrades(read_buffer, stock) != 0) {
                            cerr << "ERROR: ParseJson() failed for " << symbol1 << std::endl;
                            return -1;
                        }

                        // insert
                        for (const TradeData& td : stock.GetTrades()) {
                            string date = td.GetDate();
                            double open = td.GetOpen();
                            double high = td.GetHigh();
                            double low = td.GetLow();
                            double close = td.GetClose();
                            double adjclose = td.GetAdjClose();
                            double volume = td.GetVolume();

                            string sql = string("INSERT INTO PairOnePrices (symbol, date, open, high, low, close, adjusted_close, volume) VALUES('")
                                + symbol1.c_str() + "','" + date.c_str() + "'," + to_string(open) + "," + to_string(high) + ","
                                + to_string(low) + "," + to_string(close) + "," + to_string(adjclose) + "," + to_string(volume) + ");";

                            if (ExecuteSQL(db, sql.c_str()) != 0) {
                                return -1;
                            }
                        }

                        cout << "Insert " << symbol1 << " into PairOnePrices." << endl;
                        // Add the stock into the set
                        PairOneStocks.insert(symbol1);
                    }

                    // pull symbol2
                    if (PairTwoStocks.find(symbol2) == PairTwoStocks.end())
                    {
                        string url_request = URL_COMMON + symbol2 + ".US?from="
                            + START_DATE + "&to=" + BT_END_DATE + "&api_token=" + API_TOKEN + "&period=d&fmt=json";

                        string read_buffer;
                        vector<TradeData> trades;
                        Stock stock(symbol2, trades);

                        if (PullMarketData(url_request, read_buffer) != 0) {
                            cerr << "ERROR: Failed to pull market data: " << symbol2 << endl;
                            return -1;
                        }

                        if (PopulateTrades(read_buffer, stock) != 0) {
                            cerr << "ERROR: ParseJson() failed for " << symbol2 << std::endl;
                            return -1;
                        }

                        // insert
                        for (const TradeData& td : stock.GetTrades()) {
                            string date = td.GetDate();
                            double open = td.GetOpen();
                            double high = td.GetHigh();
                            double low = td.GetLow();
                            double close = td.GetClose();
                            double adjclose = td.GetAdjClose();
                            double volume = td.GetVolume();

                            string sql = string("INSERT INTO PairTwoPrices (symbol, date, open, high, low, close, adjusted_close, volume) VALUES('")
                                + symbol2.c_str() + "','" + date.c_str() + "'," + to_string(open) + "," + to_string(high) + ","
                                + to_string(low) + "," + to_string(close) + "," + to_string(adjclose) + "," + to_string(volume) + ");";

                            if (ExecuteSQL(db, sql.c_str()) != 0) {
                                return -1;
                            }
                        }

                        cout << "Insert " << symbol2 << " into PairTwoPrices." << endl;
                        // Add the stock into the set
                        PairTwoStocks.insert(symbol2);
                    }
                }

                CloseDatabase(db);

                cout << endl << "Stock Pair Inserted." << endl << endl;
                break;
            }

                    // C: Create PairPrices Table
            case 'C':
            case 'c': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }
                
                string sql_count = string("SELECT COUNT(*) FROM PairPrices;");
                char** result;
                int row;
                int col;

                if (sqlite3_get_table(db, sql_count.c_str(), &result, &row, &col, NULL) != 0) {
                    return -1;
                }
                
                if (stoi(result[1]) != 0) {
                    cout << "Table already created, please enter another selection." << endl << endl;
                    break;
                }

                // insert into pairprice table
                string sql = "INSERT INTO PairPrices "
                    "SELECT StockPairs.symbol1 AS symbol1, StockPairs.symbol2 AS symbol2, PairOnePrices.date AS date, PairOnePrices.open AS open1, PairOnePrices.close AS close1, PairOnePrices.adjusted_close AS adjusted_close1, PairTwoPrices.open AS open2, PairTwoPrices.close AS close2, PairTwoPrices.adjusted_close AS adjusted_close2, 0 AS profit_loss "
                    "FROM StockPairs, PairOnePrices, PairTwoPrices "
                    "WHERE (((StockPairs.symbol1 = PairOnePrices.symbol) "
                    "AND (StockPairs.symbol2 = PairTwoPrices.symbol)) "
                    "AND (PairOnePrices.date = PairTwoPrices.date)) "
                    "ORDER BY symbol1, symbol2;";

                if (ExecuteSQL(db, sql.c_str()) != 0) {
                    return -1;
                }

                CloseDatabase(db);

                cout << "Data Inserted." << endl << endl;
                break;

            }

                    // D: Calculate Volatility
            case 'D':
            case 'd': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }

                string sql = string("UPDATE StockPairs SET volatility = ")
                    + "(SELECT(AVG((adjusted_close1/adjusted_close2)*(adjusted_close1/adjusted_close2))-AVG(adjusted_close1/adjusted_close2)*AVG(adjusted_close1/adjusted_close2)) AS variance "
                    + "FROM PairPrices "
                    + "WHERE StockPairs.symbol1 = PairPrices.symbol1 AND StockPairs.symbol2 = PairPrices.symbol2 AND PairPrices.date <= \'"
                    + END_DATE + "\');";

                if (ExecuteSQL(db, sql.c_str()) != 0) {
                    return -1;
                }

                CloseDatabase(db);
            
                cout << "Volatility Calculated." << endl << endl;
                break;
            }

                    // E: Back Testing
            case 'E':
            case 'e': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }
                
                double k;
                cout << "Please enter k: ";
                cin >> k;
                
                if (PairVector.size() == 0) {
                    string sql = string("SELECT ")
                        + "symbol1, symbol2 "
                        + "FROM StockPairs;";
                    
                    char** result;
                    int row;
                    int col;

                    if (sqlite3_get_table(db, sql.c_str(), &result, &row, &col, NULL) != 0) {
                        return -1;
                    }
                    
                    for (int i = 0; i < row; i++) {
                        int base = (i + 1) * col;
                        string symbol1 = string(result[base + 0]);
                        string symbol2 = string(result[base + 1]);
                        
                        PairVector.push_back(pair<string, string>{symbol1, symbol2});
                    }
                }
                
                for (const pair<string, string>& p : PairVector) {
                    StockPairPrices spp(p);

                    pair<string, string> pair = spp.GetStockPair();
                    string symbol1 = pair.first;
                    string symbol2 = pair.second;

                    // put pair data into spp
                    string sql = string("SELECT ")
                        + "symbol1, symbol2, date, open1, close1, open2, close2 "
                        + "FROM PairPrices "
                        + "WHERE date >= '" + BT_START_DATE + "' "
                        + "AND date <= '" + BT_END_DATE + "' "
                        + "AND symbol1 = '" + symbol1 + "' "
                        + "AND symbol2 = '" + symbol2 + "' "
                        + "ORDER BY symbol1, symbol2, date;";

                    char** result;
                    int row;
                    int col;

                    if (sqlite3_get_table(db, sql.c_str(), &result, &row, &col, NULL) != 0) {
                        return -1;
                    }

                    for (int i = 0; i < row; i++) {
                        int base = (i + 1) * col;
                        string symbol1 = string(result[base + 0]);
                        string symbol2 = string(result[base + 1]);
                        string date = string(result[base + 2]);
                        double open1 = stod(string(result[base + 3]));
                        double close1 = stod(string(result[base + 4]));
                        double open2 = stod(string(result[base + 5]));
                        double close2 = stod(string(result[base + 6]));

                        PairPrice pp(open1, close1, open2, close2);
                        spp.SetDailyPairPrice(date, pp);
                    }

                    // get volatility and put into spp
                    double volatility;
                    sql = string("SELECT volatility FROM StockPairs WHERE symbol1 = '" + symbol1 + "' AND symbol2 = '" + symbol2 + "';");
                    
                    if (sqlite3_get_table(db, sql.c_str(), &result, &row, &col, NULL) != 0) {
                        return -1;
                    }

                    volatility = stod(string(result[1]));

                    spp.SetVolatility(volatility);
                    spp.SetK(k);

                    BTStockPairPricesVector.push_back(spp);
                }

                // back test
                double close1d1 = 0.0;
                double close2d1 = 0.0;
                double open1d2 = 0.0;
                double open2d2 = 0.0;
                double close1d2 = 0.0;
                double close2d2 = 0.0;

                string start_date;

                for (StockPairPrices& spp : BTStockPairPricesVector) {
                    map<string, PairPrice>& dailyPairPrices = spp.GetDailyPrices();
                    vector<string> dates;
                    for (const pair<string, PairPrice>& dp : dailyPairPrices) {
                        dates.push_back(dp.first);
                    }
                    sort(dates.begin(), dates.end());

                    for (vector<string>::iterator itr = dates.begin(); itr != dates.end(); itr++) {
                        PairPrice& pp = dailyPairPrices[*itr];
                        if (itr == dates.begin()) {
                            start_date = *itr;
                            close1d1 = pp.dClose1;
                            close2d1 = pp.dClose2;
                            continue;
                        }
                        open1d2 = pp.dOpen1;
                        open2d2 = pp.dOpen2;
                        close1d2 = pp.dClose1;
                        close2d2 = pp.dClose2;

                        int N1, N2;
                        if (abs(close1d1 / close2d1 - open1d2 / open2d2) > spp.GetVolatility() * spp.GetK()) {
                            // short
                            N1 = -10000;
                            N2 = (int)(-N1 * open1d2 / open2d2);
                        }
                        else {
                            // long
                            N1 = 10000;
                            N2 = (int)(-N1 * open1d2 / open2d2);
                        }

                        double PL = N1 * (close1d2 - open1d2) + N2 * (close2d2 - open2d2);

                        spp.UpdateProfitLoss(*itr, PL);

                        // move to next day
                        close1d1 = close1d2;
                        close2d1 = close2d2;
                    }
                }

                CloseDatabase(db);

                cout << "Back Testing Performed." << endl << endl;
                break;
            }

                    // F: Calculate Profit and Loss for Each Pair
            case 'F':
            case 'f': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }

                // all pl are stored in BT vector
                for (StockPairPrices& spp : BTStockPairPricesVector) {
                    // update PairPrices table first
                    const map<string, PairPrice>& dailyPairPrices = spp.GetDailyPrices();
                    string symbol1 = spp.GetStockPair().first;
                    string symbol2 = spp.GetStockPair().second;

                    int count = 0;
                    int sum = 0;

                    for (const pair<string, PairPrice>& dpp : dailyPairPrices) {
                        string date = dpp.first;
                        double PL = dpp.second.dProfitLoss;

                        string sql = string("UPDATE PairPrices SET profit_loss = ")
                            + to_string(PL)
                            + " WHERE PairPrices.symbol1 = '" + symbol1
                            + "' AND PairPrices.symbol2 = '" + symbol2
                            + "' AND PairPRices.date = '" + date + "';";

                        if (ExecuteSQL(db, sql.c_str()) != 0) {
                            return -1;
                        }

                        count++;
                        sum += PL;
                    }

                    // update stock's pl in general
                    string sql = string("UPDATE StockPairs SET profit_loss = ")
                        + to_string(sum)
                        + " WHERE StockPairs.symbol1 = '" + symbol1
                        + "' AND StockPairs.symbol2 = '" + symbol2 + "';";

                    if (ExecuteSQL(db, sql.c_str()) != 0) {
                        return -1;
                    }
                }

                CloseDatabase(db);

                cout << "P&L Updated." << endl << endl;
                break;
            }

                    // G: Manual Testing
            case 'G':
            case 'g': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }
                
                // print out the list
                cout << "Stock pair list:" << endl;
                cout << "==========" << endl;
                if (PairVector.size() == 0) {
                    string sql = string("SELECT ")
                        + "symbol1, symbol2 "
                        + "FROM StockPairs;";
                    
                    char** result;
                    int row;
                    int col;

                    if (sqlite3_get_table(db, sql.c_str(), &result, &row, &col, NULL) != 0) {
                        return -1;
                    }
                    
                    for (int i = 0; i < row; i++) {
                        int base = (i + 1) * col;
                        string symbol1 = string(result[base + 0]);
                        string symbol2 = string(result[base + 1]);
                        
                        PairVector.push_back(pair<string, string>{symbol1, symbol2});
                    }
                }
                for (pair<string, string>& stock : PairVector) {
                    cout << stock.first << "  " << stock.second << endl;
                }
                cout << "==========" << endl;

                string symbol1, symbol2;
                cout << "Please enter the stock pair for testing" << endl;
                cout << "Symbol1: ";
                cin >> symbol1;
                cout << "Symbol2: ";
                cin >> symbol2;
                cout << endl;
                
                pair<string, string> stockPair = {symbol1, symbol2};
                
                while (find(PairVector.begin(), PairVector.end(), stockPair) == PairVector.end()) {
                    cout << "Invalid pair, please re-enter:" << endl;
                    cout << "Symbol1: ";
                    cin >> symbol1;
                    cout << "Symbol2: ";
                    cin >> symbol2;
                    cout << endl;
                }
                
                //Fetch the volatility of that pair from Stockpairprices object
                double volatility = 0.0;
                
                // check if calculated volatility
                string sql_sum = string("SELECT SUM(volatility) FROM StockPairs;");
                char** result;
                int row;
                int col;

                if (sqlite3_get_table(db, sql_sum.c_str(), &result, &row, &col, NULL) != 0) {
                    return -1;
                }
                
                if (stoi(result[1]) == 0) {
                    string sql = string("UPDATE StockPairs SET volatility = ")
                        + "(SELECT(AVG((adjusted_close1/adjusted_close2)*(adjusted_close1/adjusted_close2))-AVG(adjusted_close1/adjusted_close2)*AVG(adjusted_close1/adjusted_close2)) AS variance "
                        + "FROM PairPrices "
                        + "WHERE StockPairs.symbol1 = PairPrices.symbol1 AND StockPairs.symbol2 = PairPrices.symbol2 AND PairPrices.date <= \'"
                        + END_DATE + "\');";

                    if (ExecuteSQL(db, sql.c_str()) != 0) {
                        return -1;
                    }
                }
                
                string sql = string("SELECT ")
                    + "volatility "
                    + "FROM StockPairs "
                    + "WHERE symbol1 = '" + symbol1 + "' AND "
                    + "symbol2 = '" + symbol2 + "';";
                
                char** resultV;
                int rowV;
                int colV;

                if (sqlite3_get_table(db, sql.c_str(), &resultV, &rowV, &colV, NULL) != 0) {
                    return -1;
                }
                
                volatility = stod(resultV[1]);

                //Now start to calculate PnL with input prices
                double close1d1, close2d1, open1d2, open2d2, close1d2, close2d2, k, PnL;
                //set K
                cout << "Please enter k: ";
                cin >> k;
                cout << endl;
                //set prices
                cout << "Please enter close price" << endl;
                cout << "Enter 1st stock close price on day 1: ";
                cin >> close1d1;
                cout << "Enter 2nd stock close price on day 1: ";
                cin >> close2d1;
                cout << "Enter 1st stock close price on day 2: ";
                cin >> close1d2;
                cout << "Enter 2nd stock close price on day 2: ";
                cin >> close2d2;
                cout << endl;
                cout << "Please enter open price" << endl;
                cout << "Enter 1st stock open price on day 2: ";
                cin >> open1d2;
                cout << "Enter 2nd stock open price on day 2: ";
                cin >> open2d2;
                cout << endl;

                int N1, N2;

                if (abs(close1d1 / close2d1 - open1d2 / open2d2) > k * volatility) {
                    N1 = -10000;
                    N2 = 10000 * (open1d2 / open2d2);
                    cout << "The trading strategy is:" << endl;
                    cout << "Short the " << symbol1 << " for 10000 shares" << endl;
                    cout << "Long the " << symbol2 << " for " << N2 << " shares" << endl;
                } else {
                    N1 = 10000;
                    N2 = -10000 * (open1d2 / open2d2);
                    cout << "The trading strategy is:" << endl;
                    cout << "Long the " << symbol1 << " for 10000 shares" << endl;
                    cout << "Short the " << symbol2 << " for " << abs(N2) << " shares" << endl;
                }

                PnL = N1 * (close1d2 - open1d2) + N2 * (close2d2 - open2d2);
                
                CloseDatabase(db);
                
                cout << "The daily PnL of the given stock pair is " << PnL << endl << endl;
                break;
            }

                    // H: Drop All tables
            case 'H':
            case 'h': {
                if (OpenDatabase(dbName.c_str(), db) != 0) {
                    return -1;
                }

                string StockPairs_sql_drop_table = string("DROP TABLE IF EXISTS StockPairs;");
                string PairOnePrices_sql_drop_table = string("DROP TABLE IF EXISTS PairOnePrices;");
                string PairTwoPrices_sql_drop_table = string("DROP TABLE IF EXISTS PairTwoPrices;");
                string PairPrices_sql_drop_table = string("DROP TABLE IF EXISTS PairPrices;");

                if (DropTable(db, PairPrices_sql_drop_table.c_str()) != 0) {
                    return -1;
                }
                if (DropTable(db, PairTwoPrices_sql_drop_table.c_str()) != 0) {
                    return -1;
                }
                if (DropTable(db, PairOnePrices_sql_drop_table.c_str()) != 0) {
                    return -1;
                }
                if (DropTable(db, StockPairs_sql_drop_table.c_str()) != 0) {
                    return -1;
                }

                CloseDatabase(db);

                cout << "All Table Dropped." << endl << endl;
                break;
            }

                    // X: Exit
            case 'X':
            case 'x': {
                cout << "Program shut down." << endl;
                flag = 0;
                break;
            }

            default: {
                cerr << "Invalid selection. Choose again." << endl;
                break;
            }
        }
    }
    return 0;
}
