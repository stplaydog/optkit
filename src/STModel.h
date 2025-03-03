
/**
 * Copy right YMSys, 2015,2016 Zhaoming Yin
 *
 * @brief    This is the class for create a spatial temporal model. 
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   02/08/16 - Creation
 *
 *  TODO    change this to template
**/

#pragma once
#ifndef __H_ST_MODEL__
#define __H_ST_MODEL__

#include <iostream>
#include <fstream>
#include "../libs/RTree.h"
#include "utils.h"

using namespace std;


/**
 * @class STModel 
 *
 * @note  This is the wrapper class that reads the raw data
 *        and build a RTree based on the raw data.
 *        This data can support two types of queries 
 *
 *        1) given the range of the cube, return all node ids
 *           in that range.
 *        2) given the range of the cube, return count of nodes 
 *           in that range.
 *
 *        This class should be able to convert time_t to a integer
 *        so that a common framework could be utilized to index points
 *        in given time and given place.
**/
class STModel 
{
public:
    STModel() {};

    ~STModel() {};

    virtual vector<int32_t> query_list(int32_t max[3], int32_t min[3]) = 0;

    virtual int32_t         query_cont(int32_t max[3], int32_t min[3]) = 0;

    virtual edge_list       build_edges(int32_t x_gap, int32_t y_gap, int32_t z_gap) = 0;

    virtual edge_list_CC    build_edge_list_CC(int32_t x_gap, int32_t y_gap, int32_t z_gap) = 0;
};

#endif // __H_ST_MODEL__ 
