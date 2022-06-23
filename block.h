#ifndef BLOCK_H
#define BLOCK_H

#include <iostream>

class Block
{
public:
    Block(std::string value, Block* previousBlock);

    const std::string &getValue() const;
    Block *getPreviousBlock() const;

private:
    std::string value;
    Block* previousBlock = nullptr;
};

#endif // BLOCK_H
