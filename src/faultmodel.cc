//
// Created by ryotta205 on 8/19/22.
//
#include "faultmodel.h"
#include <thread>
#include <bitset>
#include <boost/math/special_functions/beta.hpp>

namespace dramfaultsim {
    NaiveFaultModel::NaiveFaultModel(Config &config, uint64_t *******data_block, Stat &stat)
            : FaultModel(config, data_block, stat) {
#ifndef TEST_MODE
        std::mt19937_64 gen(rd());
#endif

#ifdef TEST_MODE
        std::cout << "NaiveFaultModel" << std::endl;
#endif
        //data_block_[Channel][Rank][BankGourp][Bank][Row][Act_Col][BL]
        fault_map_ = new FaultStruct ******[config_.channels];

        for (int i = 0; i < config_.channels; i++) {
            fault_map_[i] = new FaultStruct *****[config_.ranks];
            for (int j = 0; j < config_.ranks; j++) {
                fault_map_[i][j] = new FaultStruct ****[config_.bankgroups];
                for (int k = 0; k < config_.bankgroups; k++) {
                    fault_map_[i][j][k] = new FaultStruct ***[config_.banks_per_group];
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        fault_map_[i][j][k][q] = new FaultStruct **[config_.rows];
                        for (int e = 0; e < config_.rows; e++) {
                            fault_map_[i][j][k][q][e] = new FaultStruct *[config_.actual_colums];
                            for (int w = 0; w < config_.actual_colums; w++) {
                                fault_map_[i][j][k][q][e][w] = new FaultStruct[config_.BL];
                                for (int f = 0; f < config_.BL; f++) {
                                    fault_map_[i][j][k][q][e][w][f] = {0, 0};
                                }
                            }
                        }
                    }
                }
            }
        }

        ErrorMask = new uint64_t[config_.BL];

        if (!config_.faultmap_read_path.empty()) {
            FaultMapReader *reader_ = new FaultMapReader(config_, config_.faultmap_read_path,
                                                         fault_map_);
#ifdef TEST_MODE
            std::cout << "FaultMap Reader" << std::endl;
#endif
            fault_map_ = reader_->Read();

            delete reader_;
        } else {
#ifdef TEST_MODE
            std::cout << "HardFaultGenerator" << std::endl;
#endif
            HardFaultGenerator();
#ifdef TEST_MODE
            std::cout << "VRTFaultGenerator" << std::endl;
#endif
            VRTErrorGenerator();
        }

        num_all_cell = 0;
        num_hard_fault_cell = 0;
    }

    NaiveFaultModel::~NaiveFaultModel() {
#ifdef TEST_MODE
        std::cout << "NaiveFaultModel destructor" << std::endl;
#endif

        if (!config_.faultmap_write_path.empty()) {
            FaultMapWriter *writer_ = new FaultMapWriter(config_, config_.faultmap_write_path,
                                                         fault_map_);

#ifdef TEST_MODE
            std::cout << "FaultMap Writer" << std::endl;
#endif
            writer_->Write();

            delete writer_;
        }


        for (int i = 0; i < config_.channels; i++) {
            for (int j = 0; j < config_.ranks; j++) {
                for (int k = 0; k < config_.bankgroups; k++) {
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        for (int e = 0; e < config_.rows; e++) {
                            for (int w = 0; w < config_.actual_colums; w++) {
                                delete[] fault_map_[i][j][k][q][e][w];
                            }
                            delete[] fault_map_[i][j][k][q][e];
                        }
                        delete[] fault_map_[i][j][k][q];
                    }
                    delete[] fault_map_[i][j][k];
                }
                delete[] fault_map_[i][j];
            }
            delete[] fault_map_[i];
        }
        delete[] fault_map_;
    }

    uint64_t *NaiveFaultModel::ErrorInjection(uint64_t addr) {

        for (int i = 0; i < config_.BL; i++) {
            ErrorMask[i] = 0;
        }
        SetRecvAddress(addr);

        HardFaultError();

        VRTFaultError();

        return ErrorMask;
    }

    void NaiveFaultModel::HardFaultError() {
        for (int i = 0; i < config_.BL; i++) {
            ErrorMask[i] |= fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].hardfault;

            for (int j = 0; j < config_.bus_width; j++) {
                if (((ErrorMask[i]) & ((uint64_t) 1) << j)) {
                    stat_.hard_fault_bit_num++;
                }
            }
        }
    }

    void NaiveFaultModel::VRTFaultError() {
        int rate;
        bool fault;

        for (int i = 0; i < config_.BL; i++) {
            if (fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt_size ==
                0)
                continue;
            for (int j = 0; j <
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt_size; j++) {
                rate = GetRandomInt(1, 1000);
                fault = false;
                if (std::get<1>(
                        fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) >=
                    rate) {
                    fault = true;
                }

                /*

                if (fault != std::get<2>(
                        fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j])) {
                    rate = GetRandomInt(1, 1000);

                    if (std::get<1>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) >=
                        rate) {
                        fault = true;
                    }
                }
                 */


                if (fault) {
                    ErrorMask[i] |= (uint64_t) 1 << ((config_.bus_width - 1) -
                                                     std::get<0>(
                                                             fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]));
                    stat_.vrt_fault_bit_num++;

                    std::get<2>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) = true;

                } else {
                    std::get<2>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) = false;
                }

            }
        }
    }

    void NaiveFaultModel::DataDependentFaultError() {

    }

    void NaiveFaultModel::HardFaultGenerator() {
        num_all_cell =
                (uint64_t) config_.channels * (uint64_t) config_.ranks * (uint64_t) config_.bankgroups *
                (uint64_t) config_.banks_per_group * (uint64_t) config_.rows *
                (uint64_t) config_.columns * (uint64_t) config_.bus_width;

        num_hard_fault_cell = (uint64_t) ((double) num_all_cell * config_.fault_rate / 100);

        if (config_.thread_model == "SingleThread") {
            std::cout << "Single" << std::endl;
            HardFaultGeneratorThread(num_hard_fault_cell);
        } else if (config_.thread_model == "MultiThread") {
            std::cout << "Multi" << std::endl;
            std::thread _t[config_.thread_num];

            for (int i = 0; i < config_.thread_num; i++) {
                _t[i] = std::thread(&NaiveFaultModel::HardFaultGeneratorThread, this,
                                    num_hard_fault_cell / (uint64_t) config_.thread_num);
            }
            for (int i = 0; i < config_.thread_num; i++) {
                _t[i].join();
            }


        }

    }

    void NaiveFaultModel::HardFaultGeneratorThread(uint64_t num_generate) {
        for (uint64_t i = 0; i < num_generate; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].hardfault |= ((uint64_t) 1 << bit);
            //std::cout << channel << rank << bankgroup << bankpergroup << row << column << bit << std::endl;
            //std::cout<<fault_map_[channel][rank][bankgroup][bankpergroup][row][column].hardfault<<std::endl;
            //std::cout << i << std::endl;
        }
    }

    void NaiveFaultModel::VRTErrorGenerator() {
        num_vrt_fault_cell = num_hard_fault_cell;

        num_vrt_fault_once_cell = num_vrt_fault_cell;
        num_vrt_fault_low_low_cell = num_vrt_fault_cell / 5;
        num_vrt_fault_low_cell = num_vrt_fault_cell / 5;
        num_vrt_fault_mid_cell = num_vrt_fault_cell / 3;
        num_vrt_fault_high_cell = num_vrt_fault_cell / 5;
        num_vrt_fault_high_high_cell = num_vrt_fault_cell / 5;

        //std::cout << num_all_cell << std::endl;
        //std::cout << num_hard_fault_cell << std::endl;

        if (config_.thread_model == "SingleThread") {
            VRTErrorGeneratorThread(num_vrt_fault_once_cell, num_vrt_fault_low_low_cell, num_vrt_fault_low_cell,
                                    num_vrt_fault_mid_cell, num_vrt_fault_high_cell, num_vrt_fault_high_high_cell);
        } else if (config_.thread_model == "MultiThread") {
            std::thread _t[config_.thread_num];

            for (int i = 0; i < config_.thread_num; i++) {
                _t[i] = std::thread(&NaiveFaultModel::VRTErrorGeneratorThread, this,
                                    num_vrt_fault_once_cell / (uint64_t) config_.thread_num,
                                    num_vrt_fault_low_low_cell / (uint64_t) config_.thread_num,
                                    num_vrt_fault_low_cell / (uint64_t) config_.thread_num,
                                    num_vrt_fault_mid_cell / (uint64_t) config_.thread_num,
                                    num_vrt_fault_high_cell / (uint64_t) config_.thread_num,
                                    num_vrt_fault_high_high_cell / (uint64_t) config_.thread_num);
            }
            for (int i = 0; i < config_.thread_num; i++) {
                _t[i].join();
            }
        }

    }

    void NaiveFaultModel::VRTErrorGeneratorThread(uint64_t num_generate_once, uint64_t num_generate_low_low,
                                                  uint64_t num_generate_low, uint64_t num_generate_mid,
                                                  uint64_t num_generate_high, uint64_t num_generate_high_high) {

        uint16_t rate;

        for (uint64_t i = 0; i < num_generate_once; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = 1;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"Low : " << bit << "   " << rate << "\n";
        }

        for (uint64_t i = 0; i < num_generate_low_low; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = (uint16_t) (std::abs(GetNormalInt(0, 1)) + 2);
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"Low : " << bit << "   " << rate << "\n";
        }

        for (uint64_t i = 0; i < num_generate_low; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = (uint16_t) (std::abs(GetNormalInt(0, 100)) + 2);
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"Low : " << bit << "   " << rate << "\n";
        }


        for (uint64_t i = 0; i < num_generate_mid; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = (uint16_t) GetRandomInt(1, 999);
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"Mid : " << bit << "   " << rate << "\n";
        }

        for (uint64_t i = 0; i < num_generate_high; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = (uint16_t) (999 - (std::abs(GetNormalInt(0, 100))));
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"High : " << bit << "   " << rate << "\n";
        }

        for (uint64_t i = 0; i < num_generate_high_high; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            rate = (uint16_t) (999 - (std::abs(GetNormalInt(0, 1))));
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                    std::make_tuple(bit, rate, false));

            //std::cout <<"High : " << bit << "   " << rate << "\n";
        }

    }


    BetaDistFaultModel::BetaDistFaultModel(Config &config, uint64_t *******data_block, Stat &stat)
            : FaultModel(config, data_block, stat) {
#ifndef TEST_MODE
        std::mt19937_64 gen(rd());
#endif

#ifdef TEST_MODE
        std::cout << "NaiveFaultModel" << std::endl;
#endif
        //data_block_[Channel][Rank][BankGourp][Bank][Row][Act_Col][BL]
        fault_map_ = new FaultStruct ******[config_.channels];

        for (int i = 0; i < config_.channels; i++) {
            fault_map_[i] = new FaultStruct *****[config_.ranks];
            for (int j = 0; j < config_.ranks; j++) {
                fault_map_[i][j] = new FaultStruct ****[config_.bankgroups];
                for (int k = 0; k < config_.bankgroups; k++) {
                    fault_map_[i][j][k] = new FaultStruct ***[config_.banks_per_group];
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        fault_map_[i][j][k][q] = new FaultStruct **[config_.rows];
                        for (int e = 0; e < config_.rows; e++) {
                            fault_map_[i][j][k][q][e] = new FaultStruct *[config_.actual_colums];
                            for (int w = 0; w < config_.actual_colums; w++) {
                                fault_map_[i][j][k][q][e][w] = new FaultStruct[config_.BL];
                                for (int f = 0; f < config_.BL; f++) {
                                    fault_map_[i][j][k][q][e][w][f] = {0, 0};
                                }
                            }
                        }
                    }
                }
            }
        }

        ErrorMask = new uint64_t[config_.BL];

        if (!config_.faultmap_read_path.empty()) {
            FaultMapReader *reader_ = new FaultMapReader(config_, config_.faultmap_read_path,
                                                         fault_map_);
#ifdef TEST_MODE
            std::cout << "FaultMap Reader" << std::endl;
#endif
            fault_map_ = reader_->Read();

            delete reader_;
        } else {

#ifdef TEST_MODE
            std::cout << "HardFaultGenerator" << std::endl;
#endif
            HardFaultGenerator();
#ifdef TEST_MODE
            std::cout << "VRTFaultGenerator" << std::endl;
#endif
            VRTErrorGenerator();
        }

        num_all_cell = 0;
        num_fault_cell = 0;
    }

    BetaDistFaultModel::~BetaDistFaultModel() {
#ifdef TEST_MODE
        std::cout << "NaiveFaultModel destructor" << std::endl;
#endif

        if (!config_.faultmap_write_path.empty()) {
            FaultMapWriter *writer_ = new FaultMapWriter(config_, config_.faultmap_write_path,
                                                         fault_map_);

#ifdef TEST_MODE
            std::cout << "FaultMap Writer" << std::endl;
#endif
            writer_->Write();

            delete writer_;
        }


        for (int i = 0; i < config_.channels; i++) {
            for (int j = 0; j < config_.ranks; j++) {
                for (int k = 0; k < config_.bankgroups; k++) {
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        for (int e = 0; e < config_.rows; e++) {
                            for (int w = 0; w < config_.actual_colums; w++) {
                                delete[] fault_map_[i][j][k][q][e][w];
                            }
                            delete[] fault_map_[i][j][k][q][e];
                        }
                        delete[] fault_map_[i][j][k][q];
                    }
                    delete[] fault_map_[i][j][k];
                }
                delete[] fault_map_[i][j];
            }
            delete[] fault_map_[i];
        }
        delete[] fault_map_;
    }

    uint64_t *BetaDistFaultModel::ErrorInjection(uint64_t addr) {

        for (int i = 0; i < config_.BL; i++) {
            ErrorMask[i] = 0;
        }
        SetRecvAddress(addr);

        HardFaultError();

        VRTFaultError();

        return ErrorMask;
    }

    void BetaDistFaultModel::HardFaultError() {
        for (int i = 0; i < config_.BL; i++) {
            ErrorMask[i] |= fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].hardfault;

            for (int j = 0; j < config_.bus_width; j++) {
                if (((ErrorMask[i]) & ((uint64_t) 1) << j)) {
                    stat_.hard_fault_bit_num++;
                }
            }
        }
    }

    void BetaDistFaultModel::VRTFaultError() {
        int rate;
        bool fault;

        for (int i = 0; i < config_.BL; i++) {
            if (fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt_size ==
                0)
                continue;
            for (int j = 0; j <
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt_size; j++) {
                rate = GetRandomInt(0, 1000);
                fault = false;
                if (std::get<1>(
                        fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) >
                    rate) {
                    fault = true;
                }

                /*
                if (fault != std::get<2>(
                        fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j])) {
                    rate = GetRandomInt(1, 1000);

                    if (std::get<1>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) >=
                        rate) {
                        fault = true;
                    }
                }
                */


                if (fault) {
                    ErrorMask[i] |= (uint64_t) 1 << ((config_.bus_width - 1) -
                                                     std::get<0>(
                                                             fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]));
                    stat_.vrt_fault_bit_num++;

                    std::get<2>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) = true;

                } else {
                    std::get<2>(
                            fault_map_[recv_addr_channel][recv_addr_rank][recv_addr_bankgroup][recv_addr_bank][recv_addr_row][recv_addr_column][i].vrt[j]) = false;
                }

            }
        }
    }

    void BetaDistFaultModel::DataDependentFaultError() {

    }

    void BetaDistFaultModel::HardFaultGenerator() {
        num_all_cell =
                (uint64_t) config_.channels * (uint64_t) config_.ranks * (uint64_t) config_.bankgroups *
                (uint64_t) config_.banks_per_group * (uint64_t) config_.rows *
                (uint64_t) config_.columns * (uint64_t) config_.bus_width;

        num_fault_cell = (uint64_t) ((double) num_all_cell * config_.fault_rate / 100);

        double a = 0.0;
        double temp = 0.01;
        std::cout << config_.beta_dist_alpha << config_.beta_dist_beta << std::endl;
        for (int i = 1; i < 100; i++) {
            a += std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, temp));
            //a += std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, temp));
            temp += 0.01;
        }
        a += std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, 0.991));
        //a += std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, 0.991));

        double b = (double) num_fault_cell;
        temp = 0.01;

        for (int i = 1; i < 100; i++) {
            num_fault_array[i] = (uint64_t) ((std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, temp)) * b) / a);
            temp += 0.01;
            //std::cout << num_fault_array[i] << std::endl;
        }
        num_fault_array[100] = (uint64_t) ((std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, 0.991)) * b) / a);
        //num_fault_array[100] = (uint64_t) ((std::pow(10, boost::math::ibeta_derivative(config_.beta_dist_alpha, config_.beta_dist_beta, 0.991)) * b) / a);


        for(unsigned long i : num_fault_array){
            std::cout << i << " " ;
        }
        std::cout << std::endl;

        for (int i = 1; i <= 100; i++){
            for(int j = 10 * (i - 1) + 1 ; j < 10 * (i - 1) + 1 + 10 ; j++){
                num_fault_array_ext[j] = (uint64_t )(num_fault_array[i] / 10.0);
            }
        }

        for(unsigned long i : num_fault_array_ext){
            std::cout << i << " " ;
        }
        std::cout << std::endl;


        if (config_.thread_model == "SingleThread") {
            std::cout << "Single" << std::endl;
            HardFaultGeneratorThread(num_fault_array_ext[1000]);
        } else if (config_.thread_model == "MultiThread") {
            std::cout << "Multi" << std::endl;
            std::thread _t[config_.thread_num];

            for (int i = 0; i < config_.thread_num; i++) {
                _t[i] = std::thread(&BetaDistFaultModel::HardFaultGeneratorThread, this,
                                    num_fault_array_ext[1000] / (uint64_t) config_.thread_num);
            }
            for (int i = 0; i < config_.thread_num; i++) {
                _t[i].join();
            }
        }
    }

    void BetaDistFaultModel::HardFaultGeneratorThread(uint64_t num_generate) {
        for (uint64_t i = 0; i < num_generate; i++) {
            int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
            channel = GetRandomInt(0, (config_.channels - 1));
            rank = GetRandomInt(0, (config_.ranks - 1));
            bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
            bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
            row = GetRandomInt(0, (config_.rows - 1));
            column = GetRandomInt(0, (config_.actual_colums - 1));
            bl = GetRandomInt(0, config_.BL - 1);
            bit = GetRandomInt(0, (config_.bus_width - 1));

            fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].hardfault |= ((uint64_t) 1 << bit);
            //std::cout << channel << rank << bankgroup << bankpergroup << row << column << bit << std::endl;
            //std::cout<<fault_map_[channel][rank][bankgroup][bankpergroup][row][column].hardfault<<std::endl;
            //std::cout << i << std::endl;
        }
    }

    void BetaDistFaultModel::VRTErrorGenerator() {

        //std::cout << num_all_cell << std::endl;
        //std::cout << num_hard_fault_cell << std::endl;

        if (config_.thread_model == "SingleThread") {
            VRTErrorGeneratorThread(0);
        } else if (config_.thread_model == "MultiThread") {
            std::thread _t[config_.thread_num];

            for (int i = 0; i < config_.thread_num; i++) {
                _t[i] = std::thread(&BetaDistFaultModel::VRTErrorGeneratorThread, this, i);
            }
            for (int i = 0; i < config_.thread_num; i++) {
                _t[i].join();
            }
        }

    }

    void BetaDistFaultModel::VRTErrorGeneratorThread(int thread_id) {

        for (uint64_t i = thread_id * 1000 / config_.thread_num;
             i < (uint64_t) (thread_id * 1000 / config_.thread_num + 1000 / config_.thread_num); i++) {
            if (i == 0)
                continue;

            for (uint64_t j = 0; j < num_fault_array_ext[i]; j++) {
                int channel, rank, bankgroup, bankpergroup, row, column, bl, bit;
                channel = GetRandomInt(0, (config_.channels - 1));
                rank = GetRandomInt(0, (config_.ranks - 1));
                bankgroup = GetRandomInt(0, (config_.bankgroups - 1));
                bankpergroup = GetRandomInt(0, (config_.banks_per_group - 1));
                row = GetRandomInt(0, (config_.rows - 1));
                column = GetRandomInt(0, (config_.actual_colums - 1));
                bl = GetRandomInt(0, config_.BL - 1);
                bit = GetRandomInt(0, (config_.bus_width - 1));

                fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt_size++;
                fault_map_[channel][rank][bankgroup][bankpergroup][row][column][bl].vrt.push_back(
                        std::make_tuple(bit, (uint16_t) i, false));
            }
            //std::cout << "Index : " << i << "  Value : " << num_fault_array_ext[i] << std::endl;
            //std::cout <<"Low : " << bit << "   " << rate << "\n";
        }

    }
}