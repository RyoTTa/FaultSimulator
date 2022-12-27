//
// Created by ryotta205 on 8/17/22.
//
#include "generator.h"
#include <bitset>

namespace dramfaultsim {
    RandomGenerator::RandomGenerator(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace)
            : Generator(config, stat, fault_result, fault_trace) {
        if (config_.memory_system == "NaiveMemorySystem") {
            memory_system_ = new NaiveMemorySystem(config_, stat_, fault_result_, fault_trace_);
        }else{
            memory_system_ = new NaiveMemorySystem(config_, stat_, fault_result_, fault_trace_);
        }

        gen_data = new uint64_t [config_.BL];

    }

    RandomGenerator::~RandomGenerator() {
#ifdef TEST_MODE
        std::cout << "RandomGenerator destructor" << std::endl;
#endif

        delete memory_system_;
    }

    bool RandomGenerator::AccessMemory() {
        Generator::AccessMemory();

#ifndef TEST_MODE
        std::mt19937_64 gen(rd());
#endif

        gen_addr = gen();
        gen_addr = gen_addr & config_.effective_addr_mask;

        if(config_.data_pattern_str == "Random"){
            //Generate Random Data Pattern
            for(int i = 0; i < config_.BL ; i++){
                gen_data[i] = gen();
            }
        }else{
            //Generate Configuration Data Pattern
            for(int i = 0; i < config_.BL ; i++){
                gen_data[i] = config_.data_pattern;
            }
        }

        is_write = (gen() % 5 == 0);

/*
#ifdef TEST_MODE
        std::cout << "Generate Memory(MASK)  : 0x" << std::hex << gen_addr << std::dec << (is_write ? "  WRITE" : "  READ")<< std::endl;
        for (int i=0; i < config_.BL; i++){
            std::cout << "Generate Data         "<< i <<": 0x" << std::hex << gen_data[i] << std::dec << std::endl;
        }
#endif
*/
        memory_system_->RecvRequest(gen_addr, is_write, gen_data);

        if (config_.num_request == 0)
            return false;
        if (stat_.executed_request_num >= config_.num_request)
            return true;
        else
            return false;

    }

    SequentialGenerator::SequentialGenerator(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace)
    : Generator(config, stat, fault_result, fault_trace) {
        if (config_.memory_system == "NaiveMemorySystem") {
            memory_system_ = new NaiveMemorySystem(config_, stat_, fault_result_, fault_trace_);
        }else{
            memory_system_ = new NaiveMemorySystem(config_, stat_, fault_result_, fault_trace_);
        }

        gen_data = new uint64_t [config_.BL];

    }

    SequentialGenerator::~SequentialGenerator() {
#ifdef TEST_MODE
        std::cout << "SequentialGenerator destructor" << std::endl;
#endif

        delete memory_system_;
    }

    bool SequentialGenerator::AccessMemory() {

#ifndef TEST_MODE
        std::mt19937_64 gen(rd());
#endif

        for (int i = 0; i < config_.channels; i++) {
            for (int j = 0; j < config_.ranks; j++) {
                for (int k = 0; k < config_.bankgroups; k++) {
                    for (int q = 0; q < config_.banks_per_group; q++) {
                        for (int e = 0; e < config_.rows; e++) {
                            for (int f = 0; f < config_.actual_colums; f++) {
                                Generator::AccessMemory();

                                if(config_.data_pattern_str == "Random"){
                                    //Generate Random Data Pattern
                                    for(int w = 0; w < config_.BL ; w++){
                                        gen_data[w] = gen();
                                    }
                                }else{
                                    //Generate Configuration Data Pattern
                                    for(int w = 0; w < config_.BL ; w++){
                                        gen_data[w] = config_.data_pattern;
                                    }
                                }
                                is_write = (gen() % 5 == 0);
                                gen_addr = config_.ReverseAddressMapping(i,j,k,q,e,f);
                                /*
#ifdef TEST_MODE
                                std::cout << "Generate Memory(MASK)  : 0x" << std::hex << gen_addr << std::dec << (is_write ? "  WRITE" : "  READ")<< std::endl;
                                for (int i=0; i < config_.BL; i++){
                                    std::cout << "Generate Data         "<< i <<": 0x" << std::hex << gen_data[i] << std::dec << std::endl;
                                }
#endif
                                 */
                                memory_system_->RecvRequest(gen_addr, is_write, gen_data);

                                if (config_.num_request == 0)
                                    continue;
                                if (stat_.executed_request_num >= config_.num_request)
                                    return true;
                                else
                                    continue;
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
}