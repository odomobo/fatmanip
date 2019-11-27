
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <exception>

#include "mio.hpp"

#include "util.hpp"
#include "boot_sector.hpp"

using namespace std;

// Create the file like:
// $ dd if=/dev/zero of=fat.img bs=1M count=100
// $ mkfs.vfat -F 16 -n "Josh Volume" fat.img

string const in_filename = "../fat.img";


int main()
{
    try
    {
        mio::mmap_source mmap{ in_filename };

        auto& tmp = place<boot_sector_base>(mmap, 0);
        if (tmp.is_fat32())
        {
            throw exception{ "FAT32 not yet implemented" };
        }
        else
        {

            auto& bs1216 = place<boot_sector_1216>(mmap, 0);

            cout << bs1216;
            
            // next step is to be able to read dirents, and then to be able to read entire files

            return 0;
        }
    }
    catch (exception& e)
    {
        cout << "Unhandled exception: " << e.what() << endl;
        return 1;
    }
}

