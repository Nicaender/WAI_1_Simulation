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
#include <math.h>

#define STRING_SIZE 6
#define MINER_COUNT 100

enum difficult {easy, medium, hard}; // easy = only numbers, medium = numbers + Capital letter, hard = numbers + alphabets (capital and non capital)
int difficulty;

using namespace std;

Block* lastBlock;
mt19937 rng;
mutex found, msg;
sem_t nextMiner;
vector <thread*> threads;
bool exitState = false, allowNotification = false;

string randomizer(int length);
void minerThread(Miner* miner);
void menu(Miner *array[]);

int main()
{
    difficulty = easy+1;
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

    while(!exitState)
    {
        menu(minerArray);
    }

    for(int i = 0; i < (int)threads.size(); i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    while(firstBlock != nullptr)
    {
        Block* previousBlock = lastBlock->getPreviousBlock();
        delete lastBlock;
        lastBlock = previousBlock;
    }

    for(int i = 0; i < MINER_COUNT; i++)
    {
        delete minerArray[i];
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
        number = rng() % difficulty;
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
            string newBlockValue;
            int counter = lastBlock->getValue().size() / 2;
            while(counter != 0)
            {
                newBlockValue += lastBlock->getValue()[lastBlock->getValue().size() - counter];
                counter--;
            }
            newBlockValue += randomizer(STRING_SIZE - newBlockValue.size());
            Block* newBlock = new Block(newBlockValue, lastBlock);
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

void menu(Miner* minerArray[])
{
    string input;
    std::cout << "Welcome to simulation\n";
    std::cout << "(a) Scoreboard\n";
    std::cout << "(b) Toggle notification " << allowNotification << '\n';
    std::cout << "(c) Exit\n";
    std::cin >> input;
    if(input[0] == 'a')
    {
        for(unsigned int i = 0; i < MINER_COUNT - 1; i++)
        {
            int index = i;
            Miner* max = minerArray[i];
            for(unsigned int j = i+1; j < MINER_COUNT; j++)
            {
                if(minerArray[j]->getOwnedToken() < max->getOwnedToken())
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
            std::cout << "Miner " << std::setw(3) << std::setfill(' ') << minerArray[i]->getMyId() << ": I have " << std::setw(3) << std::setfill(' ') << minerArray[i]->getOwnedThread() << " threads and " << std::setw(3) << std::setfill(' ') << minerArray[i]->getOwnedToken() << " token(s)\n";
        }
    }
    else if(input[0] == 'b')
    {
        allowNotification = !allowNotification;
        system("CLS");
    }
    else if(input[0] == 'c')
    {
        exitState = true;
    }
    else
    {
        std::cout << "Invalid input\n";
    }
    cin.clear();
}
