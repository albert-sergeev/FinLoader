#ifndef AMIPIPEHOLDER_H
#define AMIPIPEHOLDER_H

#include<vector>
#include<filesystem>

class AmiPipeHolder
{
protected:
    std::vector<std::filesystem::directory_entry> vActivePipes;

public:


    AmiPipeHolder();

    std::vector<std::string> CheckActivePipes();
};

#endif // AMIPIPEHOLDER_H
