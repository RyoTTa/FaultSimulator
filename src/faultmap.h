//
// Created by ryotta205 on 8/24/22.
//

#ifndef DRAMFAULTSIM_FAULTMAP_H
#define DRAMFAULTSIM_FAULTMAP_H

#include "common.h"
#include "configuration.h"
#include <fstream>

namespace dramfaultsim {
    class FaultMapReader {
    public:
        FaultMapReader( Config &config, std::string path, FaultStruct ******* fault_map);

        ~FaultMapReader(){
            reader_.close();
#ifdef TEST_MODE
            std::cout << "FaultMapReader destructor" << std::endl;
#endif
        }

        FaultStruct ******* Read();

    private:
        Config &config_;
        std::string path_;
        std::ifstream reader_;
        FaultStruct ******* fault_map_;

        int channel_size;
        int channels;
        int ranks;
        int bankgroups;
        int banks_per_group;
        int rows;
        int columns;
        int device_width;
        int bus_width;
        int devices_per_rank;
        int BL;
    };

    class FaultMapWriter {
    public:
        FaultMapWriter(Config &config, std::string path, FaultStruct ******* fault_map);

        ~FaultMapWriter(){
            writer_.close();
#ifdef TEST_MODE
            std::cout << "FaultMapWriter destructor" << std::endl;
#endif
        }

        bool Write();

    private:
        Config &config_;
        std::string path_;
        std::ofstream writer_;
        FaultStruct ******* fault_map_;
    };
}
#endif //DRAMFAULTSIM_FAULTMAP_H
