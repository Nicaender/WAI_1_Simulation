#ifndef MINER_H
#define MINER_H

#include <iostream>

class Miner
{
public:
    Miner(int thread, int token);

    int getOwnedThread() const;
    int getOwnedToken() const;
    void setOwnedToken(int newOwnedToken);
    int getMyId() const;

private:
    int ownedThread = 0;
    int ownedToken = 0;
    int myId = -1;
    static int maxId;
};

#endif // MINER_H
