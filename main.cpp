#include <iostream>
#include <block.h>
#include <miner.h>
#include <random>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>
#include <semaphore.h>

#define STRING_SIZE 1
#define MINER_COUNT 10

using namespace std;

Block* lastBlock;
mt19937 rng;
mutex found, msg;
sem_t permissionStart, requestStart, createdMiner;
bool allowedStart = false;
vector <thread*> threads;

string randomizer(int length);
void minerThread(Miner* miner);
void systemFunc();

int main()
{
    rng.seed(chrono::steady_clock::now().time_since_epoch().count());
    Miner* minerArray[MINER_COUNT];
    Block* firstBlock = new Block(randomizer(STRING_SIZE), nullptr);
    lastBlock = firstBlock;
    sem_init(&permissionStart, 0, 0);
    sem_init(&requestStart, 0, 0);
    sem_init(&createdMiner, 0, 0);

    thread system(systemFunc);

    for(int i = 0; i < MINER_COUNT; i++)
    {
        minerArray[i] = new Miner((rng() % 10) + 1, 0);
        std::cout << "Creating " << minerArray[i]->getOwnedThread() << " thread for miner " << minerArray[i]->getMyId() << '\n';
        for(int j = 0; j < minerArray[i]->getOwnedThread(); j++)
        {
            thread* tmp = new thread(minerThread, minerArray[i]);
            threads.push_back(tmp);
        }
    }

    sem_post_multiple(&createdMiner, threads.size());

    for(int i = 0; i < (int)threads.size(); i++)
    {
        threads[i]->join();
    }

    delete firstBlock;
    return 0;
}

string randomizer(int length)
{
    string tmp;
    int number = 0;
    while((int)tmp.size() != length)
    {
        number = rng() % 3;
        if(number == 0)
        {
            tmp += (char(48 + (rng() % 10)));
        }
        else if(number == 1)
        {
            tmp += (char(65 + (rng() % 26)));
        }
        else if(number == 2)
        {
            tmp += (char(97 + (rng() % 26)));
        }
    }

    return tmp;
}

void minerThread(Miner *miner)
{
    Miner* myMiner = miner;
    sem_wait(&createdMiner);
    while(true)
    {
        if(!allowedStart)
        {
            sem_post(&requestStart);
            sem_wait(&permissionStart);
        }
        string answer = randomizer(STRING_SIZE);
        usleep(1000);
        msg.lock();
        std::cout << "Miner " << myMiner->getMyId() << ": I am trying with " << answer << '\n';
        msg.unlock();
        if(lastBlock->getValue() == answer)
        {
            found.lock();
            allowedStart = false;
            if(lastBlock->getValue() != answer)
                continue;
            msg.lock();
            std::cout << "Miner " << myMiner->getMyId() << ": I have found the answer, it was " << answer << " = " << lastBlock->getValue() << '\n';
            msg.unlock();
            myMiner->setOwnedToken(myMiner->getOwnedToken() + 1);
            Block* newBlock = new Block(randomizer(STRING_SIZE), lastBlock);
            lastBlock = newBlock;
            msg.lock();
            std::cout << "Miner " << myMiner->getMyId() << ": The new block value is " << lastBlock->getValue() << '\n';
            msg.unlock();
            found.unlock();
        }
    }
}

void systemFunc()
{
    int counter = 0;
    while(true)
    {
        sem_wait(&requestStart);
        counter++;
        if(counter == (int)threads.size())
        {
            counter = 0;
            sem_post_multiple(&permissionStart, (int)threads.size());
        }
    }
}
