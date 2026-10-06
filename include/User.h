#ifndef USER_H
#define USER_H
// A node of the social network. The information STATE of a user lives in
// CascadeModel (one compact vector), so state can be copied cheaply for Monte Carlo.
class User {
private:
    int id;
public:
    explicit User(int userId);
    int getId() const;
};
#endif