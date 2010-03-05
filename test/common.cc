#include "common.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

void write_result( boost::program_options::variables_map &var_map, Ipp16s *result, int siglen ) {
    if( var_map.count("outfile") ) {
        boost::filesystem::path fp( var_map["outfile"].as< std::string >() );
        boost::filesystem::ofstream file( fp );
        file.write((char *)result, sizeof(*result) * siglen);
        file.close();
    } else {
        std::cout.write((char *)result, sizeof(*result) * siglen);
    }
} 
