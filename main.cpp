#include <iostream>
#include <block.h>
#include <miner.h>
#include <random>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <iomanip>

#define STRING_SIZE 4
#define MINER_COUNT 100

using namespace std;

Block* lastBlock;
mt19937 rng;
mutex found, msg;
sem_t nextMiner;
vector <thread*> threads;
bool exitState = false, allowNotification = false;

string randomizer(int length);
void minerThread(Miner* miner);

int main()
{
    rng.seed(chrono::steady_clock::now().time_since_epoch().count());
    Miner* minerArray[MINER_COUNT];
    Block* firstBlock = new Block(randomizer(STRING_SIZE), nullptr);
    lastBlock = firstBlock;
    sem_init(&nextMiner, 0, 1);

    for(int i = 0; i < MINER_COUNT; i++) // Creating a miner with randomized threads, and then runs the thread
    {
        minerArray[i] = new Miner((rng() % 10) + 1, 0);
//        std::cout << "Creating " << minerArray[i]->getOwnedThread() << " thread for miner " << minerArray[i]->getMyId() << '\n';
        for(int j = 0; j < minerArray[i]->getOwnedThread(); j++)
        {
            thread* tmp = new thread(minerThread, minerArray[i]);
            threads.push_back(tmp);
        }
    }

    string input;
    while(!exitState)
    {
        std::cout << "Welcome to simulation\n";
        std::cout << "(a) Scoreboard\n";
        std::cout << "(b) Exit\n";
        std::cout << "(c) Toggle notification\n";
        std::cin >> input;
        if(input[0] == 'a')
        {
            for(unsigned int i = 0; i < MINER_COUNT - 1; i++)
            {
                int index = i;
                Miner* max = minerArray[i];
                for(unsigned int j = i+1; j < MINER_COUNT; j++)
                {
                    if(minerArray[j]->getOwnedToken() > max->getOwnedToken())
                    {
                        index = j;
                        max = minerArray[j];
                    }
                }
                minerArray[index] = minerArray[i];
                minerArray[i] = max;
            }
            for(int i = 0; i < MINER_COUNT; i++)
            {
                std::cout << "Miner " << std::setw(2) << std::setfill('0') << minerArray[i]->getMyId() << ": I have " << std::setw(2) << std::setfill('0') << minerArray[i]->getOwnedThread() << " threads and " << std::setw(2) << std::setfill('0') << minerArray[i]->getOwnedToken() << " token(s)\n";
            }
        }
        else if(input[0] == 'b')
        {
            exitState = true;
        }
        else if(input[0] == 'c')
        {
            allowNotification = !allowNotification;
            system("CLS");
        }
        else
        {
            std::cout << "Invalid input\n";
        }
        input.clear();
        cin.clear();
    }

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
        number = rng() % 1;
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
    while(true)
    {
        sem_wait(&nextMiner);
        if(exitState)
        {
            sem_post(&nextMiner);
            return;
        }
        string answer = randomizer(STRING_SIZE);
//        msg.lock();
//        std::cout << "Miner " << myMiner->getMyId() << ": I am trying with " << answer << '\n';
//        msg.unlock();
        if(lastBlock->getValue() == answer)
        {
            found.lock();
            if(lastBlock->getValue() != answer)
                continue;
            if(allowNotification)
            {
                msg.lock();
                std::cout << "Miner " << myMiner->getMyId() << ": I have found the answer, it was " << answer << " = " << lastBlock->getValue() << '\n';
                msg.unlock();
            }
            myMiner->setOwnedToken(myMiner->getOwnedToken() + 1);
            Block* newBlock = new Block(randomizer(STRING_SIZE), lastBlock);
            lastBlock = newBlock;
            if(allowNotification)
            {
                msg.lock();
                std::cout << "Miner " << myMiner->getMyId() << ": The new block value is " << lastBlock->getValue() << '\n';
                msg.unlock();
            }
            found.unlock();
        }
        sem_post(&nextMiner);
    }
}
