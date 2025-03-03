/**
 * Copy right YMSys, 2015,2016 Zhaoming Yin
 *
 *  @brief    Unittest of CSR graph structure. 
 *
 *  MODIFIED   (MM/DD/YY)
 *  stplaydog   07/10/16 - Finish all the tests 
 *  stplaydog   07/03/16 - Creation
**/

#include <../src/Config.h>
#include <../src/Stats.h>
#include <gtest/gtest.h>
#include <fstream>
#include "CrimeSTModel.h" 

using namespace std;
using boost::archive::text_oarchive;
using boost::archive::text_iarchive;
using boost::archive::archive_exception;

/**
 * @brief   Test serialize config 
 *
**/
TEST(ConfigSerializationTest_1, Success)
{
    Config::instance()->clear();

    {
        ofstream ofs("./filename");
        text_oarchive oa(ofs);
        string key = "key1";
        string val = "val1";
        Config::instance()->set(key, val);

        key = "key2";
        val = "val2";
        Config::instance()->set(key, val);

        oa<<*Config::instance();

        Config::instance()->clear();
    }

    {
        ifstream ifs("./filename");
        text_iarchive ia(ifs);
        ia>>*Config::instance();

        string key = "key1";
        ASSERT_EQ(Config::instance()->get(key) == "val1", true);
        key = "key2";
        ASSERT_EQ(Config::instance()->get(key) == "val2", true);
    }

    std::remove("./filename");
}

/**
 * @brief   Test serialize Stats 
 *
**/
TEST(StatsSerializationTest_1, Success)
{
    Stats::instance()->clear();

    int_feature_l    t_range        = {1, 2, 3};
    int_feature_l    t_numV         = {1, 2, 3, 4, 5, 6};
    int_feature_l    t_numE         = {1, 2, 3, 4, 5, 6};
    int_feature_l    t_diameter     = {1, 2, 3, 4, 5, 6};
    int_feature_l    t_girth        = {1, 2, 3, 4, 5, 6};
    double_feature_l t_clusterCoeff = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    double_feature_ll t_betweenCentrl = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    int_pair_feature_ll t_numTruss = {{{1,2}, {3,4}},{{5,6},{7,8}}};
    int_pair_feature_ll t_numClique = {{{1,2}, {3,4}},{{5,6},{7,8}}};

    {
        ofstream ofs("./filename");
        text_oarchive oa(ofs);
        Stats::instance()->m_application               = "stmodel";
        Stats::instance()->m_outFile                   = "file1";
        Stats::instance()->m_time                      = "time1";
        Stats::instance()->m_dataName                  = "dataName1";
        Stats::instance()->m_range       	       = t_range;
        Stats::instance()->m_gProperty.m_numCC         = 1234;
        Stats::instance()->m_gProperty.m_numV          = t_numV;
        Stats::instance()->m_gProperty.m_numE          = t_numE;
        Stats::instance()->m_gProperty.m_diameter      = t_diameter;
        Stats::instance()->m_gProperty.m_girth         = t_girth;
        Stats::instance()->m_gProperty.m_clusterCoeff  = t_clusterCoeff;
        Stats::instance()->m_gProperty.m_betweenCentrl = t_betweenCentrl;
        Stats::instance()->m_gProperty.m_numTruss      = t_numTruss;
        Stats::instance()->m_gProperty.m_numClique     = t_numClique;

        oa<<*Stats::instance();

        Stats::instance()->clear();
    }

    {
        ifstream ifs("./filename");
        text_iarchive ia(ifs);
        ia>>*Stats::instance();

#define TEST_S_INS(x, y) ASSERT_EQ((Stats::instance()->x == y), true)

        TEST_S_INS(m_application, 		"stmodel");
        TEST_S_INS(m_outFile,     		"file1");
        TEST_S_INS(m_time,        		"time1");
        TEST_S_INS(m_dataName,    		"dataName1");
        TEST_S_INS(m_range,                     t_range);
        TEST_S_INS(m_gProperty.m_numCC,         1234);
        TEST_S_INS(m_gProperty.m_numV,          t_numV);
        TEST_S_INS(m_gProperty.m_numE,          t_numE);
        TEST_S_INS(m_gProperty.m_diameter,      t_diameter);
        TEST_S_INS(m_gProperty.m_girth,         t_girth);
        TEST_S_INS(m_gProperty.m_clusterCoeff,  t_clusterCoeff);
        TEST_S_INS(m_gProperty.m_betweenCentrl, t_betweenCentrl);
        TEST_S_INS(m_gProperty.m_numTruss,      t_numTruss);
        TEST_S_INS(m_gProperty.m_numClique,     t_numClique);
    }

    std::remove("./filename");
}

/**
 * @brief   Test serialize st model 
 *
**/
TEST(CrimeSTModelSerializationTest_1, Success)
{
    string input_file = "../data/truss/ny_crime.csv"; 
    CrimeSTModel stm(input_file);

    {
        ofstream ofs("./filename");
        text_oarchive oa(ofs);
        oa<<stm;
    }

    {
        CrimeSTModel stm1;
        ifstream ifs("./filename");
        text_iarchive ia(ifs);
        ia>>stm1;
        ASSERT_EQ((stm1 == stm), true);
    }

    std::remove("./filename");
}

/**
 * @brief   Test serialize edges 
 *
**/
TEST(EdgeListSerializationTest_1, Success)
{
    string input_file = "../data/truss/ny_crime.csv"; 
    CrimeSTModel stm(input_file);
    edge_list edges = stm.build_edges(200, 200, 30);

    {
        ofstream ofs("./filename");
        text_oarchive oa(ofs);
        oa<<edges;
    }

    {
        edge_list edges1;
        ifstream ifs("./filename");
        text_iarchive ia(ifs);
        ia>>edges1;
        ASSERT_EQ((edges == edges1), true);
    }

    std::remove("./filename");
}

/**
 * @brief   Test serialize edge list with connected components 
 *
**/
TEST(EdgeListCCSerializationTest_1, Success)
{
    string input_file = "../data/truss/ny_crime.csv"; 
    CrimeSTModel stm(input_file);
    edge_list_CC edges = stm.build_edge_list_CC(200, 200, 30);

    {
        ofstream ofs("./filename");
        text_oarchive oa(ofs);
        oa<<edges;
    }

    {
        edge_list_CC edges1;
        ifstream ifs("./filename");
        text_iarchive ia(ifs);
        ia>>edges1;
        ASSERT_EQ((edges == edges1), true);
    }

    std::remove("./filename");
}
