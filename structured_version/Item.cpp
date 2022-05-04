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

Item::Item() {}

Item::Item(int id, int w)
{
    this->id = id;
    this->w = w;
}
/**
 * Print to video std::out the information
 **/
void Item::show()
{
    std::cout << "Item|ID_" + std::to_string(this->id) + "\tW_" + std::to_string(w) + "\n";
}
Item::~Item()
{
}