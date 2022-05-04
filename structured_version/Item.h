#ifndef ITEM_H
#define ITEM_H

class Item
{
public:
    int id = -1;
    int w = -1; // weight of the item

    Item();
    Item(int id, int w);
    void show();
    ~Item();
};
#endif
