#include "miner.h"

int Miner::maxId = 1;

Miner::Miner(int thread, int token)
{
    this->ownedThread = thread;
    this->ownedToken = token;
    this->myId = maxId++;
}

int Miner::getOwnedThread() const
{
    return ownedThread;
}

int Miner::getOwnedToken() const
{
    return ownedToken;
}

void Miner::setOwnedToken(int newOwnedToken)
{
    ownedToken = newOwnedToken;
}

int Miner::getMyId() const
{
    return myId;
}
