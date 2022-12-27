//
// Created by ryotta205 on 8/31/22.
//
#include "faultresult.h"

namespace dramfaultsim{

    FaultResult::FaultResult(Config &config, Stat &stat)
    : config_(config), stat_(stat){
    }

    void FaultResult::PrintFaultResult(uint64_t addr, int BL, uint64_t *fault_mask, uint64_t *data) {
        SetRecvAddress(addr);

        std::string ID, CHANNEL, RANK, BANKGROUP, BANK, ROW, COL, DEV, BIT;

        CHANNEL = std::string(2 - std::to_string(recv_addr_channel).length(), '0').append(
                std::to_string(recv_addr_channel));
        RANK = std::string(2 - std::to_string(recv_addr_rank).length(), '0').append(std::to_string(recv_addr_rank));
        BANKGROUP = std::string(3 - std::to_string(recv_addr_bankgroup).length(), '0').append(
                std::to_string(recv_addr_bankgroup));
        BANK = std::string(3 - std::to_string(recv_addr_bank).length(), '0').append(std::to_string(recv_addr_bank));
        ROW = std::string(7 - std::to_string(recv_addr_row).length(), '0').append(std::to_string(recv_addr_row));
        COL = std::string(7 - std::to_string(recv_addr_column * config_.BL + BL).length(), '0').append(
                std::to_string(recv_addr_column * config_.BL + BL));


        for (int j = 0; j < config_.bus_width; j++) {
            if ((fault_mask[BL] >> (((config_.bus_width - 1) - j)) & (uint64_t) 1)
            &&  (data[BL] >> (((config_.bus_width - 1) - j)) & (uint64_t)1) == (uint64_t)1) {

                DEV = std::string(3 - std::to_string(j / config_.device_width).length(), '0').append(
                        std::to_string(j / config_.device_width));
                BIT = std::string(3 - std::to_string(j % config_.device_width).length(), '0').append(
                        std::to_string(j % config_.device_width));

                ID = CHANNEL + RANK + BANKGROUP + BANK + ROW + COL + DEV + BIT;
                writer_ << stat_.all_fault_bit_num++ << "," << stat_.repeat_round << "," << ID << "," << CHANNEL << ","
                        << RANK << "," << BANKGROUP << "," << BANK << "," << ROW << "," << COL << "," << DEV << ","
                        << BIT << "," << "F" << "\n";
            }
        }
    }
    void FaultResult::SetFaultResult(){
        writer_.open(
                config_.output_dir + "/" + config_.output_prefix + "_Round_" + std::to_string(stat_.repeat_round) +
                "_FaultResult.csv",
                std::ios::out | std::ios::trunc);
        if (writer_.fail()) {
            std::cerr << "Can't write fault result file - " << config_.output_dir << "/" << config_.output_prefix
                      << "_Round_" << stat_.repeat_round << "_FaultResult.csv" << std::endl;
            AbruptExit(__FILE__, __LINE__);
        }

        writer_ << ",Round,Id,Channel,Rank,BankGroup,Bank,Row,Col,Dev,Bit,Result\n";
    }

    void FaultResult::ResetFaultResult() {
        writer_.close();
    }
}