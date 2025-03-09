/*****************************************************************************
 * Extended Library Management System
 * Demonstrates:
 *  - Four main classes: User (abstract), Book, Account, Library
 *  - Derived: Student, Faculty, Librarian from User
 *  - Overdue checks, fines for Students, faculty overdue block
 *  - Pay fines feature
 *  - Borrowing limit (3 for Student, 5 for Faculty)
 *  - Borrowing period (15 days / 30 days)
 *  - Writes data to BookData.csv, AccountData.csv
 *****************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
using namespace std;

// Cross-platform screen clear
void Clear() {
#if defined _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// We'll store a "day number" from epoch for dueDate/borrowDate
// A simple helper: get current day count from epoch
int currentDayFromEpoch() {
    time_t now = time(nullptr);
    return static_cast<int>( now / (24*3600) ); // floor
}

// Compute difference in days = (nowDay - dueDay)
int diffInDays(int day1, int day2) {
    return (day1 - day2);
}

// ---------------------------------------------------------------------
// Class: Book
//  - "status" = "Available" or "Borrowed"
//  - "borrowDate" & "dueDate": store day-from-epoch
// ---------------------------------------------------------------------
class Book {
private:
    string  title;
    string  author;
    string  isbn;
    string  publisher;
    int     year;
    string  status;       // "Available" or "Borrowed"
    int     borrowDate;   // day-from-epoch
    int     dueDate;      // day-from-epoch
    string  borrowedBy;   // userID or "-None-"

public:
    Book() : year(0), borrowDate(0), dueDate(0), status("Available"), borrowedBy("-None-") {}
    Book(const string &t, const string &a, const string &i,
         const string &p, int y)
       : title(t), author(a), isbn(i), publisher(p), year(y),
         status("Available"), borrowDate(0), dueDate(0), borrowedBy("-None-") {}

    // Getters
    string getTitle()      const { return title; }
    string getAuthor()     const { return author; }
    string getISBN()       const { return isbn; }
    string getPublisher()  const { return publisher; }
    int    getYear()       const { return year; }
    string getStatus()     const { return status; }
    int    getBorrowDate() const { return borrowDate; }
    int    getDueDate()    const { return dueDate; }
    string getBorrowedBy() const { return borrowedBy; }

    // Setters
    void setTitle(const string &s)     { title = s; }
    void setAuthor(const string &s)    { author = s; }
    void setISBN(const string &s)      { isbn = s; }
    void setPublisher(const string &s) { publisher = s; }
    void setYear(int y)                { year = y; }
    void setStatus(const string &s)    { status = s; }
    void setBorrowDate(int bd)         { borrowDate = bd; }
    void setDueDate(int dd)            { dueDate = dd; }
    void setBorrowedBy(const string&s) { borrowedBy = s; }

    // Helper to print
    void printInfo() const {
        cout << "Title="<<title<<", Auth="<<author
             <<", Year="<<year<<", Status="<<status
             <<", BorrowedBy="<<borrowedBy;
        if(status=="Borrowed") {
            cout <<", dueDay="<<dueDate;
        }
        cout<<"\n";
    }
};

// ---------------------------------------------------------------------
// Abstract base: User
//   - Derived: Student, Faculty, Librarian
//   - We store a "fine" in here. For Faculty, it remains 0 always
//   - We store a "borrowHistory" of titles returned in the past
// ---------------------------------------------------------------------
class User {
private:
    string name;
    string userID;
protected:
    int    fine;  // for students
    vector<string> borrowHistory; // list of titles previously returned

public:
    User(const string &nm, const string &id)
       : name(nm), userID(id), fine(0) {}
    virtual ~User() {}

    string getName() const   { return name; }
    string getUserID() const { return userID; }
    int    getFine() const   { return fine; }

    void setName(const string &n)   { name = n; }
    void setUserID(const string &id){ userID = id; }
    void setFine(int f)             { fine = f; }

    // Borrowing & returning are specialized
    //  - canBorrowMore(...) is used by the system to check user constraints
    //  - getBorrowDays() returns 15 or 30
    //  - handleOverdueBook(...) adds a fine if the user is a Student
    virtual bool canBorrowMore(const vector<Book*> &myBooks) = 0;
    virtual int  getBorrowDays() = 0; 
    virtual void handleOverdueBook(int daysOverdue) = 0; 

    // Add to history
    void addHistory(const string &title) {
        borrowHistory.push_back(title);
    }

    // Print history
    void showHistory() const {
        if(borrowHistory.empty()) {
            cout<<"No returned-book history.\n";
            return;
        }
        cout<<"Returned Books:\n";
        for(auto &h: borrowHistory) {
            cout<<" - "<<h<<"\n";
        }
    }

    // Called from "Pay Fine" menu for Student
    virtual void payFines() {
        // default no-op for faculty/librarian
    }

    // Provide a hook so Student can check if "fine>0 => block"
    virtual bool hasUnpaidFines() const {
        return (fine > 0);
    }
};

// ---------------------------------------------------------------------
// Student
//   - max 3 books
//   - 15 days borrowed
//   - fine = 10 rupees/day overdue
// ---------------------------------------------------------------------
class Student : public User {
public:
    static const int MAX_BOOKS = 3;
    static const int BORROW_DAYS = 15;
    static const int FINE_RATE  = 10;

    Student(const string &nm, const string &id)
       : User(nm, id) {}

    bool canBorrowMore(const vector<Book*> &myBooks) override {
        // block if fines >0
        if(hasUnpaidFines()) return false;
        // block if >=3 borrowed
        if((int)myBooks.size() >= MAX_BOOKS) return false;
        return true;
    }

    int getBorrowDays() override {
        return BORROW_DAYS;
    }

    void handleOverdueBook(int daysOverdue) override {
        // For student: fine += daysOverdue * 10
        if(daysOverdue>0) {
            setFine( getFine() + (daysOverdue * FINE_RATE) );
        }
    }

    void payFines() override {
        if(getFine()==0) {
            cout<<"No fines to pay.\n";
            return;
        }
        cout<<"Your total fine is "<<getFine()<<" rupees.\nPay now? (y/n): ";
        char c; cin>>c;
        if(c=='y'||c=='Y') {
            setFine(0);
            cout<<"Fines cleared.\n";
        } else {
            cout<<"Cancelled.\n";
        }
    }

    bool hasUnpaidFines() const override {
        return (getFine() > 0);
    }
};

// ---------------------------------------------------------------------
// Faculty
//   - max 5 books
//   - 30 days borrowed
//   - no fine, but cannot borrow if any book is overdue >60 days
// ---------------------------------------------------------------------
class Faculty : public User {
public:
    static const int MAX_BOOKS   = 5;
    static const int BORROW_DAYS = 30;

    Faculty(const string &nm, const string &id)
       : User(nm, id) {}

    bool canBorrowMore(const vector<Book*> &myBooks) override {
        // check if already 5
        if((int)myBooks.size()>= MAX_BOOKS) return false;
        // We also check if any is overdue >60 in the library code
        // to block new borrowing. 
        return true; 
    }

    int getBorrowDays() override {
        return BORROW_DAYS;
    }

    void handleOverdueBook(int daysOverdue) override {
        // Faculty => no fine
        // We'll just note that if overdue>60, they get blocked from new borrowing
    }

    // Faculty does not pay fines => no override needed
};

// ---------------------------------------------------------------------
// Librarian
//   - typically doesn't borrow books
// ---------------------------------------------------------------------
class Librarian : public User {
public:
    Librarian(const string &nm, const string &id)
       : User(nm, id) {}

    bool canBorrowMore(const vector<Book*> &myBooks) override {
        return false; // Librarian doesn't borrow
    }
    int getBorrowDays() override {
        return 0; // not used
    }
    void handleOverdueBook(int daysOverdue) override {
        // no-op
    }
};

// ---------------------------------------------------------------------
// Class: Account
//   - For login credentials
//   - Associates with a (User*) pointer
//   - role = "student","faculty","librarian"
// ---------------------------------------------------------------------
class Account {
private:
    string username;  // for login
    string password;  
    string role;      // "student","faculty","librarian"
    User*  userPtr;   // Student*, Faculty*, or Librarian*

public:
    Account(const string &un, const string &pw, const string &r, User* uptr)
      : username(un), password(pw), role(r), userPtr(uptr) {}

    string getUsername() const { return username; }
    string getPassword() const {return password;  }
    bool   checkPassword(const string &pw) const { return (pw == password); }
    string getRole() const { return role; }
    User*  getUser() const { return userPtr; }

    void   setPassword(const string &pw) { password = pw; }

    bool isStudent()   const { return (role=="student"); }
    bool isFaculty()   const { return (role=="faculty"); }
    bool isLibrarian() const { return (role=="librarian"); }
};

// ---------------------------------------------------------------------
// Class: Library
//   - Manages a vector<Book> and vector<Account>
//   - On startup, loads from CSV. On destruction, saves to CSV.
//   - Rebuilds "which books a user currently has" by scanning BookData
//     for "borrowedBy = that userID" each time we log in
// ---------------------------------------------------------------------
class Library {
private:
    vector<Book>    books;
    vector<Account> accounts;

public:
    Library() {
        loadBooks("BookData.csv");
        loadAccounts("AccountData.csv");
    }
    ~Library() {
        saveBooks("BookData.csv");
        saveAccounts("AccountData.csv");
        // Also must delete the user objects allocated in loadAccounts
        for(auto &acc : accounts) {
            delete acc.getUser();
        }
    }

    // ----------------------------
    // Book I/O
    // ----------------------------
    void loadBooks(const string &fname) {
        ifstream fin(fname);
        if(!fin.is_open()) {
            cerr<<"Could not open "<<fname<<". Will create on save.\n";
            return;
        }
        string line;
        while(getline(fin, line)) {
            if(line.size()<5) continue;
            // Format:
            // Title,Author,ISBN,Publisher,Year,status,borrowDate,dueDate,borrowedBy
            vector<string> tok;
            {
                stringstream ss(line);
                string temp;
                while(getline(ss,temp,',')) {
                    tok.push_back(temp);
                }
            }
            if(tok.size()<9) continue;
            Book b;
            b.setTitle(tok[0]);
            b.setAuthor(tok[1]);
            b.setISBN(tok[2]);
            b.setPublisher(tok[3]);
            b.setYear(stoi(tok[4]));
            b.setStatus(tok[5]);
            b.setBorrowDate(stoi(tok[6]));
            b.setDueDate(stoi(tok[7]));
            b.setBorrowedBy(tok[8]);
            books.push_back(b);
        }
        fin.close();
    }

    void saveBooks(const string &fname) {
        ofstream fout(fname, ios::out);
        for(size_t i=0; i<books.size(); i++){
            Book &b = books[i];
            fout<<b.getTitle()<<","
                <<b.getAuthor()<<","
                <<b.getISBN()<<","
                <<b.getPublisher()<<","
                <<b.getYear()<<","
                <<b.getStatus()<<","
                <<b.getBorrowDate()<<","
                <<b.getDueDate()<<","
                <<b.getBorrowedBy();
            if(i<books.size()-1) fout<<"\n";
        }
        fout.close();
    }

    // For convenience in code
    Book* findBookByTitle(const string &title) {
        for(auto &b : books) {
            if(b.getTitle()==title) return &b;
        }
        return nullptr;
    }

    void listAllBooks() {
        if(books.empty()){
            cout<<"No books.\n";
            return;
        }
        cout<<"--- All Books ---\n";
        for(auto &b : books) {
            b.printInfo();
        }
    }

    // Librarian actions
    void addBook(const string &t, const string &a, const string &i,
                 const string &p, int y) {
        Book b(t,a,i,p,y);
        books.push_back(b);
        cout<<"Book added.\n";
    }
    void removeBook(const string &title) {
        auto it = remove_if(books.begin(), books.end(), [&](Book &b){
            return (b.getTitle()==title);
        });
        if(it==books.end()) {
            cout<<"No book with that title.\n";
        } else {
            books.erase(it, books.end());
            cout<<"Removed.\n";
        }
    }

    // ----------------------------
    // Account / User I/O
    // ----------------------------
    void loadAccounts(const string &fname){
        ifstream fin(fname);
        if(!fin.is_open()) {
            cerr<<"Could not open "<<fname<<". Will create on save.\n";
            return;
        }
        string line;
        while(getline(fin, line)) {
            if(line.size()<5) continue;
            // Format:
            // username,password,role,userID,fine,historyBook1,historyBook2,...
            vector<string> tok;
            {
                stringstream ss(line);
                string temp;
                while(getline(ss,temp,',')) {
                    tok.push_back(temp);
                }
            }
            if(tok.size()<5) continue;
            string un   = tok[0];
            string pw   = tok[1];
            string role = tok[2];
            string uid  = tok[3];
            int    fn   = stoi(tok[4]); // fine

            // create user
            User* uptr=nullptr;
            if(role=="student") {
                auto st = new Student(un, uid);
                st->setFine(fn);
                uptr = st;
            } else if(role=="faculty") {
                auto fc = new Faculty(un, uid);
                fc->setFine(fn); 
                uptr = fc;
            } else if(role=="librarian") {
                auto lb = new Librarian(un, uid);
                lb->setFine(fn);
                uptr = lb;
            } else {
                cerr<<"Unknown role: "<<role<<"\n";
                continue;
            }
            // parse any further tokens as "borrowHistory"
            for(size_t i=5; i<tok.size(); i++){
                if(!tok[i].empty()) {
                    uptr->addHistory(tok[i]);
                }
            }
            // Make an Account
            Account acc(un, pw, role, uptr);
            accounts.push_back(acc);
        }
        fin.close();
    }

    void saveAccounts(const string &fname){
    ofstream fout(fname, ios::out);
    for(size_t i=0; i<accounts.size(); i++){
        Account &acc = accounts[i];
        User* uptr = acc.getUser();
        if(!uptr) continue; 

        fout<<acc.getUsername()<<","
            <<acc.getPassword()<<","  // ✅ Now saving actual password
            <<acc.getRole()<<","
            <<uptr->getUserID()<<","
            <<uptr->getFine();

        if(i<accounts.size()-1) fout<<"\n";
    }
    fout.close();
}


    // ----------------------------
    // We won't let you create new accounts at runtime in this example
    // but you can push them into "accounts" if needed.
    // ----------------------------

    // ----------------------------
    // Login
    // ----------------------------
    Account* login(const string &un, const string &pw) {
    for(auto &acc : accounts) {
        if(acc.getUsername() == un) {
            if(acc.checkPassword(pw)) {  // ✅ Uses checkPassword() here
                return &acc;
            } else {
                return nullptr;
            }
        }
    }
    return nullptr;
}

    // Reconstruct a user's "current borrowed books" by scanning "books" array
    // for Book::borrowedBy = user->userID
    vector<Book*> gatherUserBorrowed(const string &userID) {
        vector<Book*> res;
        for(auto &bk: books) {
            if(bk.getBorrowedBy() == userID && bk.getStatus()=="Borrowed") {
                res.push_back(&bk);
            }
        }
        return res;
    }

    // ----------------------------
    // The key operation: Borrow
    //   - If user is Faculty, we also check if they have a book overdue by >60 days
    //   - If user is Student, we check if fine>0 or if they've reached 3 books
    //   - If a book is "Available", we set it "Borrowed" w/ dueDate
    // ----------------------------
    void userBorrowBook(User* u, Book* b) {
    if(dynamic_cast<Student*>(u)) {
        auto userBooks = gatherUserBorrowed(u->getUserID());
        if (!u->canBorrowMore(userBooks)) {
            cout<<"Cannot borrow. You have reached the borrowing limit or have unpaid fines.\n";
            return;
        }
    } 

    if(dynamic_cast<Faculty*>(u)) {
        auto userBooks = gatherUserBorrowed(u->getUserID());
        for(auto &bk : userBooks) {
            int overdueDays = diffInDays(currentDayFromEpoch(), bk->getDueDate());
            if(overdueDays > 60) {
                cout<<"Faculty cannot borrow more books as they have an overdue book exceeding 60 days.\n";
                return;
            }
        }
    }

    if(b->getStatus() == "Borrowed") {
        cout<<"Book is already borrowed.\n";
        return;
    }

    // Update book status
    b->setStatus("Borrowed");
    b->setBorrowedBy(u->getUserID());
    int borrowDay = currentDayFromEpoch();
    int dueDay = borrowDay + u->getBorrowDays();
    b->setBorrowDate(borrowDay);
    b->setDueDate(dueDay);

    cout<<"Successfully borrowed: "<<b->getTitle()<<". Due in "<<u->getBorrowDays()<<" days.\n";
}


    // ----------------------------
    // Return Book
    //  - If overdue => compute daysOverdue
    //  - For Student => add fine
    //  - For Faculty => no fine
    //  - Then Book => status=Available, borrowedBy="-None-"
    //  - Add to user history
    // ----------------------------
    void userReturnBook(User* u, Book* b) {
    if(b->getStatus() == "Available") {
        cout<<"Book not borrowed.\n";
        return;
    }

    if(b->getBorrowedBy() != u->getUserID()) {
        cout<<"That book isn't borrowed by you.\n";
        return;
    }

    int today = currentDayFromEpoch();
    int overdueDays = diffInDays(today, b->getDueDate());

    if(overdueDays > 0) {
        if(dynamic_cast<Student*>(u)) {
            u->handleOverdueBook(overdueDays);
            cout<<"Student was fined "<<overdueDays * 10<<" rupees for overdue.\n";
        }
        if(dynamic_cast<Faculty*>(u) && overdueDays > 60) {
            cout<<"Faculty cannot borrow new books until this overdue book is cleared.\n";
        }
    } else {
        cout<<"Returned on time.\n";
    }

    // Update book status
    b->setStatus("Available");
    b->setBorrowedBy("-None-");
    b->setBorrowDate(0);
    b->setDueDate(0);

    // Add to user's history
    u->addHistory(b->getTitle());
    cout<<"Book returned successfully.\n";
}

};

// ---------------------------------------------------------------------
// Now a demonstration main:
// ---------------------------------------------------------------------
int main() {
    Library lib;
    while(true) {
        Clear();
        cout<<"=== LIBRARY SYSTEM ===\n"
            <<"1. Login\n"
            <<"0. Exit\n"
            <<"Choice: ";
        int c; cin>>c;
        if(!cin.good()) break;

        if(c==0) {
            cout<<"Exiting.\n";
            break;
        } else if(c==1) {
            Clear();
            cout<<"Username: ";
            string un; cin>>un;
            cout<<"Password: ";
            string pw; cin>>pw;
            Account* acc = lib.login(un, pw);
            if(!acc) {
                cout<<"Invalid login.\n";
                cin.ignore();cin.get();
                continue;
            }
            User* u = acc->getUser();
            Clear();
            // If Librarian
            if(acc->isLibrarian()) {
                while(true){
                    Clear();
                    cout<<"Hello Librarian "<<u->getName()<<" ["<<u->getUserID()<<"]\n"
                        <<"1. List all books\n"
                        <<"2. Add book\n"
                        <<"3. Remove book\n"
                        <<"0. Logout\n"
                        <<"Choice: ";
                    int lc; cin>>lc;
                    if(!cin.good()) break;
                    if(lc==0) break;
                    if(lc==1) {
                        lib.listAllBooks();
                        cin.ignore();cin.get();
                    } else if(lc==2) {
                        Clear();
                        cout<<"Title: ";
                        cin.ignore();
                        string t; getline(cin,t);
                        cout<<"Author: ";
                        string a; getline(cin,a);
                        cout<<"ISBN: ";
                        string i; getline(cin,i);
                        cout<<"Publisher: ";
                        string p; getline(cin,p);
                        cout<<"Year: ";
                        int y; cin>>y;
                        lib.addBook(t,a,i,p,y);
                        cin.ignore();cin.get();
                    } else if(lc==3) {
                        Clear();
                        cout<<"Enter title to remove: ";
                        cin.ignore();
                        string t; getline(cin,t);
                        lib.removeBook(t);
                        cin.ignore();cin.get();
                    } else {
                        cout<<"Invalid.\n";
                        cin.ignore();cin.get();
                    }
                }
            }
            else if(acc->isStudent()||acc->isFaculty()) {
                // Student or Faculty
                while(true) {
                    Clear();
                    cout<<"Hello "<<u->getName()<<" ["<<u->getUserID()<<"], role="<<acc->getRole()<<"\n"
                        <<"Your current fine: "<<u->getFine()<<"\n"
                        <<"1. List all books\n"
                        <<"2. Borrow a book\n"
                        <<"3. Return a book\n"
                        <<"4. Pay Fines (Student only)\n"
                        <<"5. Show returned-book history\n"
                        <<"0. Logout\n"
                        <<"Choice: ";
                    int uc; cin>>uc;
                    if(!cin.good()) break;
                    if(uc==0) break;
                    if(uc==1) {
                        lib.listAllBooks();
                        cin.ignore();cin.get();
                    }
                    else if(uc==2) {
                        // borrow
                        Clear();
                        cout<<"Enter book title to borrow: ";
                        cin.ignore();
                        string bt; getline(cin,bt);
                        Book* b = lib.findBookByTitle(bt);
                        if(!b) {
                            cout<<"No book found.\n";
                        } else {
                            lib.userBorrowBook(u,b);
                        }
                        cin.ignore();cin.get();
                    }
                    else if(uc==3) {
                        // return
                        Clear();
                        cout<<"Enter book title to return: ";
                        cin.ignore();
                        string bt; getline(cin, bt);
                        Book* b = lib.findBookByTitle(bt);
                        if(!b) {
                            cout<<"No such book.\n";
                        } else {
                            lib.userReturnBook(u,b);
                        }
                        cin.ignore();cin.get();
                    }
                    else if(uc==4) {
                        Clear();
                        if(acc->isStudent()) {
                            // pay
                            dynamic_cast<Student*>(u)->payFines();
                        } else {
                            cout<<"Faculty doesn't pay fines.\n";
                        }
                        cin.ignore();cin.get();
                    }
                    else if(uc==5) {
                        Clear();
                        u->showHistory();
                        cin.ignore();cin.get();
                    }
                    else {
                        cout<<"Invalid.\n";
                        cin.ignore();cin.get();
                    }
                }
            }
            else {
                cout<<"Unknown role.\n";
                cin.ignore();cin.get();
            }
        } else {
            cout<<"Invalid.\n";
            cin.ignore();cin.get();
        }
    }
    return 0;
}
