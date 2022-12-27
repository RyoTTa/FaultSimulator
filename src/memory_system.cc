//
// Created by ryotta205 on 8/17/22.
//
#include "memory_system.h"
#include <bitset>

namespace dramfaultsim {


    NaiveMemorySystem::NaiveMemorySystem(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace)
            : MemorySystem(config, stat, fault_result, fault_trace) {
#ifndef TEST_MODE
        std::mt19937_64 gen(rd());
#endif
#ifdef TEST_MODE
        std::cout << "NaiveMemorySystem" << std::endl;
#endif
        //data_block_[Channel][Rank][BankGourp][Bank][Row][Col]
        data_block_ = new uint64_t ******[config_.channels];

        for (int i = 0; i < config_.channels; i++) {
            data_block_[i] = new uint64_t *****[config_.ranks];
            for (int j = 0; j < config_.ranks; j++) {
                data_block_[i][j] = new uint64_t ****[config_.bankgroups];
                for (int k = 0; k < config_.bankgroups; k++) {
                    data_block_[i][j][k] = new uint64_t ***[config_.banks_per_group];
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        data_block_[i][j][k][q] = new uint64_t **[config_.rows];
                        for (int e = 0; e < config_.rows; e++) {
                            data_block_[i][j][k][q][e] = new uint64_t *[config_.actual_colums];
                            for (int w = 0; w < config_.actual_colums; w++) {
                                data_block_[i][j][k][q][e][w] = new uint64_t[config_.BL];
                                if (config_.data_pattern_str == "Random") {
                                    //Generate Random Data Pattern
                                    for (int f = 0; f < config_.BL; f++) {
                                        data_block_[i][j][k][q][e][w][f] = gen();
                                    }
                                } else {
                                    //Generate Configuration Data Pattern
                                    for (int f = 0; f < config_.BL; f++) {
                                        data_block_[i][j][k][q][e][w][f] = config_.data_pattern;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (config_.fault_model == "NaiveFaultModel") {
            faultmodel_ = new NaiveFaultModel(config_, data_block_, stat_);
        } else if (config_.fault_model == "BetaDistFaultModel") {
            faultmodel_ = new BetaDistFaultModel(config_, data_block_, stat_);
        } else {
            config_.fault_model = "NaiveFaultModel";
            faultmodel_ = new NaiveFaultModel(config_, data_block_, stat_);
        }
        fault_data = new uint64_t[config_.BL];

    }

    NaiveMemorySystem::~NaiveMemorySystem() {
#ifdef TEST_MODE
        std::cout << "NaiveMemorySystem destructor" << std::endl;
#endif
        delete faultmodel_;

        for (int i = 0; i < config_.channels; i++) {
            for (int j = 0; j < config_.ranks; j++) {
                for (int k = 0; k < config_.bankgroups; k++) {
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        for (int e = 0; e < config_.rows; e++) {
                            for (int w = 0; w < config_.actual_colums; w++) {
                                delete[] data_block_[i][j][k][q][e][w];
                            }
                            delete[] data_block_[i][j][k][q][e];
                        }
                        delete[] data_block_[i][j][k][q];
                    }
                    delete[] data_block_[i][j][k];
                }
                delete[] data_block_[i][j];
            }
            delete[] data_block_[i];
        }
        delete[] data_block_;

        writer_.close();
    }

    void NaiveMemorySystem::RecvRequest(uint64_t addr, bool is_write, uint64_t *data) {
        /*
#ifdef TEST_MODE
        std::cout << "Recv Request  : 0x" << std::hex << addr << std::dec << (is_write ? "  WRITE" : "  READ")<< std::endl;
        for (int i=0; i < config_.BL; i++){
            std::cout << "Recv Data    "<< i <<": 0x" << std::hex << data[i] << std::dec << std::endl;
        }
#endif
         */
        //Translate Recv Address
        SetRecvAddress(addr);

        if (is_write == true) {
            Write(data);
        } else {
            Read(data);
        }

        fault_mask = faultmodel_->ErrorInjection(addr);
        FaultData(data);

        CheckFaultMask();

        return;
    }

    void NaiveMemorySystem::CheckFaultMask() {

        bool check = true;
        int fault_count_in_column = 0;
        int fault_count_in_BL = 0;

        for (int i = 0; i < config_.BL; i++) {
            if (fault_mask[i] != (uint64_t) 0) {

                fault_count_in_column = FaultCountInColumn(i);
                if (fault_count_in_column == 1)
                    stat_.single_fault_in_column_num++;
                if (fault_count_in_column == 2)
                    stat_.double_fault_in_column_num++;
                if (fault_count_in_column == 3)
                    stat_.triple_fault_in_column_num++;
                else
                    stat_.others_fault_in_column_num++;

                fault_count_in_BL += fault_count_in_column;
                //PrintFaultResult(i);
                if (fault_count_in_column != 0) {
                    fault_result_.PrintFaultResult(recv_addr_.hex_addr, i, fault_mask,
                                                   data_block_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column]);
                    stat_.fault_column_num++;
                }
            } else {
                stat_.correct_column_num++;
            }
        }
        if (fault_count_in_BL == 0)
            check = false;
        else if (fault_count_in_BL == 1)
            stat_.single_fault_in_BL_num++;
        else if (fault_count_in_BL == 2)
            stat_.double_fault_in_BL_num++;
        else if (fault_count_in_BL == 3)
            stat_.triple_fault_in_BL_num++;
        else
            stat_.others_fault_in_BL_num++;

        if (check) {
            stat_.fault_request_num++;
        } else {
            stat_.correct_request_num++;
        }

        if (check && config_.fault_trace_on) {
            fault_trace_.PrintFaultTrace(recv_addr_.hex_addr,
                                         data_block_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column],
                                         fault_data);
        }
    }

    int NaiveMemorySystem::FaultCountInColumn(int BL) {
        int count = 0;

        for (int i = 0; i < config_.bus_width; i++) {
            if ((((uint64_t) 1 & (fault_mask[BL] >> i)) == (uint64_t) 1) // Only 1 -> 0 Fault Count, High -> Low
                &&
                (data_block_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][BL]
                         >> i & (uint64_t) 1) == (uint64_t) 1) {
                count++;
            }
        }
        return count;
    }


    void NaiveMemorySystem::Read(uint64_t *data) {
        for (int i = 0; i < config_.BL; i++) {
            data_block_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i] = data[i];
        }

        return;
    }

    void NaiveMemorySystem::Write(uint64_t *data) {
        for (int i = 0; i < config_.BL; i++) {
            data_block_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i] = data[i];
        }

        return;
    }

    void NaiveMemorySystem::FaultData(uint64_t *data) {
        for (int i = 0; i < config_.BL; i++) {
            //fault_data[i] = data[i] ^ fault_mask[i];
            fault_data[i] = data[i] & (~fault_mask[i]);
        }
    }


}