//
// Created by ryotta205 on 8/17/22.
//
#ifndef DRAMFAULTSIM_GENERATOR_H
#define DRAMFAULTSIM_GENERATOR_H

#include "common.h"
#include "configuration.h"
#include "memory_system.h"
#include "stat.h"
#include "faultresult.h"
#include <random>

namespace dramfaultsim {

    class Generator {
    public:
        Generator(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace)
                : config_(config), stat_(stat), fault_result_(fault_result),fault_trace_(fault_trace),num_executed_request(0) {};
        virtual ~Generator(){};

        virtual bool AccessMemory() {
            num_executed_request++;
            stat_.executed_request_num++;
            stat_.executed_column_num += config_.BL;
            return true;
        }

        Config &config_;
        Stat &stat_;
        FaultResult &fault_result_;
        FaultTrace &fault_trace_;
        MemorySystem *memory_system_;
        uint64_t num_executed_request;

    };

    class RandomGenerator : public Generator {
    public:
        RandomGenerator(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace);
        ~RandomGenerator() override;

        bool AccessMemory() override;

    private:
        std::random_device rd;
        uint64_t gen_addr;
        uint64_t *gen_data;
        bool is_write;

#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif
    };

    class SequentialGenerator : public Generator {
    public:
        SequentialGenerator(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace);
        ~SequentialGenerator() override;

        bool AccessMemory() override;

    private:
        std::random_device rd;
        uint64_t gen_addr;
        uint64_t *gen_data;
        bool is_write;

#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif
    };
}

#endif //DRAMFAULTSIM_GENERATOR_H
