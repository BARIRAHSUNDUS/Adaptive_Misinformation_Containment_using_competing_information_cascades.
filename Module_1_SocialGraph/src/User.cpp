#include "User.h"

User::User(int userId)
{
    id = userId;
}

int User::getId() const
{
    return id;
}
