#include "block.h"

Block::Block(std::string value, Block* previousBlock)
{
    this->value = value;
    this->previousBlock = previousBlock;
}

const std::string &Block::getValue() const
{
    return value;
}

Block *Block::getPreviousBlock() const
{
    return previousBlock;
}
