#include "Item.h"
#include <iostream>

Item::Item() {}
Item::Item(int id, int w)
{
    this->id = id;
    this->w = w;
}
void Item::show()
{
    std::cout << "Item|ID_" + std::to_string(this->id) + "\tW_" + std::to_string(w) + "\n";
}
Item::~Item()
{
}