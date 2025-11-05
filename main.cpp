#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

// Constants
const string ROOT_USERNAME = "root";
const string ROOT_PASSWORD = "sjtu";
const int ROOT_PRIVILEGE = 7;

// Data structures
struct User {
    string userID;
    string password;
    string username;
    int privilege;

    User(string id = "", string pwd = "", string name = "", int priv = 1)
        : userID(id), password(pwd), username(name), privilege(priv) {}
};

struct Book {
    string ISBN;
    string bookName;
    string author;
    string keyword;
    double price;
    int stockQuantity;

    Book(string isbn = "", string name = "", string auth = "", string kw = "",
         double p = 0.0, int stock = 0)
        : ISBN(isbn), bookName(name), author(auth), keyword(kw), price(p), stockQuantity(stock) {}
};

struct Transaction {
    string type; // "buy" or "import"
    double amount;

    Transaction(string t = "", double a = 0.0)
        : type(t), amount(a) {}
};

// Global variables
vector<User> users;
vector<Book> books;
vector<Transaction> transactions;
vector<string> loginStack;
string selectedISBN = "";

// File names
const string USER_FILE = "users.dat";
const string BOOK_FILE = "books.dat";
const string TRANSACTION_FILE = "transactions.dat";

// Function declarations
void initializeSystem();
void loadData();
void saveData();
void processCommand(const string& command);
void executeSu(const vector<string>& tokens);
void executeLogout();
void executeRegister(const vector<string>& tokens);
void executePasswd(const vector<string>& tokens);
void executeUseradd(const vector<string>& tokens);
void executeDelete(const vector<string>& tokens);
void executeShow(const vector<string>& tokens);
void executeBuy(const vector<string>& tokens);
void executeSelect(const vector<string>& tokens);
void executeModify(const vector<string>& tokens);
void executeImport(const vector<string>& tokens);
void executeShowFinance(const vector<string>& tokens);
void executeLog();
void executeReportFinance();
void executeReportEmployee();

int getCurrentPrivilege();
User* getCurrentUser();
bool userExists(const string& userID);
User* getUser(const string& userID);
bool bookExists(const string& ISBN);
Book* getBook(const string& ISBN);
vector<string> parseCommand(const string& command);
string trim(const string& str);

int main() {
    initializeSystem();

    string line;
    while (getline(cin, line)) {
        if (line == "quit" || line == "exit") {
            break;
        }
        processCommand(line);
    }

    saveData();
    return 0;
}

void initializeSystem() {
    // Check if this is first run by checking if user file exists
    ifstream userFile(USER_FILE);
    if (!userFile.good()) {
        // First run - create root user
        User root(ROOT_USERNAME, ROOT_PASSWORD, "root", ROOT_PRIVILEGE);
        users.push_back(root);
        saveData();
    } else {
        loadData();
    }
}

void loadData() {
    // Load users
    ifstream userFile(USER_FILE);
    if (userFile.is_open()) {
        string line;
        while (getline(userFile, line)) {
            stringstream ss(line);
            string id, pwd, name;
            int priv;
            ss >> id >> pwd >> name >> priv;
            User user(id, pwd, name, priv);
            users.push_back(user);
        }
        userFile.close();
    }

    // Load books
    ifstream bookFile(BOOK_FILE);
    if (bookFile.is_open()) {
        string line;
        while (getline(bookFile, line)) {
            stringstream ss(line);
            string isbn, name, auth, kw;
            double price;
            int stock;
            ss >> isbn >> name >> auth >> kw >> price >> stock;
            Book book(isbn, name, auth, kw, price, stock);
            books.push_back(book);
        }
        bookFile.close();
    }

    // Load transactions
    ifstream transFile(TRANSACTION_FILE);
    if (transFile.is_open()) {
        string line;
        while (getline(transFile, line)) {
            stringstream ss(line);
            string type;
            double amount;
            ss >> type >> amount;
            Transaction trans(type, amount);
            transactions.push_back(trans);
        }
        transFile.close();
    }
}

void saveData() {
    // Save users
    ofstream userFile(USER_FILE);
    for (const auto& user : users) {
        userFile << user.userID << " " << user.password << " "
                 << user.username << " " << user.privilege << "\n";
    }
    userFile.close();

    // Save books
    ofstream bookFile(BOOK_FILE);
    for (const auto& book : books) {
        bookFile << book.ISBN << " " << book.bookName << " "
                 << book.author << " " << book.keyword << " "
                 << fixed << setprecision(2) << book.price << " "
                 << book.stockQuantity << "\n";
    }
    bookFile.close();

    // Save transactions
    ofstream transFile(TRANSACTION_FILE);
    for (const auto& trans : transactions) {
        transFile << trans.type << " " << fixed << setprecision(2)
                  << trans.amount << "\n";
    }
    transFile.close();
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" ");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" ");
    return str.substr(start, end - start + 1);
}

vector<string> parseCommand(const string& command) {
    vector<string> tokens;
    string trimmed = trim(command);

    if (trimmed.empty()) return tokens;

    stringstream ss(trimmed);
    string token;

    while (ss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

void processCommand(const string& command) {
    vector<string> tokens = parseCommand(command);

    if (tokens.empty()) {
        return;
    }

    string cmd = tokens[0];

    try {
        if (cmd == "su") {
            executeSu(tokens);
        } else if (cmd == "logout") {
            executeLogout();
        } else if (cmd == "register") {
            executeRegister(tokens);
        } else if (cmd == "passwd") {
            executePasswd(tokens);
        } else if (cmd == "useradd") {
            executeUseradd(tokens);
        } else if (cmd == "delete") {
            executeDelete(tokens);
        } else if (cmd == "show") {
            if (tokens.size() > 1 && tokens[1] == "finance") {
                executeShowFinance(tokens);
            } else {
                executeShow(tokens);
            }
        } else if (cmd == "buy") {
            executeBuy(tokens);
        } else if (cmd == "select") {
            executeSelect(tokens);
        } else if (cmd == "modify") {
            executeModify(tokens);
        } else if (cmd == "import") {
            executeImport(tokens);
        } else if (cmd == "log") {
            executeLog();
        } else if (cmd == "report") {
            if (tokens.size() > 1) {
                if (tokens[1] == "finance") {
                    executeReportFinance();
                } else if (tokens[1] == "employee") {
                    executeReportEmployee();
                } else {
                    cout << "Invalid\n";
                }
            } else {
                cout << "Invalid\n";
            }
        } else {
            cout << "Invalid\n";
        }
    } catch (const exception& e) {
        cout << "Invalid\n";
    }
}

// Implement command functions
void executeSu(const vector<string>& tokens) {
    if (tokens.size() < 2 || tokens.size() > 3) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    User* user = getUser(userID);

    if (!user) {
        cout << "Invalid\n";
        return;
    }

    if (tokens.size() == 3) {
        string password = tokens[2];
        if (user->password != password) {
            cout << "Invalid\n";
            return;
        }
    } else {
        // No password provided - check if current privilege is higher
        if (getCurrentPrivilege() <= user->privilege) {
            cout << "Invalid\n";
            return;
        }
    }

    loginStack.push_back(userID);
    // Clear selected book when switching users
    selectedISBN = "";
}

void executeLogout() {
    if (loginStack.empty()) {
        cout << "Invalid\n";
        return;
    }

    loginStack.pop_back();
    // Clear selected book when logging out
    selectedISBN = "";
}

void executeRegister(const vector<string>& tokens) {
    if (tokens.size() != 4) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    string password = tokens[2];
    string username = tokens[3];

    if (userExists(userID)) {
        cout << "Invalid\n";
        return;
    }

    User newUser(userID, password, username, 1);
    users.push_back(newUser);
}

void executePasswd(const vector<string>& tokens) {
    if (tokens.size() < 3 || tokens.size() > 4) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    User* user = getUser(userID);

    if (!user) {
        cout << "Invalid\n";
        return;
    }

    if (tokens.size() == 3) {
        // Only new password provided - must be root
        if (getCurrentPrivilege() != 7) {
            cout << "Invalid\n";
            return;
        }
        string newPassword = tokens[2];
        user->password = newPassword;
    } else {
        // Both current and new password provided
        string currentPassword = tokens[2];
        string newPassword = tokens[3];

        if (user->password != currentPassword) {
            cout << "Invalid\n";
            return;
        }
        user->password = newPassword;
    }
}

void executeUseradd(const vector<string>& tokens) {
    if (tokens.size() != 5) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    string password = tokens[2];
    int privilege = stoi(tokens[3]);
    string username = tokens[4];

    if (privilege != 1 && privilege != 3 && privilege != 7) {
        cout << "Invalid\n";
        return;
    }

    if (privilege >= getCurrentPrivilege()) {
        cout << "Invalid\n";
        return;
    }

    if (userExists(userID)) {
        cout << "Invalid\n";
        return;
    }

    User newUser(userID, password, username, privilege);
    users.push_back(newUser);
}

void executeDelete(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];

    if (!userExists(userID)) {
        cout << "Invalid\n";
        return;
    }

    // Check if user is logged in
    for (const auto& loggedInUser : loginStack) {
        if (loggedInUser == userID) {
            cout << "Invalid\n";
            return;
        }
    }

    // Remove user
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->userID == userID) {
            users.erase(it);
            break;
        }
    }
}

void executeShow(const vector<string>& tokens) {
    if (getCurrentPrivilege() < 1) {
        cout << "Invalid\n";
        return;
    }

    vector<Book> results;

    if (tokens.size() == 1) {
        // Show all books
        results = books;
    } else {
        // Show with filter
        string filter = tokens[1];

        if (filter.find("-ISBN=") == 0) {
            string ISBN = filter.substr(6);
            for (const auto& book : books) {
                if (book.ISBN == ISBN) {
                    results.push_back(book);
                }
            }
        } else if (filter.find("-name=") == 0) {
            string name = filter.substr(6);
            for (const auto& book : books) {
                if (book.bookName == name) {
                    results.push_back(book);
                }
            }
        } else if (filter.find("-author=") == 0) {
            string author = filter.substr(8);
            for (const auto& book : books) {
                if (book.author == author) {
                    results.push_back(book);
                }
            }
        } else if (filter.find("-keyword=") == 0) {
            string keyword = filter.substr(9);
            // Check if keyword contains multiple keywords
            if (keyword.find('|') != string::npos) {
                cout << "Invalid\n";
                return;
            }
            for (const auto& book : books) {
                if (book.keyword.find(keyword) != string::npos) {
                    results.push_back(book);
                }
            }
        } else {
            cout << "Invalid\n";
            return;
        }
    }

    // Sort by ISBN
    sort(results.begin(), results.end(),
         [](const Book& a, const Book& b) { return a.ISBN < b.ISBN; });

    // Output results
    for (const auto& book : results) {
        cout << book.ISBN << "\t" << book.bookName << "\t"
             << book.author << "\t" << book.keyword << "\t"
             << fixed << setprecision(2) << book.price << "\t"
             << book.stockQuantity << "\n";
    }

    if (results.empty()) {
        cout << "\n";
    }
}

void executeBuy(const vector<string>& tokens) {
    if (tokens.size() != 3) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() < 1) {
        cout << "Invalid\n";
        return;
    }

    string ISBN = tokens[1];
    int quantity = stoi(tokens[2]);

    if (quantity <= 0) {
        cout << "Invalid\n";
        return;
    }

    Book* book = getBook(ISBN);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    if (book->stockQuantity < quantity) {
        cout << "Invalid\n";
        return;
    }

    double total = book->price * quantity;
    book->stockQuantity -= quantity;

    // Record transaction
    Transaction trans("buy", total);
    transactions.push_back(trans);

    cout << fixed << setprecision(2) << total << "\n";
}

void executeSelect(const vector<string>& tokens) {
    if (tokens.size() != 2) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    string ISBN = tokens[1];

    if (!bookExists(ISBN)) {
        // Create new book
        Book newBook(ISBN);
        books.push_back(newBook);
    }

    selectedISBN = ISBN;
}

void executeModify(const vector<string>& tokens) {
    if (tokens.size() < 2) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    if (selectedISBN.empty()) {
        cout << "Invalid\n";
        return;
    }

    Book* book = getBook(selectedISBN);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    // Check for duplicate parameters
    map<string, bool> paramUsed;

    for (size_t i = 1; i < tokens.size(); i++) {
        string token = tokens[i];

        if (token.find("-ISBN=") == 0) {
            if (paramUsed["ISBN"]) {
                cout << "Invalid\n";
                return;
            }
            paramUsed["ISBN"] = true;

            string newISBN = token.substr(6);
            if (newISBN == selectedISBN) {
                cout << "Invalid\n";
                return;
            }
            if (bookExists(newISBN)) {
                cout << "Invalid\n";
                return;
            }
            book->ISBN = newISBN;
            selectedISBN = newISBN;

        } else if (token.find("-name=") == 0) {
            if (paramUsed["name"]) {
                cout << "Invalid\n";
                return;
            }
            paramUsed["name"] = true;

            string name = token.substr(6);
            book->bookName = name;

        } else if (token.find("-author=") == 0) {
            if (paramUsed["author"]) {
                cout << "Invalid\n";
                return;
            }
            paramUsed["author"] = true;

            string author = token.substr(8);
            book->author = author;

        } else if (token.find("-keyword=") == 0) {
            if (paramUsed["keyword"]) {
                cout << "Invalid\n";
                return;
            }
            paramUsed["keyword"] = true;

            string keyword = token.substr(9);
            // Check for duplicate segments
            vector<string> segments;
            stringstream ss(keyword);
            string segment;
            while (getline(ss, segment, '|')) {
                if (find(segments.begin(), segments.end(), segment) != segments.end()) {
                    cout << "Invalid\n";
                    return;
                }
                segments.push_back(segment);
            }
            book->keyword = keyword;

        } else if (token.find("-price=") == 0) {
            if (paramUsed["price"]) {
                cout << "Invalid\n";
                return;
            }
            paramUsed["price"] = true;

            double price = stod(token.substr(7));
            book->price = price;

        } else {
            cout << "Invalid\n";
            return;
        }
    }
}

void executeImport(const vector<string>& tokens) {
    if (tokens.size() != 3) {
        cout << "Invalid\n";
        return;
    }

    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    if (selectedISBN.empty()) {
        cout << "Invalid\n";
        return;
    }

    int quantity = stoi(tokens[1]);
    double totalCost = stod(tokens[2]);

    if (quantity <= 0 || totalCost <= 0) {
        cout << "Invalid\n";
        return;
    }

    Book* book = getBook(selectedISBN);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    book->stockQuantity += quantity;

    // Record transaction
    Transaction trans("import", -totalCost);
    transactions.push_back(trans);
}

void executeShowFinance(const vector<string>& tokens) {
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    int count = transactions.size();
    if (tokens.size() > 1) {
        count = stoi(tokens[1]);
    }

    if (count < 0) {
        cout << "Invalid\n";
        return;
    }

    if (count > (int)transactions.size()) {
        cout << "Invalid\n";
        return;
    }

    if (count == 0) {
        cout << "\n";
        return;
    }

    double income = 0.0, expenditure = 0.0;

    // Get last 'count' transactions
    int startIdx = max(0, (int)transactions.size() - count);
    for (int i = startIdx; i < (int)transactions.size(); i++) {
        if (transactions[i].type == "buy") {
            income += transactions[i].amount;
        } else if (transactions[i].type == "import") {
            expenditure += abs(transactions[i].amount);
        }
    }

    cout << "+ " << fixed << setprecision(2) << income
         << " - " << fixed << setprecision(2) << expenditure << "\n";
}

void executeLog() {
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    // Simple log implementation
    cout << "=== System Log ===\n";
    cout << "Total users: " << users.size() << "\n";
    cout << "Total books: " << books.size() << "\n";
    cout << "Total transactions: " << transactions.size() << "\n";
}

void executeReportFinance() {
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    cout << "=== Financial Report ===\n";
    double totalIncome = 0.0, totalExpenditure = 0.0;

    for (const auto& trans : transactions) {
        if (trans.type == "buy") {
            totalIncome += trans.amount;
        } else if (trans.type == "import") {
            totalExpenditure += abs(trans.amount);
        }
    }

    cout << "Total Income: " << fixed << setprecision(2) << totalIncome << "\n";
    cout << "Total Expenditure: " << fixed << setprecision(2) << totalExpenditure << "\n";
    cout << "Net Profit: " << fixed << setprecision(2) << (totalIncome - totalExpenditure) << "\n";
}

void executeReportEmployee() {
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    cout << "=== Employee Work Report ===\n";
    cout << "Currently logged in users: " << loginStack.size() << "\n";
    for (const auto& userID : loginStack) {
        User* user = getUser(userID);
        if (user) {
            cout << "- " << user->userID << " (" << user->username << ") - Privilege: " << user->privilege << "\n";
        }
    }
}

// Helper functions
int getCurrentPrivilege() {
    if (loginStack.empty()) {
        return 0;
    }
    User* user = getUser(loginStack.back());
    return user ? user->privilege : 0;
}

User* getCurrentUser() {
    if (loginStack.empty()) {
        return nullptr;
    }
    return getUser(loginStack.back());
}

bool userExists(const string& userID) {
    return getUser(userID) != nullptr;
}

User* getUser(const string& userID) {
    for (auto& user : users) {
        if (user.userID == userID) {
            return &user;
        }
    }
    return nullptr;
}

bool bookExists(const string& ISBN) {
    return getBook(ISBN) != nullptr;
}

Book* getBook(const string& ISBN) {
    for (auto& book : books) {
        if (book.ISBN == ISBN) {
            return &book;
        }
    }
    return nullptr;
}