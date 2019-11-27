#pragma once

#include <string>
#include <iostream>

#include "types.hpp"

// Note that everything needs to be packed, because offsets aren't always on proper boundaries.
// Note that all specs were derived from https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system

static_assert(sizeof(char) == 1, "sizeof(char) must be 1 in order for this to build properly; sadly, this is not the case on your system (are you using windows with UTF-16 strings enabled?)");

#pragma pack(push, 1)
struct boot_sector_base {
    uint8_t jump_instruction[3];
    char oem_name[8];
    // offset 0x0B
    // DOS 2.0 BPB
    uint16_le bytes_per_logical_sector;
    uint8_t logical_sectors_per_cluster;
    uint16_le reserved_logical_sector_count;
    uint8_t file_allocation_table_count;
    uint16_le maximum_root_directory_entries; // only applies to FAT12, FAT16; 0 for FAT32
    uint16_le total_logical_sectors_1216; // only applies to FAT12, FAT16, but can still be 0. 0 for FAT32, in which case, look at offset 0x20 = 32 into boot sector
    uint8_t media_descriptor; // not really needed
    uint16_le logical_sectors_per_fat_1216; // only applies to FAT12, FAT16. 0 for FAT32, in which case, look at offset 0x24 = 36 into boot sector
    // offset 0x18
    // DOS 3.31 extra BPB entries
    uint16_le physical_sectors_per_track_floppy; // only applies to floppy-like devices
    uint16_le number_of_heads_floppy; // only applies to floppy-like devices
    uint32_le hidden_sectors_before_partition;
    uint32_le total_logical_sectors_32;
    // offset 0x24

    // end of fields

    std::string get_oem_name() const {
        return std::string{ oem_name, sizeof(oem_name) };
    }

    uint32_t get_total_logical_sectors() const
    {
        if (total_logical_sectors_1216 != 0)
            return total_logical_sectors_1216;
        else
            return total_logical_sectors_32;
    }

    int is_fat32() const {
        // TODO: this can be made more robust
        return logical_sectors_per_fat_1216 == 0;
    }
};
#pragma pack(pop)

// only print common values
inline std::ostream& operator<<(std::ostream& os, boot_sector_base const& bs)
{
    os << "OEM Name                       : " << bs.get_oem_name() << '\n';
    os << "Bytes per logical sector       : " << bs.bytes_per_logical_sector << '\n';
    os << "Logical sectors per cluster    : " << (int)bs.logical_sectors_per_cluster << '\n';
    os << "Reserved logical sector count  : " << bs.reserved_logical_sector_count << '\n';
    os << "File allocation table count    : " << (int)bs.file_allocation_table_count << '\n';
    os << "Hidden sectors before partition: " << bs.hidden_sectors_before_partition << '\n';
    os << "Total logical sectors          : " << bs.get_total_logical_sectors() << '\n';

    return os;
}

#pragma pack(push, 1)
struct boot_sector_1216 : public boot_sector_base {
    // offset 0x24
    // Extended BPB (FAT12 and FAT16)
    uint8_t physical_drive_number;
private:
    uint8_t _reserved;
public:
    uint8_t extended_bios_signature; // should be 0x29 to indicate the following 3 entries exist
    uint32_le volume_id;
    char partition_volume_label[11]; // blank-padded
    char file_system_type[8]; // blank-padded
    // offset 0x44

    // end of fields

    std::string get_partition_volume_label() const {
        return std::string{ partition_volume_label, sizeof(partition_volume_label) };
    }

    std::string get_file_system_type() const {
        return std::string{ file_system_type, sizeof(file_system_type) };
    }

    // TODO: maybe move these calculations into base? depends if we'll need them for FAT32, or even for calculating FAT32

    // This seems to be calculating the same value as is being used.
    // Got algorithm from http://homepage.ntlworld.com/jonathan.deboynepollard/FGA/determining-fat-widths.html
    // Although you probably need to use archive.org: https://web.archive.org/web/20160604095557/http://homepage.ntlworld.com/jonathan.deboynepollard/FGA/determining-fat-widths.html
    int get_data_start_sector() const {
        int sectors_in_root_directory = (maximum_root_directory_entries * 32 + bytes_per_logical_sector - 1) / bytes_per_logical_sector; // this performs a ceiling instead of a floor
        int data_start = reserved_logical_sector_count + file_allocation_table_count * logical_sectors_per_fat_1216 + sectors_in_root_directory;
        return data_start;
    }

    int get_cluster_count() const {
        int data_start = get_data_start_sector();
        int cluster_count = 2 + (get_total_logical_sectors() - data_start) / logical_sectors_per_cluster;
        return cluster_count;
    }

    int get_fat_type() const {
        // TODO: maybe should make this more robust, check more than just this
        if (get_cluster_count() <= 4086)
            return 12;
        else
            return 16;
    }
};
#pragma pack(pop)

// TODO: boot_sector_32

inline std::ostream& operator<<(std::ostream& os, boot_sector_1216 const& bs)
{
    os << "FAT type                       : FAT" << bs.get_fat_type() << '\n';
    // first print all base values
    os << reinterpret_cast<boot_sector_base const&>(bs);
    os << "Maximum root directory entries : " << bs.maximum_root_directory_entries << '\n';
    os << "Physical drive number          : " << hx(2) << (int)bs.physical_drive_number << restore << '\n';
    os << "Logical sectors per FAT        : " << bs.logical_sectors_per_fat_1216 << '\n';
    os << "Calculated data start sector   : " << bs.get_data_start_sector() << '\n';
    os << "Calculated cluster count       : " << bs.get_cluster_count() << '\n';

    return os;
}

