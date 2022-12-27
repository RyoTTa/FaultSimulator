#ifndef DRAMFAULTSIM_CONFIG_H
#define DRAMFAULTSIM_CONFIG_H

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "common.h"
#include "INIReader.h"

namespace dramfaultsim {

    class Config {

    public:
        //Basic Function
        Config(std::string config_file, std::string out_dir, std::string out_prefix, uint64_t request,
               std::string faultmap_read_path,
               std::string faultmap_write_path);

        ~Config();

        Address AddressMapping(uint64_t hex_addr) const;

        uint64_t ReverseAddressMapping(int ch, int ra, int bg, int ba, int ro, int co);

        void PrintInfo();
        //DRAM physical address structure

        int channel_size;
        int channels;
        int ranks;
        int banks;
        int bankgroups;
        int banks_per_group;
        int rows;
        int columns;
        int device_width;
        int bus_width;
        int devices_per_rank;
        int BL;
        int actual_colums;

        // Address mapping numbers
        int shift_bits;
        int ch_pos, ra_pos, bg_pos, ba_pos, ro_pos, co_pos;
        uint64_t ch_mask, ra_mask, bg_mask, ba_mask, ro_mask, co_mask;
        uint64_t effective_addr_mask;

        // System
        std::string address_mapping;
        std::string output_dir;
        std::string output_prefix;
        std::string txt_stats_name;
        std::string memory_system;
        std::string generator_system;
        std::string fault_model;
        std::string thread_model;
        int thread_num;

        // Running Request Number
        uint64_t num_request;

        // For Fault Map Read and Write
        std::string faultmap_read_path;
        std::string faultmap_write_path;

        // Computed parameters
        int request_size_bytes;

        // For Fault Model
        double fault_rate;
        std::string data_pattern_str; //For RandomGenerator or SequentialGenerator
        uint64_t data_pattern;
        double beta_dist_alpha, beta_dist_beta; // For Beta Distribution Fault Model
        bool fault_trace_on;
        int size_of_codeword;
        int num_codeword_burst;

        std::map<std::string, int> field_widths;
        std::vector<std::string> fields_name;


    private:
        INIReader *reader_;

        void CalculateSize();

        int GetInteger(const std::string &sec, const std::string &opt,
                       int default_val) const;

        double GetDouble(const std::string &sec, const std::string &opt,
                         double default_val) const;

        void InitDRAMParams();

        void InitSystemParams();

        void InitFaultModelParams();

        void SetAddressMapping();


    };


}
#endif //DRAMFAULTSIM_CONFIG_H